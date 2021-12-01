#include <sqlite3ext.h>
SQLITE_EXTENSION_INIT1
#include <assert.h>
#include <string.h>
#include <tos/debug/log.hpp>
/*
** Forward declaration of objects used by this utility
*/
typedef struct sqlite3_vfs MemVfs;
typedef struct MemFile MemFile;

/* An open file */
struct MemFile {
    sqlite3_file base;    /* IO methods */
    sqlite3_int64 sz;     /* Size of the file */
    sqlite3_int64 szMax;  /* Space allocated to aData */
    unsigned char* aData; /* content of the file */
    int bFreeOnClose;     /* Invoke sqlite3_free() on aData at close */
};
/*
** Methods for MemFile
*/
static int memClose(sqlite3_file*);
static int memRead(sqlite3_file*, void*, int iAmt, sqlite3_int64 iOfst);
static int memWrite(sqlite3_file*, const void*, int iAmt, sqlite3_int64 iOfst);
static int memTruncate(sqlite3_file*, sqlite3_int64 size);
static int memSync(sqlite3_file*, int flags);
static int memFileSize(sqlite3_file*, sqlite3_int64* pSize);
static int memLock(sqlite3_file*, int);
static int memUnlock(sqlite3_file*, int);
static int memCheckReservedLock(sqlite3_file*, int* pResOut);
static int memFileControl(sqlite3_file*, int op, void* pArg);
static int memSectorSize(sqlite3_file*);
static int memDeviceCharacteristics(sqlite3_file*);
static int memFetch(sqlite3_file*, sqlite3_int64 iOfst, int iAmt, void** pp);
static int memUnfetch(sqlite3_file*, sqlite3_int64 iOfst, void* p);
/*
** Methods for MemVfs
*/
static int memOpen(sqlite3_vfs*, const char*, sqlite3_file*, int, int*);
static int memDelete(sqlite3_vfs*, const char* zName, int syncDir);
static int memAccess(sqlite3_vfs*, const char* zName, int flags, int*);
static int memFullPathname(sqlite3_vfs*, const char* zName, int, char* zOut);
static int memRandomness(sqlite3_vfs*, int nByte, char* zOut);
static int memSleep(sqlite3_vfs*, int microseconds);
static int memCurrentTime(sqlite3_vfs*, double*);
static int memGetLastError(sqlite3_vfs*, int, char*);
static int memCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);
static sqlite3_vfs mem_vfs = {
    2,                  /* iVersion */
    sizeof(MemFile),    /* szOsFile (set when registered) */
    1024,               /* mxPathname */
    nullptr,            /* pNext */
    "memvfs",           /* zName */
    nullptr,            /* pAppData (set when registered) */
    memOpen,            /* xOpen */
    memDelete,          /* xDelete */
    memAccess,          /* xAccess */
    memFullPathname,    /* xFullPathname */
    nullptr,            /* xDlOpen */
    nullptr,            /* xDlError */
    nullptr,            /* xDlSym */
    nullptr,            /* xDlClose */
    memRandomness,      /* xRandomness */
    memSleep,           /* xSleep */
    memCurrentTime,     /* xCurrentTime */
    memGetLastError,    /* xGetLastError */
    memCurrentTimeInt64 /* xCurrentTimeInt64 */
};

