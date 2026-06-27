
#include "../../crypto_include/pke_config.h"


#ifdef SUPPORT_C25519

#include "../../crypto_include/pke/x25519.h"
#include "../../crypto_include/crypto_common/utility.h"
#include "../../crypto_include/trng/trng.h"


/* Function: decode X25519 u coordinate for point multiplication
 * Parameters:
 *     u -------------------------- input,
 *     p -------------------------- input, modulus in little-endian
 *     out ------------------------ output, big scalar in little-endian
 *     bytes ---------------------- input, byte length of u, p and out
 * Return: none
 * Caution:
 *     1. all operands are of 256 bits for X25519.
 */
static uint32_t x25519_decode_u(const uint8_t *u, const uint32_t *p, uint32_t *out)
{
    uint32_t ret;
    uint8_t *out_u8 = (uint8_t *)out;

    if(u != ((uint8_t *)out))
    {
        memcpy_(out_u8, u, 32u);
    }
    else
    {}

    out_u8[32u - 1u] &= (uint8_t)0x7F;    //clear highest bit

    //mod p
    if(uint32_BigNumCmp((uint32_t *)out, 8u, p, 8u) >= 0)
    {
#if 0
        ret = pke_sub((uint32_t *)out, p, (uint32_t *)out, 8u);
#else
        ret = pke_mod_add_sub_mul_256bits_internal((uint32_t *)out, p, (uint32_t *)out, MICROCODE_INTSUB);
#endif
    }
    else
    {
        ret = PKE_SUCCESS;
    }

    return ret;
}


/* Function: get X25519 public key from private key
 * Parameters:
 *     prikey --------------------- input, private key, 32 bytes, little-endian
 *     pubkey --------------------- output, public key, 32 bytes, little-endian
 * Return: X25519_SUCCESS(success); other(error)
 * caution:
 */
uint32_t x25519_get_pubkey_from_prikey(const uint8_t prikey[32], uint8_t pubkey[32])
{
    uint32_t t[C25519_WORD_LEN];
    uint32_t ret;

    if((NULL == prikey) || (NULL == pubkey))
    {
        ret = X25519_POINTER_NULL;
    }
    else
    {
        x25519_ed25519_decode_scalar(prikey, (uint8_t *)t);

        //it could be proved that here t is not a multiple of c25519->n, so no need to compare
        //(t mod c25519->n) with c25519->n

        ret = x25519_pointMul(c25519, t, c25519->u, t);
        if(PKE_SUCCESS == ret)
        {
            memcpy_(pubkey, (uint8_t *)t, C25519_BYTE_LEN);
            ret = X25519_SUCCESS;
        }
        else
        {}
    }

    return ret;
}


/* Function: get x25519 random key pair
 * Parameters:
 *     prikey --------------------- output, private key, 32 bytes, little-endian
 *     pubkey --------------------- output, public key, 32 bytes, little-endian
 * Return: X25519_SUCCESS(success); other(error)
 * caution:
 */
uint32_t x25519_getkey(uint8_t prikey[32], uint8_t pubkey[32])
{
    uint32_t ret;

    if((NULL == prikey) || (NULL == pubkey))
    {
        ret = X25519_POINTER_NULL;
    }
    else
    {
        ret = get_rand(prikey, C25519_BYTE_LEN);
        if(TRNG_SUCCESS == ret)
        {
            ret = x25519_get_pubkey_from_prikey(prikey, pubkey);
        }
        else
        {}
    }

    return ret;
}


/* Function:  X25519 key agreement
 * Parameters:
 *     local_prikey --------------- input, local private key, 32 bytes, little-endian
 *     peer_pubkey ---------------- input, peer Public key, 32 bytes, little-endian
 *     key ------------------------ output, derived key
 *     keyByteLen ----------------- input, byte length of output key
 *     kdf ------------------------ input, KDF function
 * Return: X25519_SUCCESS(success); other(error)
 * Caution:
 *     1. if no KDF function, please set kdf to be NULL
 */
uint32_t x25519_compute_key(const uint8_t local_prikey[32], const uint8_t peer_pubkey[32],
        uint8_t *key, uint32_t keyByteLen, KDF_FUNC kdf)
{
    uint32_t k[C25519_WORD_LEN], u[C25519_WORD_LEN];
    uint32_t ret;

    if((NULL == local_prikey) || (NULL == peer_pubkey) || (NULL == key))
    {
        ret = X25519_POINTER_NULL;
    }
    else if((0u == keyByteLen) || ((NULL == kdf) && (keyByteLen > C25519_BYTE_LEN)))
    {
        ret = X25519_INVALID_INPUT;
    }
    else
    {
        ret = x25519_decode_u(peer_pubkey, c25519->p, u);    //decode u
        if(PKE_SUCCESS == ret)
        {
            //u could not be zero, otherwise it will return PKE_NO_MODINV no matter what the scalar is.
            if(1u == uint32_BigNum_Check_Zero(u, C25519_WORD_LEN))
            {
                ret = X25519_INVALID_INPUT;
            }
            else
            {}
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            x25519_ed25519_decode_scalar(local_prikey, (uint8_t *)k);     //decode scalar
            ret = x25519_pointMul(c25519, k, u, u);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            //make sure u is not zero
            if(0u != uint32_BigNum_Check_Zero(u, C25519_WORD_LEN))
            {
                ret = X25519_ZERO_ALL;
            }
            else
            {}
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            if(NULL != kdf)
            {
                (void)kdf(u, C25519_BYTE_LEN, key, keyByteLen);
            }
            else
            {
                memcpy_(key, (uint8_t *)u, keyByteLen);
            }
            ret = X25519_SUCCESS;
        }
        else
        {}
    }

    return ret;
}

#endif

