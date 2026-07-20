#include <string.h>

#include "common/mem.h"

#include "ehsmdrv/basic/api.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"

void ehsm_demo_test_get_version(bool_t *is_bl)
{
    uint32_t ret;
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx();
    ehsm_version_st *version = (ehsm_version_st *)ehsm_demo_get_buffer(0);
    memset(version, 0, sizeof(ehsm_version_st));

    ehsm_ctx_init(ctx, 0, false, NULL);

    ret = ehsm_get_version(ctx, version);
    if (ret != EHSM_OK) {
        ehsm_port_printf("ehsm_get_version failed, ret: 0x%08x\n", ret);
    } else {
        ehsm_port_printf("type: %s\n", version->type == 0 ? "BootLoader" : "Firmware");
        if (version->ver_pre_release[0] != '\0') {
            ehsm_port_printf("version: %d.%d.%d-%s\n", version->ver_major, version->ver_minor, version->ver_patch,
                (const char *)version->ver_pre_release);
        } else {
            ehsm_port_printf("version: %d.%d.%d\n", version->ver_major, version->ver_minor, version->ver_patch);
        }
        ehsm_port_printf("hash lib version: 0x%08x\n", version->hash_lib_ver);
        ehsm_port_printf("trng lib version: 0x%08x\n", version->trng_lib_ver);
        ehsm_port_printf("ske lib version: 0x%08x\n", version->ske_lib_ver);
        ehsm_port_printf("pke lib version: 0x%08x\n", version->pke_lib_ver);
        ehsm_port_printf("HW version: 0x%08x\n", version->hw_ver);
        ehsm_port_printf("UID: ");
        for (uint32_t i = 0; i < 16; i++) {
            ehsm_port_printf("%02x", (uint32_t)version->uid[i]);
        }
        ehsm_port_printf("\n");

        *is_bl = ((uint32_t)version->type == 0U) ? true : false;
    }
}
