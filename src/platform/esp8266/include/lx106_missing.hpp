#ifndef ESPMISSINGINCLUDES_H
#define ESPMISSINGINCLUDES_H
#ifdef __cplusplus
extern "C"
{
#endif
#include <ets_sys.h>
#include <stdint.h>

//Missing function prototypes in include folders. Gcc will warn on these if we don't define 'em anywhere.
//MOST OF THESE ARE GUESSED! but they seem to swork and shut up the compiler.
typedef struct espconn espconn;

int atoi(const char *nptr);
void ets_isr_mask(unsigned intr);
void ets_isr_unmask(unsigned intr);
int ets_memcmp(const void *s1, const void *s2, size_t n);
void *ets_memcpy(void *dest, const void *src, size_t n);
void *ets_memset(void *s, int c, size_t n);
int ets_sprintf(char *str, const char *format, ...)  __attribute__ ((format (printf, 2, 3)));
int ets_str2macaddr(void *, void *);
int ets_strcmp(const char *s1, const char *s2);
char *ets_strcpy(char *dest, const char *src);
int ets_strlen(const char *s);
char *ets_strncpy(char *dest, const char *src, size_t n);
char *ets_strstr(const char *haystack, const char *needle);
void ets_timer_disarm(ETSTimer *a);
void ets_timer_setfn(ETSTimer *t, ETSTimerFunc *fn, void *parg);
void ets_update_cpu_frequency(int freqmhz);
int os_printf(const char *format, ...)  __attribute__ ((format (printf, 1, 2)));
int os_snprintf(char *str, size_t size, const char *format, ...) __attribute__ ((format (printf, 3, 4)));
int os_printf_plus(const char *format, ...)  __attribute__ ((format (printf, 1, 2)));
void pvPortFree(void *ptr);
void *vPortMalloc(size_t xWantedSize);
uint8 wifi_get_opmode(void);
uint32 system_get_time();
unsigned long os_random();
int rand(void);
void ets_bzero(void *s, size_t n);
#ifdef __cplusplus
}
#endif
#endif
