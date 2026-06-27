#ifndef DBGAUTH_SRV_H
#define DBGAUTH_SRV_H
/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "types.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
/* 32 bytes of random number, 16 bytes of UID */
#define EHSM_DEBUG_CHALLENGE_SIZE (32U + 16U)
/* 16 bytes of random number*/
#define EHSM_FW_AUTH_CHALLENGE_SIZE (16U)

#define EHSM_SHE_DEBUG_SIGN_SIZE          (16U)
#define EHSM_DBG_AUTH_CMAC_SIGNATURE_SIZE (16U)

/*Definition the invalid challeng type*/
#define EHSM_CHALLENGE_TYPE_INVALID 0x00U

/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/
/*Definition the debug auth information struction*/
typedef struct {
    uint8_t alg;
    uint8_t type;
    uint32_t signature_size;
    uint8_t *signature;
    uint32_t public_key_size;
    uint8_t *public_key;
    uint32_t soc_dbg_bitmap[5]; // bit 1 enable operate bit 0 disable operate, the word of soc_en_bitmap[4] only the
                                // lowest bit is valid
} ehsm_debug_auth_st;

/***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t dbgauth_srv_handler(const cmd_req_data_st *req_data, cmd_rsp_data_st *rsp_data);

uint32_t dbgauth_ehsm_debug_auth(uint8_t challenge_type, ehsm_debug_auth_st *debug_auth_st);

uint32_t dbgauth_close_debug(uint8_t type);

uint32_t dbgauth_get_challenge(uint8_t challenge_type, uint8_t *challenge_buf);

uint32_t dbgauth_read_challenge(uint8_t type, uint8_t *challenge_buf);
#endif /* DBGAUTH_SRV_H */