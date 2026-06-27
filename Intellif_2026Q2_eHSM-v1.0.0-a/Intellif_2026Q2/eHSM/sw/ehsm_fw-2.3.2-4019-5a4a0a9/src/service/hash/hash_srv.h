#ifndef HASH_SRV_H
#define HASH_SRV_H

/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "types.h"
#include "component/crypto_api.h"
/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
// The size of data buffer used in CPU mode
#define HASH_SRV_DATA_BUF_BYTE_SIZE (512U)

/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/
typedef enum {
    HASH_DMA_MODE = 0U,
    HASH_CPU_MODE = 1U,
} hash_mode_e;

typedef enum {
    DIGEST_KEEP_IN_LOCAL = 0U,
    DIGEST_WRITE_TO_HOST = 1U,
} hash_output_mode_e;
/***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/

/**
 *   @brief      Configure the DMA as AXI-read and AHB-read
 *
 *
 *   @return     uint32_t
 *
 *   @note
 */
uint32_t hash_srv_using_ahb_dma(void);

/**
 *   @brief      Recovery the DMA register
 *
 *   @param [in] origin_val
 *
 *
 *   @note
 */
void hash_srv_release_ahb_dma(uint32_t origin_val);

/**
 *   @brief      Hash update service in CPU mode
 *
 *   @param [in] ctx_buf_addr
 *   @param [in] msg_addr
 *   @param [in] input_sz
 *
 *   @return     uint32_t
 *
 *   @note
 */
uint32_t hash_srv_cpu_update(raddr_t ctx_buf_addr, raddr_t msg_addr, uint32_t input_sz);

/**
 *   @brief      Hash update service in DMA mode
 *
 *   @param [in] ctx_buf_addr
 *   @param [in] msg_addr
 *   @param [in] input_sz
 *
 *   @return     uint32_t
 *
 *   @note
 */
uint32_t hash_srv_dma_update(raddr_t ctx_buf_addr, raddr_t msg_addr, uint32_t input_sz);

/**
 *   @brief      Hash general stepwise initialization service
 *
 *   @param [in] ctx_buf_addr
 *   @param [in] alg
 *
 *   @return     uint32_t
 *
 *   @note
 */
uint32_t hash_srv_general_init(raddr_t ctx_buf_addr, cpt_hash_alg_e alg);

/**
 *   @brief      Hash general stepwise update service
 *
 *   @param [in] ctx_buf_addr
 *   @param [in] msg_addr
 *   @param [in] input_sz
 *
 *   @return     uint32_t
 *
 *   @note
 */
uint32_t hash_srv_general_update(raddr_t ctx_buf_addr, raddr_t msg_addr, uint32_t input_sz);

/**
 *   @brief      Hash general stepwise final service
 *
 *   @param [in] ctx_buf_addr
 *   @param [in] msg_addr
 *   @param [in] input_sz
 *   @param [in] digest
 *   @param [in] digest_sz
 *
 *   @return     uint32_t
 *
 *   @note
 */
uint32_t hash_srv_general_final(
    raddr_t ctx_buf_addr, raddr_t msg_addr, uint32_t input_sz, uint8_t *digest, uint32_t *digest_sz);

/**
 *   @brief      Hash general single-call service
 *
 *   @param [in] alg
 *   @param [in] msg_addr
 *   @param [in] input_sz
 *   @param [in] digest_raddr
 *   @param [in] digest_lptr
 *   @param [in] digest_sz
 *   @param [in] mode
 *
 *   @return     uint32_t
 *
 *   @note
 */
uint32_t hash_srv_general_onepass(cpt_hash_alg_e alg, raddr_t msg_addr, uint32_t input_sz, raddr_t digest_raddr,
    uint8_t *digest_lptr, uint32_t *digest_sz, hash_output_mode_e mode);
/**
 *   @brief      Initialize the hash service
 *
 *
 *   @return     uint32_t
 *
 *   @note
 */
uint32_t hash_srv_init(void);

/**
 *   @brief      Hash service handler
 *
 *   @param [in] req_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
uint32_t hash_srv_handler(const cmd_req_data_st *req_data, cmd_rsp_data_st *rsp_data);

#endif /* HASH_SRV_H */
