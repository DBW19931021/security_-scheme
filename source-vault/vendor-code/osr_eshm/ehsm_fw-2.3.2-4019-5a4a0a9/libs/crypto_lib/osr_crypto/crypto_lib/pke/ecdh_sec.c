
#include "../../crypto_include/pke_config.h"


#if (defined(PKE_SEC) && defined(ECDH_SEC))

#include "../../crypto_include/pke/ecdh.h"
#include "../../crypto_include/crypto_common/utility_sec.h"
#include "../../crypto_include/trng/trng.h"
#include "eccp_sec_common.h"



/* Function: ECDH compute key(internal interface)
 * Parameters:
 *     ctx ------------------------ input, eccp_sec_ctx_t struct pointer
 *     local_prikey --------------- input, local private key, big-endian
 *     peer_pubkey ---------------- input, peer public key, big-endian
 *     key ------------------------ output, output key
 *     keyByteLen ----------------- input, byte length of output key
 *     KDF ------------------------ input, KDF function to get key
 * Return:
 *     ECDH_SUCCESS_S(success); other(error)
 * Caution:
 *     1. ctx must be initialized by eccp_curve_init() before calling this function().
 */
static uint32_t ecdh_compute_key_s_internal(eccp_sec_ctx_t *ctx, const uint8_t *local_prikey, 
        const uint8_t *peer_pubkey, uint8_t *key, uint32_t keyByteLen, KDF_FUNC kdf)
{
    uint32_t ret = ECDH_ERROR_S, tmp_step = 0u;
    uint32_t k[ECCP_MAX_WORD_LEN], Px[ECCP_MAX_WORD_LEN], Py[ECCP_MAX_WORD_LEN];
    uint32_t pByteLen = 0u, pWordLen, nByteLen = 0u, nWordLen;
    uint32_t *curve_b, *curve_p, *curve_p_h;
    const eccp_curve_st * curve1;
    
    if((NULL == local_prikey) || (NULL == peer_pubkey) || (NULL == key) || (0u == keyByteLen))
    {}
    else
    {
        curve1 = ctx->curve;

        pByteLen = GET_BYTE_LEN(curve1->eccp_p_bitLen);
        pWordLen = GET_WORD_LEN(curve1->eccp_p_bitLen);
        nByteLen = GET_BYTE_LEN(curve1->eccp_n_bitLen);
        nWordLen = GET_WORD_LEN(curve1->eccp_n_bitLen);

        //actually the following pointers are the same as the corresponding fields of curve1
        curve_p   = ctx->eccp_curve_mem;
        curve_p_h = &(curve_p[pWordLen]);
        curve_b   = &(curve_p_h[pWordLen<<1]);

        if((NULL == kdf) && (keyByteLen > pByteLen))
        {}
#ifdef SUPPORT_STATIC_ANALYSIS   //just for static analysis.
        else if((0u == nWordLen) || (0u == pWordLen))
        {}
#endif
        else
        {
            ret = PKE_SUCCESS;
        }
    }

    if(PKE_SUCCESS == ret)
    {
        //make sure private key is in [1, n-1]
        k[nWordLen - 1u] = 0u;
        reverse_byte_array(local_prikey, (uint8_t *)k, nByteLen);
        ret = uint32_integer_check_sec(k, curve1->eccp_n, nWordLen, ECDH_ERROR_S, ECDH_ERROR_S,
                ECDH_SUCCESS_S);
        if(ECDH_SUCCESS_S == ret)
        {
            //check public key
            Px[pWordLen - 1u] = 0u;
            Py[pWordLen - 1u] = 0u;
            reverse_byte_array(peer_pubkey, (uint8_t *)Px, pByteLen);
            reverse_byte_array(&peer_pubkey[pByteLen], (uint8_t *)Py, pByteLen);
            ret = eccp_check_point(curve1, Px, Py);
        }
        else
        {}
            
        if(PKE_SUCCESS == ret)
        {
            tmp_step = pke_get_operand_bytes();

            //copy back curve paras that not covered by output
            uint32_copy(curve_b,   (uint32_t *)(rPKE_A(4u,tmp_step)), pWordLen);
            uint32_copy(curve_p,   (uint32_t *)(rPKE_A(0u,tmp_step)), pWordLen);
            uint32_copy(curve_p_h, (uint32_t *)(rPKE_B(0u,tmp_step)), pWordLen);

            ret = eccp_pointMul_sec(curve1, k, Px, Py, Px, NULL);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            //copy back curve paras that not covered by output
            uint32_copy(curve_p,   (uint32_t *)(rPKE_A(0u,tmp_step)), pWordLen);
            uint32_copy(curve_p_h, (uint32_t *)(rPKE_B(0u,tmp_step)), pWordLen);

            reverse_byte_array((uint8_t *)Px, (uint8_t *)Px, pByteLen);

            if(NULL != kdf)
            {
                (void)kdf(Px, pByteLen, key, keyByteLen);
            }
            else
            {
                memcpy_(key, (uint8_t *)Px, keyByteLen);
            }
                
            ret = ECDH_SUCCESS_S;
        }
        else
        {
            ret = ECDH_ERROR_S;
        }
    }
    else
    {}

    (void)get_rand_fast((uint8_t *)k, nByteLen);
    (void)get_rand_fast((uint8_t *)Px, pByteLen);
    (void)get_rand_fast((uint8_t *)Py, pByteLen);

    return ret;      
}


/* Function: ECDH compute key
 * Parameters:
 *     local_prikey --------------- input, local private key, big-endian
 *     peer_pubkey ---------------- input, peer public key, big-endian
 *     key ------------------------ output, output key
 *     keyByteLen ----------------- input, byte length of output key
 *     KDF ------------------------ input, KDF function to get key
 * Return:
 *     ECDH_SUCCESS_S(success); other(error)
 * Caution:
 */
uint32_t ecdh_compute_key_s(const eccp_curve_st *curve, const uint8_t *local_prikey, 
        const uint8_t *peer_pubkey, uint8_t *key, uint32_t keyByteLen, KDF_FUNC kdf)
{
    uint32_t ret = ECDH_ERROR_S;
    uint32_t k_bytes = keyByteLen;
    const eccp_curve_st * curve1;
    eccp_sec_ctx_t ctx[1];
    uint16_t eccp_curve_crc16;

    if(NULL == curve)
    {}
    else
    {
        //init curve
        curve1 = eccp_curve_init(ctx, curve);
        if(NULL != curve1)
        {
            //check crc16 of curve paras
            eccp_curve_crc16 = calc_eccp_curve_crc16(curve);
            if(0u == ecc_crc16_check(curve1, eccp_curve_crc16))
            {
                ret = ecdh_compute_key_s_internal(ctx, local_prikey, peer_pubkey, key, k_bytes, kdf);
            }
            else
            {}

            if(ECDH_SUCCESS_S == ret)
            {
                //check crc16 of curve paras
                if(0u != ecc_crc16_check(curve1, eccp_curve_crc16))
                {
                    ret = ECDH_ERROR_S;
                }
                else
                {}
            }
            else
            {
                ret = ECDH_ERROR_S;
            }
        }
        else
        {}
    }

    eccp_curve_uninit(ctx);
    if(ECDH_SUCCESS_S != ret)
    {
        (void)get_rand_fast((uint8_t *)key, k_bytes);
    }
    else
    {}

    return ret;
}
#endif