static const sqlite3_io_methods mem_io_methods = {
    3,                        /* iVersion */
    memClose,                 /* xClose */
    memRead,                  /* xRead */
    memWrite,                 /* xWrite */
    memTruncate,              /* xTruncate */
    memSync,                  /* xSync */
    memFileSize,              /* xFileSize */
    memLock,                  /* xLock */
    memUnlock,                /* xUnlock */
    memCheckReservedLock,     /* xCheckReservedLock */
    memFileControl,           /* xFileControl */
    memSectorSize,            /* xSectorSize */
    memDeviceCharacteristics, /* xDeviceCharacteristics */
    nullptr,                  /* xShmMap */
    nullptr,                  /* xShmLock */
    nullptr,                  /* xShmBarrier */
    nullptr,                  /* xShmUnmap */
    memFetch,                 /* xFetch */
    memUnfetch                /* xUnfetch */
};
/*
** Close an mem-file.
**
** The pData pointer is owned by the application, so there is nothing
** to free.
*/
static int memClose(sqlite3_file* pFile) {
    MemFile* p = (MemFile*)pFile;
    if (p->bFreeOnClose)
        sqlite3_free(p->aData);
    return SQLITE_OK;
}
/*
** Read data from an mem-file.
*/
static int memRead(sqlite3_file* pFile, void* zBuf, int iAmt, sqlite_int64 iOfst) {

    MemFile* p = (MemFile*)pFile;
    LOG("Read", p->aData);
    // LOG(p->aData);
    memcpy(zBuf, p->aData + iOfst, iAmt);
    // tos::span<uint8_t> buf{static_cast<uint8_t*>(zBuf), static_cast<size_t>(iAmt)};
    // LOG(buf);
    return SQLITE_OK;
}
/*
** Write data to an mem-file.
*/
static int memWrite(sqlite3_file* pFile, const void* z, int iAmt, sqlite_int64 iOfst) {
    LOG("Write", z, iAmt, (int)iOfst);
    MemFile* p = (MemFile*)pFile;
    if (iOfst + iAmt > p->sz) {
        if (iOfst + iAmt > p->szMax)
            return SQLITE_FULL;
        if (iOfst > p->sz)
            memset(p->aData + p->sz, 0, iOfst - p->sz);
        p->sz = iOfst + iAmt;
    }
    memcpy(p->aData + iOfst, z, iAmt);
    return SQLITE_OK;
}
/*
** Truncate an mem-file.
*/
static int memTruncate(sqlite3_file* pFile, sqlite_int64 size) {
    LOG("Truncate");
    MemFile* p = (MemFile*)pFile;
    if (size > p->sz) {
        if (size > p->szMax)
            return SQLITE_FULL;
        memset(p->aData + p->sz, 0, size - p->sz);
    }
    p->sz = size;
    return SQLITE_OK;
}
/*
** Sync an mem-file.
*/
static int memSync(sqlite3_file* pFile, int flags) {
    return SQLITE_OK;
}
/*
** Return the current file-size of an mem-file.
*/
static int memFileSize(sqlite3_file* pFile, sqlite_int64* pSize) {
    MemFile* p = (MemFile*)pFile;
    *pSize = p->sz;
    return SQLITE_OK;
}
/*
** Lock an mem-file.
*/
static int memLock(sqlite3_file* pFile, int eLock) {
    return SQLITE_OK;
}
/*
** Unlock an mem-file.
*/
static int memUnlock(sqlite3_file* pFile, int eLock) {
    return SQLITE_OK;
}
/*
** Check if another file-handle holds a RESERVED lock on an mem-file.
*/
static int memCheckReservedLock(sqlite3_file* pFile, int* pResOut) {
    *pResOut = 0;
    return SQLITE_OK;
}
/*
** File control method. For custom operations on an mem-file.
*/
static int memFileControl(sqlite3_file* pFile, int op, void* pArg) {
    MemFile* p = (MemFile*)pFile;
    int rc = SQLITE_NOTFOUND;
    if (op == SQLITE_FCNTL_VFSNAME) {
        *(char**)pArg = sqlite3_mprintf("mem(%p,%lld)", p->aData, p->sz);
        rc = SQLITE_OK;
    }
    return rc;
}
/*
** Return the sector-size in bytes for an mem-file.
*/
static int memSectorSize(sqlite3_file* pFile) {
    return 512;
}
/*
** Return the device characteristic flags supported by an mem-file.
*/
static int memDeviceCharacteristics(sqlite3_file* pFile) {
    return SQLITE_IOCAP_ATOMIC | SQLITE_IOCAP_POWERSAFE_OVERWRITE |
           SQLITE_IOCAP_SAFE_APPEND | SQLITE_IOCAP_SEQUENTIAL;
}

