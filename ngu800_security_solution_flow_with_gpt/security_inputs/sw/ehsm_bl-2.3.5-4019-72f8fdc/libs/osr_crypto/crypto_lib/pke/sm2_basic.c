
#include "../../crypto_include/pke_config.h"


#ifdef SUPPORT_SM2

#include "./sm2_internal.h"
#include "../../crypto_include/pke/sm2.h"
#include "../../crypto_include/trng/trng.h"
#include "../../crypto_include/crypto_common/utility.h"
#ifdef PKE_SEC
#include "../../crypto_include/crypto_common/utility_sec.h"
#endif




#define SM2_DEFAULT_ID_BYTE_LEN         (16u)
const uint8_t g_sm2_default_id[16] = {0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38};



/* function: get SM2 Z value = SM3(bitLenofID||ID||a||b||Gx||Gy||Px||Py), internla API
 * parameters:
 *     id ------------------------- input, User ID
 *     id_bytes ------------------- input, byte length of ID, must be less than 2^13
 *     pubKey --------------------- input, public key(0x04 + x + y), 65 bytes, big-endian
 *     Z -------------------------- output, Z value, SM3 digest, 32 bytes
 * return:
 *     HASH_SUCCESS(success); other(error)
 * caution:
 *     1. id can not be NULL
 *     2. id_byrtes can not be zero, and bit length of id must be less than 2^16, thus 
 *        id_byrtes must be less than 2^13
 *     3. please make sure the pubKey is valid
 */
FLAG_STATIC uint32_t sm2_getZ_internal(const uint8_t *id, uint32_t id_bytes, const uint8_t pubKey[65], 
        uint8_t Z[32])
{
    uint32_t ret;
    uint32_t tmp[SM2_WORD_LEN<<1];
    hash_ctx_st ctx[1];
    uint8_t tmp_u8[2];

    u8big_to_u32little_256bits(&pubKey[1u], tmp);
    u8big_to_u32little_256bits(&pubKey[1u+SM2_BYTE_LEN], &tmp[SM2_WORD_LEN]);
    ret = eccp_check_point(sm2_curve, tmp, &tmp[SM2_WORD_LEN]);
    if(PKE_SUCCESS == ret)
    {
        ret = hash_init(ctx, HASH_SM3);
    }
    else
    {}

    if(HASH_SUCCESS == ret)
    {
        tmp_u8[0] = (uint8_t)((id_bytes>>5u) & 0xFFu);
        tmp_u8[1] = (uint8_t)((id_bytes<<3u) & 0xFFu);
        ret = hash_update(ctx, (uint8_t *)&tmp_u8, 2u);
    }
    else
    {}

    if(HASH_SUCCESS == ret)
    {
        ret = hash_update(ctx, id, id_bytes);
    }
    else
    {}

    if(HASH_SUCCESS == ret)
    {
        u8big_to_u32little_256bits((const uint8_t *)sm2_curve->eccp_a, tmp);
        u8big_to_u32little_256bits((const uint8_t *)sm2_curve->eccp_b, &tmp[SM2_WORD_LEN]);
        ret = hash_update(ctx, (uint8_t *)tmp, SM2_BYTE_LEN<<1);
    }
    else
    {}

    if(HASH_SUCCESS == ret)
    {
        u8big_to_u32little_256bits((const uint8_t *)sm2_curve->eccp_Gx, tmp);
        u8big_to_u32little_256bits((const uint8_t *)sm2_curve->eccp_Gy, &tmp[SM2_WORD_LEN]);
        ret = hash_update(ctx, (uint8_t *)tmp, SM2_BYTE_LEN<<1);
    }
    else
    {}

    if(HASH_SUCCESS == ret)
    {
        ret = hash_update(ctx, &pubKey[1u], SM2_BYTE_LEN<<1);
    }
    else
    {}

    if(HASH_SUCCESS == ret)
    {
        ret = hash_final(ctx, Z);
    }
    else
    {}

    return ret;
}


/* function: get SM2 Z value = SM3(bitLenofID||ID||a||b||Gx||Gy||Px||Py)
 * parameters:
 *     ID ------------------------- input, User ID
 *     byteLenofID ---------------- input, byte length of ID, must be less than 2^13
 *     pubKey --------------------- input, public key(0x04 + x + y), 65 bytes, big-endian
 *     Z -------------------------- output, Z value, SM3 digest, 32 bytes
 * return:
 *     SM2_SUCCESS(success); other(error)
 * caution:
 *     1. bit length of ID must be less than 2^16, thus byte length must be less than 2^13
 *     2. if ID is NULL, then replace it with sm2 default ID
 *     3. please make sure the pubKey is valid
 */
