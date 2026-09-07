
#include "../../crypto_include/pke_config.h"


#ifdef SUPPORT_ECDH

#include "../../crypto_include/pke/ecdh.h"
#include "../../crypto_include/crypto_common/utility.h"




/* Function: ECDH compute key
 * Parameters:
 *     local_prikey --------------- input, local private key, big-endian
 *     peer_pubkey ---------------- input, peer public key, big-endian
 *     key ------------------------ output, output key
 *     keyByteLen ----------------- input, byte length of output key
 *     KDF ------------------------ input, KDF function to get key
 * Return:
 *     ECDH_SUCCESS(success); other(error)
 * Caution:
 */
uint32_t ecdh_compute_key(const eccp_curve_st *curve, const uint8_t *local_prikey, 
        const uint8_t *peer_pubkey, uint8_t *key, uint32_t keyByteLen, KDF_FUNC kdf)
{
    uint32_t k[ECCP_MAX_WORD_LEN], Px[ECCP_MAX_WORD_LEN], Py[ECCP_MAX_WORD_LEN];
    uint32_t pByteLen, pWordLen, nByteLen, nWordLen;
    uint32_t ret = ECDH_SUCCESS;

    if((NULL == curve) || (NULL == local_prikey) || (NULL == peer_pubkey) || (NULL == key))
    {
        ret = ECDH_POINTOR_NULL;
    }
    else if(0u == keyByteLen)
    {
        ret = ECDH_INVALID_INPUT;
    }
    else
    { 
    	pWordLen = GET_WORD_LEN(curve->eccp_p_bitLen);
    	nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);       
    }

    
#ifdef SUPPORT_STATIC_ANALYSIS
    if((ECDH_SUCCESS == ret) && ((nWordLen > 0u) && (pWordLen > 0u)))   //just for static analysis.
#else
    if(ECDH_SUCCESS == ret)   //just for static analysis.
#endif
    {
        pByteLen = GET_BYTE_LEN(curve->eccp_p_bitLen);
        nByteLen = GET_BYTE_LEN(curve->eccp_n_bitLen);

        //make sure private key is in [1, n-1]
        k[nWordLen - 1u] = 0u;
        reverse_byte_array(local_prikey, (uint8_t *)k, nByteLen);
        ret = uint32_integer_check(k, curve->eccp_n, nWordLen, ECDH_ZERO_ALL, ECDH_INTEGER_TOO_BIG,
            ECDH_SUCCESS);
        if(ECDH_SUCCESS == ret)
        {
            //check public key
            Px[pWordLen - 1u] = 0u;
            Py[pWordLen - 1u] = 0u;
            reverse_byte_array(peer_pubkey, (uint8_t *)Px, pByteLen);
            reverse_byte_array(&peer_pubkey[pByteLen], (uint8_t *)Py, pByteLen);
            ret = eccp_check_point(curve, Px, Py);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            ret = eccp_pointMul(curve, k, Px, Py, Px, NULL);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            reverse_byte_array((uint8_t *)Px, (uint8_t *)Px, pByteLen);

            if(NULL != kdf)
            {
                (void)kdf(Px, pByteLen, key, keyByteLen);
            }
            else
            {
                if(keyByteLen > pByteLen)
                {
                    memcpy_(key, (uint8_t *)Px, pByteLen);
                }
                else
                {
                    memcpy_(key, (uint8_t *)Px, keyByteLen);
                }
            }

            ret = ECDH_SUCCESS;
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {
        ret = ECDH_INVALID_INPUT;
    }
#else
    }
#endif

    return ret;
}

#endif

