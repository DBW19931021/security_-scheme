
#include "../../crypto_include/pke_config.h"


#if (defined(SUPPORT_RSASSA_PSS) || defined(SUPPORT_RSAES_OAEP))

#include "../../crypto_include/pke/rsa.h"
#include "../../crypto_include/hash_hmac/hash_kdf.h"
#include "../../crypto_include/crypto_common/utility.h"




/* function: RSA PKCS#1_v2.2 MGF1(a mask generation function based on a hash function)
 * parameters:
 *     hash_alg ------------------- input, specific hash algorithm for MGF1
 *     seed ----------------------- input, seed
 *     seed_bytes ----------------- input, byte length of seed
 *     in ------------------------- input, this is to XOR mask, and this could be NULL
 *     out ------------------------ output, if in is NULL, this is mask directly, otherwise,
 *                                  this is (mask XOR in).
 *     mask_bytes ----------------- input, if in is NULL, this is byte length of out(mask), otherwise,
 *                                  this is byte length of in or out(mask XOR in).
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. out = mask XOR in, if in is NULL, out is mask directly.
 */
uint32_t rsa_pkcs1_mgf1_with_xor_in(hash_alg_e hash_alg, const uint8_t *seed, uint32_t seed_bytes, 
        const uint8_t *in, uint8_t *out, uint32_t mask_bytes)
{
    uint8_t counter_buf[4] = {0,0,0,0};
    const uint8_t *counter = counter_buf;
    uint32_t ret;
#if 0
    hash_node_st digest_node[2] = 
    {
        {seed, seed_bytes},
        {counter, 4u},
    };
#else
    hash_node_st digest_node[2];

    digest_node[0].msg_addr  = seed;
    digest_node[0].msg_bytes = seed_bytes;
    digest_node[1].msg_addr  = counter;
    digest_node[1].msg_bytes = 4u;
#endif

    ret = ansi_x9_63_kdf_node_with_xor_in(hash_alg, digest_node, 2, counter_buf, in, out, mask_bytes, 1);
    if(HASH_SUCCESS == ret)
    {
        ret = RSA_SUCCESS;
    }
    else
    {}

    return ret;
}

#endif