uint32_t sm2_getZ(const uint8_t *ID, uint32_t byteLenofID, const uint8_t pubKey[65], uint8_t Z[32])
{
    uint32_t ret;
    const uint8_t *id_p = ID;
    uint32_t id_bytes = byteLenofID;

    if((NULL == pubKey) || (NULL == Z))
    {
        ret = SM2_BUFFER_NULL;
    }
    else if(POINT_UNCOMPRESSED != pubKey[0])
    {
        ret = SM2_INPUT_INVALID;
    }
    else if(id_bytes > SM2_MAX_ID_BYTE_LEN)
    {
        ret = SM2_INPUT_INVALID;
    } 
    else
    {
        if((NULL == ID) || (0u == id_bytes))
        {
            id_p = g_sm2_default_id;
            id_bytes = SM2_DEFAULT_ID_BYTE_LEN;
        }
        else
        {}

        ret = sm2_getZ_internal(id_p, id_bytes, pubKey, Z);
        if(HASH_SUCCESS == ret)
        {
            ret = SM2_SUCCESS;
        }
        else
        {}
    }

    return ret;
}


/* function: get SM2 E value = SM3(Z||M) (one-off style)
 * parameters:
 *     M      --------------------- input, Message
 *     byteLen -------------------- input, byte length of M
 *     Z      --------------------- input, Z value, 32 bytes
 *     E      --------------------- output, E value, 32 bytes
 * return:
 *     SM2_SUCCESS(success); other(error)
 * caution:
 */
uint32_t sm2_getE(const uint8_t *M, uint32_t byteLen, const uint8_t Z[32], uint8_t E[32])
{
    hash_ctx_st ctx[1];
    uint32_t ret;

    if((NULL == M) || (NULL == Z) || (NULL == E))
    {
        ret = SM2_BUFFER_NULL;
    }
    else if(0u == byteLen)
    {
        ret= SM2_INPUT_INVALID;
    }
    else
    {
        ret = hash_init(ctx, HASH_SM3);
        if(HASH_SUCCESS == ret)
        {
            ret = hash_update(ctx, Z, 32u);
        }
        else
        {}

        if(HASH_SUCCESS == ret)
        {
            ret = hash_update(ctx, M, byteLen);
        }
        else
        {}

        if(HASH_SUCCESS == ret)
        {
            ret = hash_final(ctx, E);
        }
        else
        {}

        if(HASH_SUCCESS == ret)
        {
            ret = SM2_SUCCESS;
        }
        else
        {}
    }

    return ret;
}


#ifdef SM2_GETE_BY_STEPS
/* function: step 1 of getting SM2 E value(stepwise style), init
 * parameters:
 *     ctx ------------------------ input, hash_ctx_st context pointer
 *     Z -------------------------- input, Z value, 32 bytes
 * return: SM2_SUCCESS(success), other(error)
 * caution:
 */
uint32_t sm2_getE_init(hash_ctx_st *ctx, const uint8_t Z[32])
{
    uint32_t ret;

    ret = hash_init(ctx, HASH_SM3);
    if(HASH_SUCCESS == ret)
    {
        ret = hash_update(ctx, Z, 32u);
        if(HASH_SUCCESS == ret)
        {
            ret = SM2_SUCCESS;
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: step 2 of getting SM2 E value(stepwise style), update message
 * parameters:
 *     ctx ------------------------ input, hash_ctx_st context pointer
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the input message
 * return: SM2_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the three parameters are valid, and ctx is initialized
 */
uint32_t sm2_getE_update(hash_ctx_st *ctx, const uint8_t *msg, uint32_t msg_bytes)
{
    uint32_t ret;

    ret = hash_update(ctx, msg, msg_bytes);
    if(HASH_SUCCESS == ret)
    {
        ret = SM2_SUCCESS;
    }
    else
    {}

    return ret;
}


/* function: step 3 of getting SM2 E value(stepwise style), message update done, get the digest(SM2 E value)
 * parameters:
 *     ctx ------------------------ input, hash_ctx_st context pointer
 *     E -------------------------- output, hash digest, SM2 E value
 * return: SM2_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the ctx is valid and initialized
 *     2. please make sure the digest buffer E is sufficient
 */
uint32_t sm2_getE_final(hash_ctx_st *ctx, uint8_t E[32])
{
    uint32_t ret;

    ret = hash_final(ctx, E);
    if(HASH_SUCCESS == ret)
    {
        ret = SM2_SUCCESS;
    }
    else
    {}

    return ret;
}
#endif


#endif

