#include <tos/debug/log.hpp>

#include "host/ble_hs.h"
#include <host/ble_gatt.h>
#include <os/os_mbuf.h>
#include <services/gatt/ble_svc_gatt.h>

static const ble_uuid128_t gatt_svr_svc_sec_test_uuid = BLE_UUID128_INIT(0x2d,
                                                                         0x71,
                                                                         0xa2,
                                                                         0x59,
                                                                         0xb4,
                                                                         0x58,
                                                                         0xc8,
                                                                         0x12,
                                                                         0x99,
                                                                         0x99,
                                                                         0x43,
                                                                         0x95,
                                                                         0x12,
                                                                         0x2f,
                                                                         0x46,
                                                                         0x59);

/* 5c3a659e-897e-45e1-b016-007107c96df6 */
static const ble_uuid128_t gatt_svr_chr_sec_test_rand_uuid = BLE_UUID128_INIT(0xf6,
                                                                              0x6d,
                                                                              0xc9,
                                                                              0x07,
                                                                              0x71,
                                                                              0x00,
                                                                              0x16,
                                                                              0xb0,
                                                                              0xe1,
                                                                              0x45,
                                                                              0x7e,
                                                                              0x89,
                                                                              0x9e,
                                                                              0x65,
                                                                              0x3a,
                                                                              0x5c);

/* 5c3a659e-897e-45e1-b016-007107c96df7 */
static const ble_uuid128_t gatt_svr_chr_sec_test_static_uuid = BLE_UUID128_INIT(0xf7,
                                                                                0x6d,
                                                                                0xc9,
                                                                                0x07,
                                                                                0x71,
                                                                                0x00,
                                                                                0x16,
                                                                                0xb0,
                                                                                0xe1,
                                                                                0x45,
                                                                                0x7e,
                                                                                0x89,
                                                                                0x9e,
                                                                                0x65,
                                                                                0x3a,
                                                                                0x5c);

static uint8_t gatt_svr_sec_test_static_val;

static int gatt_svr_chr_write(
    struct os_mbuf* om, uint16_t min_len, uint16_t max_len, void* dst, uint16_t* len) {
    uint16_t om_len;
    int rc;

    om_len = OS_MBUF_PKTLEN(om);
    if (om_len < min_len || om_len > max_len) {
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }

    rc = ble_hs_mbuf_to_flat(om, dst, max_len, len);
    if (rc != 0) {
        return BLE_ATT_ERR_UNLIKELY;
    }

    return 0;
}

static int gatt_svr_chr_access_sec_test(uint16_t conn_handle,
                                        uint16_t attr_handle,
                                        struct ble_gatt_access_ctxt* ctxt,
                                        void* arg) {
    const ble_uuid_t* uuid;
    int rand_num;
    int rc;

    uuid = ctxt->chr->uuid;

    /* Determine which characteristic is being accessed by examining its
     * 128-bit UUID.
     */

    if (ble_uuid_cmp(uuid, &gatt_svr_chr_sec_test_rand_uuid.u) == 0) {
        LOG(ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR);

        /* Respond with a 32-bit random number. */
        rand_num = 42;
        rc = os_mbuf_append(ctxt->om, &rand_num, sizeof rand_num);
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    if (ble_uuid_cmp(uuid, &gatt_svr_chr_sec_test_static_uuid.u) == 0) {
        switch (ctxt->op) {
        case BLE_GATT_ACCESS_OP_READ_CHR:
            LOG("Reading");
            rc = os_mbuf_append(ctxt->om,
                                &gatt_svr_sec_test_static_val,
                                sizeof gatt_svr_sec_test_static_val);
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

        case BLE_GATT_ACCESS_OP_WRITE_CHR:
            rc = gatt_svr_chr_write(ctxt->om,
                                    sizeof gatt_svr_sec_test_static_val,
                                    sizeof gatt_svr_sec_test_static_val,
                                    &gatt_svr_sec_test_static_val,
                                    NULL);
            return rc;

        default:
            assert(0);
            return BLE_ATT_ERR_UNLIKELY;
        }
    }

    LOG_ERROR("Unknown service");

    /* Unknown characteristic; the nimble stack should not have called this
     * function.
     */
    assert(0);
    return BLE_ATT_ERR_UNLIKELY;
}

static ble_gatt_chr_def characteristics[] = {
    {
        /*** Characteristic: Random number generator. */
        .uuid = &gatt_svr_chr_sec_test_rand_uuid.u,
        .access_cb = gatt_svr_chr_access_sec_test,
        .flags = BLE_GATT_CHR_F_READ,
    },
    {
        /*** Characteristic: Static value. */
        .uuid = &gatt_svr_chr_sec_test_static_uuid.u,
        .access_cb = gatt_svr_chr_access_sec_test,
        .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_ENC,
    },
    {
        0, /* No more characteristics in this service. */
    }};

static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        /*** Service: Security test. */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &gatt_svr_svc_sec_test_uuid.u,
        .characteristics = &characteristics[0],
    },
    {
        0, /* No more services. */
    },
};

void gatt_svr_register_cb(struct ble_gatt_register_ctxt* ctxt, void* arg) {
    char buf[BLE_UUID_STR_LEN];

    switch (ctxt->op) {
    case BLE_GATT_REGISTER_OP_SVC:
        LOG_FORMAT("registered service {} with handle={}\n",
            ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
            ctxt->svc.handle);
        break;

    case BLE_GATT_REGISTER_OP_CHR:
        LOG_FORMAT("registering characteristic {} with "
            "def_handle={} val_handle={}\n",
            ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
            ctxt->chr.def_handle,
            ctxt->chr.val_handle);
        break;

    case BLE_GATT_REGISTER_OP_DSC:
        LOG_FORMAT("registering descriptor {} with handle={}\n",
            ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
            ctxt->dsc.handle);
        break;

    default:
        assert(0);
        break;
    }
}

int gatt_svr_init(void) {
    int rc;

    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    return 0;
}