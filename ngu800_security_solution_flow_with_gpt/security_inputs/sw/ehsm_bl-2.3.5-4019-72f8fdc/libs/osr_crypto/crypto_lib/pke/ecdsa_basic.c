#include "../../crypto_include/pke_config.h"


#ifdef SUPPORT_ECDSA

#include "../../crypto_include/pke/ecdsa.h"
#include "../../crypto_include/crypto_common/utility.h"
#include "../../crypto_include/trng/trng.h"



/* function: Generate big number e_bn in U32 little-endian style
 * parameters:
 *     n_bits --------------------- input, bit length of ecc curve parameter n(order of base point)
 *     n_bytes -------------------- input, byte length of ecc curve parameter n(order of base point)
 *     n_words -------------------- input, word length of ecc curve parameter n(order of base point)
 *     e -------------------------- input, e, could be the hash digest, U8 big-endian style
 *     e_bytes -------------------- input, byte length of e
 *     e_bn ----------------------- output, big number e in U32 little-endian style
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. 
 */
uint32_t ecdsa_get_e_bn(uint32_t n_bits, uint32_t n_bytes, uint32_t n_words, 
        const uint8_t *e, uint32_t e_bytes, uint32_t *e_bn)
{
    uint32_t ret;
    uint32_t tmpLen, e_real_bytes = e_bytes;

    if(NULL == e_bn)      //e could be NULL, means e_bytes is 0u
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
        //e could be zero
        if(NULL == e)
        {
            e_real_bytes = 0;
        }
        else
        {}
        
        //get integer e_bn from hash value E(according to SEC1-V2 2009)
        uint32_clear(e_bn, n_words);
        if(n_bits >= (e_real_bytes<<3))  //in this case, make e as e_bn directly
        {
            reverse_byte_array(e, (uint8_t *)e_bn, e_real_bytes);
        }
        else                        //in this case, make left n_bits bits of e as e_bn
        {
            reverse_byte_array(e, (uint8_t *)e_bn, n_bytes);
            tmpLen = (n_bits)&7u;
            if(0u != tmpLen)
            {
                (void)Big_Div2n(e_bn, n_words, 8u-tmpLen);
            }
            else
            {}
        }

        ret = PKE_SUCCESS;
    }

    return ret;
}

#endif