/* Fetch a page of a memory-mapped file */
static int memFetch(sqlite3_file* pFile, sqlite3_int64 iOfst, int iAmt, void** pp) {
    LOG("Fetch", (int)iOfst, iAmt);
    MemFile* p = (MemFile*)pFile;
    *pp = (void*)(p->aData + iOfst);
    return SQLITE_OK;
}
/* Release a memory-mapped page */
static int memUnfetch(sqlite3_file* pFile, sqlite3_int64 iOfst, void* pPage) {
    return SQLITE_OK;
}
/*
** Open an mem file handle.
*/
static int memOpen(sqlite3_vfs* pVfs,
                   const char* zName,
                   sqlite3_file* pFile,
                   int flags,
                   int* pOutFlags) {
    LOG("Open", zName);
    auto p = (MemFile*)pFile;
    memset(p, 0, sizeof(*p));
    p->aData = (unsigned char*)sqlite3_uri_int64(zName, "ptr", 0);
    if (p->aData == 0) {
        tos::debug::error("Bad ptr");
        return SQLITE_CANTOPEN;
    }
    p->sz = sqlite3_uri_int64(zName, "sz", 0);
    if (p->sz < 0) {
        tos::debug::error("Bad size");
        return SQLITE_CANTOPEN;
    }
    p->szMax = sqlite3_uri_int64(zName, "max", p->sz);
    if (p->szMax < p->sz) {
        tos::debug::error("Bad max");
        return SQLITE_CANTOPEN;
    }

    LOG("Open", zName, p->aData, (int)p->sz, (int)p->szMax);

    p->bFreeOnClose = sqlite3_uri_boolean(zName, "freeonclose", 0);
    pFile->pMethods = &mem_io_methods;
    LOG(p->aData);
    return SQLITE_OK;
}
/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
static int memDelete(sqlite3_vfs* pVfs, const char* zPath, int dirSync) {
    return SQLITE_IOERR_DELETE;
}
/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
static int memAccess(sqlite3_vfs* pVfs, const char* zPath, int flags, int* pResOut) {
    LOG("Access", zPath, flags);
    *pResOut = 0;
    return SQLITE_OK;
}
/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
static int memFullPathname(sqlite3_vfs* pVfs, const char* zPath, int nOut, char* zOut) {
    sqlite3_snprintf(nOut, zOut, "%s", zPath);
    LOG("Full path", zOut);
    return SQLITE_OK;
}


/*
** Populate the buffer pointed to by zBufOut with nByte bytes of
** random data.
*/
static int memRandomness(sqlite3_vfs* pVfs, int nByte, char* zBufOut) {
    return nByte;
}
/*
** Sleep for nMicro microseconds. Return the number of microseconds
** actually slept.
*/
static int memSleep(sqlite3_vfs* pVfs, int nMicro) {
    return nMicro;
}
/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
static int memCurrentTime(sqlite3_vfs* pVfs, double* pTimeOut) {
    return 0;
}

static int memGetLastError(sqlite3_vfs* pVfs, int a, char* b) {
    b = const_cast<char*>("");
    return 0;
}

static int memCurrentTimeInt64(sqlite3_vfs* pVfs, sqlite3_int64* p) {
    return 0;
}

/*
** This routine is called when the extension is loaded.
** Register the new VFS.
*/
int sqlite3_memvfs_init() {
    int rc = SQLITE_OK;
    rc = sqlite3_vfs_register(&mem_vfs, 1);
    LOG(rc);
    if (rc == SQLITE_OK)
        rc = SQLITE_OK_LOAD_PERMANENTLY;
    return rc;
}