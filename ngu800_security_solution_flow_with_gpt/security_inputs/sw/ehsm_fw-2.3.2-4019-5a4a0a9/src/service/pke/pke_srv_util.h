#ifndef PKE_SRV_UTIL_H
#define PKE_SRV_UTIL_H

/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "types.h"
#include "hash/hash_srv.h"
/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/

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
 *   @brief      PKE digest-calcaultion init service
 *   
 *   @param [in] ctx_buf_addr
 *   @param [in] alg
 *   
 *   @return     uint32_t
 *   
 *   @note
 */
uint32_t pke_srv_init(raddr_t ctx_buf_addr, cpt_hash_alg_e alg);

/**
 *   @brief      PKE digest-calcaultion update service in CPU mode
 *   
 *   @param [in] ctx_buf_addr
 *   @param [in] msg_addr
 *   @param [in] input_sz
 *   
 *   @return     uint32_t
 *   
 *   @note
 */
uint32_t pke_srv_cpu_update(raddr_t ctx_buf_addr, raddr_t msg_addr, uint32_t input_sz);


/**
 *   @brief      PKE digest-calcaultion update service in DMA mode
 *   
 *   @param [in] ctx_buf_addr
 *   @param [in] msg_addr
 *   @param [in] input_sz
 *   
 *   @return     uint32_t
 *   
 *   @note
 */
uint32_t pke_srv_dma_update(raddr_t ctx_buf_addr, raddr_t msg_addr, uint32_t input_sz);


/**
 *   @brief      PKE digest-calcaultion update service
 *   
 *   @param [in] ctx_buf_addr
 *   @param [in] msg_addr
 *   @param [in] input_sz
 *   
 *   @return     uint32_t
 *   
 *   @note
 */
uint32_t pke_srv_update(raddr_t ctx_buf_addr, raddr_t msg_addr, uint32_t input_sz);


/**
 *   @brief      PKE digest-calcaultion final service
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
uint32_t pke_srv_hash_final(raddr_t ctx_buf_addr, raddr_t msg_addr, uint32_t input_sz, uint8_t *digest, uint32_t *digest_sz);


/**
 *   @brief      PKE digest-calcaultion single-call service
 *   
 *   @param [in] alg
 *   @param [in] msg_addr
 *   @param [in] input_sz
 *   @param [in] digest
 *   @param [in] digest_sz
 *   
 *   @return     uint32_t
 *   
 *   @note
 */
uint32_t pke_srv_hash_onepass(cpt_hash_alg_e alg, raddr_t msg_addr, uint32_t input_sz, uint8_t *digest, uint32_t *digest_sz);

#endif /* PKE_SRV_UTIL_H */
