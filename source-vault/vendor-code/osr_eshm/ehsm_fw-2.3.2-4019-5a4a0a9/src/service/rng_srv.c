/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "rng_srv.h"
#include "mb.h"
#include "mmap.h"
#include "component/util.h"
#include "component/crypto_api.h"
/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define RNG_SRV_DATA_BUF_BYTE_SIZE (128U)
/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  EXTERN
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL TYPEDEFS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
static uint32_t rng_check_alg(uint8_t alg)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t rng_post_proc_cfg;
    uint32_t cur_alg = cpt_trng_get_drbg_alg();
    if (MB_RNG_ALGORITHM_SM4_CTRDRBG == alg) {
        rng_post_proc_cfg = GM_GLOBAL_CONFIG;
    } else if (MB_RNG_ALGORITHM_AES_CTRDRBG == alg) {
        rng_post_proc_cfg = NIST_GLOBAL_CONFIG;
    } else {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (alg != cur_alg) {
            ret = cpt_trng_set_global_init_config(rng_post_proc_cfg);
        }
    }
    return ret;
}

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t rng_srv_init(void)
{
    return EHSM_ERR_SW_SUCCESS;
}

uint32_t rng_srv_handler(const cmd_req_data_st *req_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t once_sz;
    uint32_t remain_sz;
    uint64_t host_addr;
    uint8_t rng_data_buf[RNG_SRV_DATA_BUF_BYTE_SIZE];
    const mb_cmd_rng_st *rng_cmd_data;

    if ((NULL != req_data) && (NULL != rsp_data)) {
        rng_cmd_data = (const mb_cmd_rng_st *)(req_data);
        ret = rng_check_alg(rng_cmd_data->algorithm);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            if (0U == rng_cmd_data->random_data_addr) {
                ret = EHSM_ERR_PARAM_ERROR;
            }
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            host_addr = rng_cmd_data->random_data_addr;
            remain_sz = rng_cmd_data->require_size;
            while (remain_sz > 0U) {
                once_sz = ((remain_sz > RNG_SRV_DATA_BUF_BYTE_SIZE) ? RNG_SRV_DATA_BUF_BYTE_SIZE : remain_sz);
                ret = cpt_get_rand(rng_data_buf, once_sz);
                if (EHSM_ERR_SW_SUCCESS == ret) {
                    ret = mmap_write_remote_data(host_addr, rng_data_buf, once_sz);
                }
                if (EHSM_ERR_SW_SUCCESS == ret) {
                    remain_sz -= once_sz;
                    host_addr += (uint64_t)once_sz;
                    util_memset(rng_data_buf, 0U, sizeof(rng_data_buf));
                } else {
                    remain_sz = 0U;
                    util_memset(rng_data_buf, 0U, sizeof(rng_data_buf));
                    // Hold the return code of write_host_data()
                }
            }
        }
        // No need to response data
        rsp_data->rsp_data_len = 0U;
    } else {
        ret = EHSM_ERR_PARAM_ERROR;
    }

    return ret;
}
