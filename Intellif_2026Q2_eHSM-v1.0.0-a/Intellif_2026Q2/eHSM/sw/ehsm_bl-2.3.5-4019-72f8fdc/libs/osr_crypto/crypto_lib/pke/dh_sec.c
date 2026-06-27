
#include "../../crypto_include/pke_config.h"


#if (defined(SUPPORT_DH) && defined(DH_SEC))

#include "../../crypto_include/pke/dh.h"
#include "../../crypto_include/crypto_common/utility_sec.h"
#include "../../crypto_include/trng/trng.h"



/* Function: DH compute key(secure version)
 * Parameters:
 *     dh_para -------------------- input, dh_para_st struct pointer
 *     local_prikey --------------- input, local private key, big-endian
 *     peer_pubkey ---------------- input, peer public key, big-endian
 *     key ------------------------ output, output key
 * Return:
 *     DH_SUCCESS(success); other(error)
 * Caution:
 *     1. local_prikey occupies (dh_para->q_bits+7)/8 bytes
 *     2. peer_pubkey and key occupy (dh_para->p_bits+7)/8 bytes
 */
uint32_t dh_compute_key_s(const dh_para_st *dh_para, const uint8_t *local_prikey, 
        const uint8_t *peer_pubkey, uint8_t *key)
{
    uint32_t tmp[DH_MAX_WORD_LEN];
    uint32_t prikey[DH_MAX_WORD_LEN];
    uint32_t pubkey[DH_MAX_WORD_LEN];
    uint32_t pByteLen = 0u, qByteLen = 0u, pWordLen = 0u, qWordLen = 0u;
    uint32_t ret;

    if((NULL == dh_para) || (NULL == local_prikey) || (NULL == peer_pubkey) || (NULL == key))
    {
        ret = DH_ERROR_S;
    }
    else
    {
        pByteLen = GET_BYTE_LEN(dh_para->p_bits);
        qByteLen = GET_BYTE_LEN(dh_para->q_bits);
        pWordLen = GET_WORD_LEN(dh_para->p_bits);
        qWordLen = GET_WORD_LEN(dh_para->q_bits);

#ifdef SUPPORT_STATIC_ANALYSIS
        if(qWordLen > 0u)   //just for static analysis.
        {
#endif
            prikey[qWordLen - 1u] = 0u;
            reverse_byte_array(local_prikey, (uint8_t *)prikey, qByteLen);
#ifdef SUPPORT_STATIC_ANALYSIS
        }
        else
        {}
#endif

        //get tmp = q-1
        uint32_copy(tmp, dh_para->q, qWordLen);
#ifdef SUPPORT_STATIC_ANALYSIS
        if(tmp[0u] > 0u)   //just for static analysis.
        {
#endif
            tmp[0u] -= 1u;
#ifdef SUPPORT_STATIC_ANALYSIS
        }
        else
        {}
#endif

        //make sure private key is in [2, q-2]
        if(get_valid_bits(prikey, qWordLen) <= 1u)
        {
            ret = DH_ERROR_S;
        }
        else if(uint32_BigNumCmp(prikey, qWordLen, tmp, qWordLen) >= 0)
        {
            ret = DH_ERROR_S;
        }
        else
        {
            ret = PKE_SUCCESS;
        }
    }

    if(PKE_SUCCESS == ret)
    {
        //get tmp = p-1
        uint32_copy(tmp, dh_para->p, pWordLen);
        tmp[0u] -= 1u;

        if(pWordLen >= 1u)   //just for static analysis.
        {
            pubkey[pWordLen - 1u] = 0u;
            reverse_byte_array(peer_pubkey, (uint8_t *)pubkey, pByteLen);
        }
        else
        {}

        //check public key
        ret = dh_check_public_key(dh_para, tmp, pubkey);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = pke_modexp_ladder_internal(prikey, pubkey, tmp, pWordLen, qWordLen);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        reverse_byte_array((uint8_t *)tmp, (uint8_t *)key, pByteLen);
        ret = DH_SUCCESS_S;
    }
    else
    {
        ret = DH_ERROR_S;
    }

    (void)get_rand_fast((uint8_t *)tmp, pByteLen);
    (void)get_rand_fast((uint8_t *)prikey, qByteLen);
    (void)get_rand_fast((uint8_t *)pubkey, pByteLen);

    return ret;
}

#endif

