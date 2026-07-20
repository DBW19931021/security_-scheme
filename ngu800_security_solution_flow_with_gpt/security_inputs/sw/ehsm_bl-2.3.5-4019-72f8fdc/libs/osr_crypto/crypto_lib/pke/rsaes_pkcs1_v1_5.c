
#include "../../crypto_include/pke_config.h"


#ifdef SUPPORT_RSAES_PKCS1_V1_5

#include "../../crypto_include/pke/rsa.h"
#include "../../crypto_include/pke/rsa_u8.h"
#include "../../crypto_include/crypto_common/utility.h"
#include "../../crypto_include/trng/trng.h"


/* function: check rsaes pkcs1-v1.5 parameter
 * parameters:
 *     modulus -------------------- input, modulus
 *     exponent ------------------- input, exponent
 *     n_bits --------------------- input, bit length of n
 *     exp_bits ------------------- input, bit length of exponent
 *     msg ------------------------ input, message to be encode or decode
 *     cipher --------------------- input, cipher to encrypt or decrypt
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. modulus must be odd
 *     2. please make sure exp_bits <= n_bits <= OPERAND_MAX_BIT_LEN
 */
FLAG_STATIC uint32_t rsa_es_pkcs1_v1_5_param_check(const uint8_t *modulus, uint32_t n_bits, const void *exponent, uint32_t exp_bits,
    const uint8_t *msg, const uint8_t *cipher)
{
    uint32_t tmp, ret = RSA_SUCCESS;

    tmp = GET_BYTE_LEN(n_bits);

    if((NULL == exponent) ||(NULL == modulus) || (NULL == msg) || (NULL == cipher))
    {
        ret = RSA_INPUT_INVALID;
    }
    else if((n_bits > RSA_MAX_BIT_LEN) || (exp_bits > n_bits) || (n_bits < RSA_MIN_BIT_LEN))
    {
        ret = RSA_INPUT_INVALID;
    }
    else if(((uint8_t)0) == (modulus[tmp-1U] & 1U)) //n can not be even
    {
        ret = RSA_INPUT_INVALID;
    }
    else
    {
        //handld other
    }

    return ret;
}


/* function: get nonzero octets
 * parameters:
 *     ps ------------------------- output, padding octets
 *     ps_bytes ------------------- input, bytes length of ps
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 */
FLAG_STATIC uint32_t eme_pkcs1_v1_5_encode_gen_ps(uint8_t *ps, uint32_t ps_bytes)
{
    uint32_t i, ret;

    ret = get_rand(ps, ps_bytes);
    if(TRNG_SUCCESS == ret)
    {
        ret = RSA_SUCCESS;

        //make sure PS is nonzero octets
        for(i=0U; i<ps_bytes; i++)
        {
            while(0U == ps[i])
            {
                ret = get_rand(&ps[i], 1U);
                if(TRNG_SUCCESS != ret)
                {
                    ret = RSA_ERROR;
                    break;
                }
                else
                {}
            }

            if(RSA_ERROR == ret)
            {
                break;
            }
            else
            {}
        }
    }
    else
    {}

    return ret;
}


/* function: EME-PKCS1-v1_5 encoding
 * parameters:
 *     msg ------------------------ input, message to be enc
 *     msg_bytes ------------------ input, byte length of message
 *     ps ------------------------- input, padding octets
 *     em ------------------------- output, big integer to be encrypt, big-endian.
 *     em_bytes ------------------- input, byte length of em, should be bit length of RSA modulus n
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. if you do not have ps, please set the parameter to be NULL, it will be generated inside.
 *        otherwise please make sure ps is nonzero octets, and it occupy em_bytes-3-msg_bytes.
 *     2. em_bytes is (em_bits+7)/8
 */
uint32_t eme_pkcs1_v1_5_encode(const uint8_t *msg, uint32_t msg_bytes, const uint8_t *ps, uint8_t *em, uint32_t em_bytes)
{
    uint32_t ret = RSA_SUCCESS;
	uint32_t i, ps_bytes;

    if(((NULL == msg) && (0U != msg_bytes)) || (NULL == em))
    {
        ret = RSA_INPUT_INVALID;
    }
    else if((msg_bytes+0x0BU) > em_bytes) //ps occupy at lease 8 bytes
    {
        ret = RSA_INPUT_INVALID;
    }
    else
    {
        //handle other
    }

    //0x00 || 0x02 || PS
    if(RSA_SUCCESS == ret)
    {
        ps_bytes = em_bytes-3U-msg_bytes;

        em[0] = 0x00U;
        em[1] = 0x02;

        if(NULL != ps)
        {
            //ps from outside
            for(i=0; i<ps_bytes; i++)    
            {
                if(ps[i] == (uint8_t)0x00) //PS is nonzero octets
                {
                    ret = RSA_INPUT_INVALID;
                    break;
                }
                else
                {}
            }

            if(RSA_SUCCESS == ret)
            {
                memcpy_(&em[2], ps, ps_bytes);
            }
            else
            {}
        }
        else
        {
            //ps generate inside
            ret = eme_pkcs1_v1_5_encode_gen_ps(&em[2], ps_bytes);
            if(TRNG_SUCCESS == ret)
            {
                ret = RSA_SUCCESS;
            }
            else
            {}
        }
    }
    else
    {}
    
    if(RSA_SUCCESS == ret)
    {
        //em = 0x00 || 0x02 || PS || 0x00 || msg
        em[2U+ps_bytes] = 0x00U;
        memcpy_(&em[3U+ps_bytes], msg, msg_bytes);
    }
    else
    {}

    return ret;
}


/* function: EME-PKCS1-v1_5 decoding
 * parameters:
 *     msg ------------------------ output, message to be enc
 *     msg_bytes ------------------ output, byte length of message
 *     em ------------------------- input, big integer to be parse, big-endian.
 *     em_bytes ------------------- input, byte length of em, should be bit length of RSA modulus n
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. em_bytes is (em_bits+7)/8
 */
uint32_t eme_pkcs1_v1_5_decode(uint8_t *msg, uint32_t *msg_bytes, const uint8_t *em, uint32_t em_bytes)
{
    uint32_t ret = RSA_SUCCESS, i;
    
    if((NULL == msg) || (NULL == msg_bytes) || (NULL == em))
    {
        ret = RSA_INPUT_INVALID;
    }
    else
    {
        //check eme first and second byte
        if((0x00U != em[0]) || (0x02U != em[1]))
        {
            ret = RSA_INPUT_INVALID;
        }
        else
        {}
    }

    if(RSA_SUCCESS == ret)
    {
        for(i=2; i<em_bytes; i++)    
        {
            if(em[i] == 0x00U)
            {
                break;
            }
            else
            {}
        }
    }
    else
    {}

    if(RSA_SUCCESS == ret)
    {
        //check PS bytes >= 8
        if((em_bytes == i) || (i < 10U))
        {
            ret = RSA_INPUT_INVALID;
        }
        else
        {
            *msg_bytes = em_bytes - (i+1U);
            memcpy_(msg, &em[i+1U], *msg_bytes);
        }
    }
    else
    {}

    return ret;
}


/* function: RSA PKCS#1_v2.2 RSAES_PKCS1_V1_5 ENCRYPTO with message
 * parameters:
 *     msg ------------------------ input, message to be enc
 *     msg_bytes ------------------ input, byte length of message
 *     ps ------------------------- input, padding octets
 *     e -------------------------- input, RSA public key e, (e_bits+7)/8 bytes, big-endian.
 *     e_bits --------------------- input, bit length of e
 *     n -------------------------- input, RSA modulus n, (n_bits+7)/8 bytes, big-endian.
 *     n_bits --------------------- input, bit length of n
 *     cipher ----------------- output, RSA cipher, (n_bits+7)/8 bytes, big-endian.
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. if you do not have ps, please set the parameter to be NULL, it will be generated inside.
 *        otherwise please make sure ps is nonzero octets, and it occupy em_bytes-3-msg_bytes.
 *     2. em_bytes is (em_bits+7)/8
 */
uint32_t rsa_es_pkcs1_v1_5_enc(const uint8_t *msg, uint32_t msg_bytes, const uint8_t *ps, const uint8_t *e, 
    uint32_t e_bits, const uint8_t *n, uint32_t n_bits, uint8_t *cipher)
{
    uint8_t em[RSA_MAX_BYTE_LEN];
    uint32_t ret;

    ret = rsa_es_pkcs1_v1_5_param_check(n, n_bits, e, e_bits, msg, cipher);
    if(RSA_SUCCESS == ret)
    {
        ret = eme_pkcs1_v1_5_encode(msg, msg_bytes, ps, em, GET_BYTE_LEN(n_bits));
    }
    else
    {}

    if(RSA_SUCCESS == ret)
    {
        ret = pke_modexp_U8(n, e, em, cipher, n_bits, e_bits, 1);
    }
    else
    {}

    return ret;
}


/* function: RSA PKCS#1_v2.2 RSAES_PKCS1_V1_5 DECRYPTO with message
 * parameters:
 *     msg ------------------------ output, message to be enc
 *     msg_bytes ------------------ output, byte length of message
 *     d -------------------------- input, RSA private key d, (n_bits+7)/8 bytes, big-endian.
 *     n -------------------------- input, RSA modulus n, (n_bits+7)/8 bytes, big-endian.
 *     n_bits --------------------- input, bit length of n
 *     cipher --------------------- input, RSA cipher, (n_bits+7)/8 bytes, big-endian.
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 */
uint32_t rsa_es_pkcs1_v1_5_dec(uint8_t *msg, uint32_t *msg_bytes, const uint8_t *d, 
        const uint8_t *n, uint32_t n_bits, const uint8_t *cipher)
{
    uint8_t em[RSA_MAX_BYTE_LEN];
    uint32_t ret;

    ret = rsa_es_pkcs1_v1_5_param_check(n, n_bits, d, n_bits, msg, cipher);
    if(RSA_SUCCESS == ret)
    {
        ret = pke_modexp_U8(n, d, cipher, em, n_bits, n_bits, 1U);
    }
    else
    {}

    if(RSA_SUCCESS == ret)
    {
        ret = eme_pkcs1_v1_5_decode(msg, msg_bytes, em, GET_BYTE_LEN(n_bits));
    }
    else
    {}

    return ret;
}


/* function: RSAES_PKCS1_V1_5 DECRYPTO with message(private key is CRT style)
 * parameters:
 *     msg ------------------------ output, message to be enc
 *     msg_bytes ------------------ output, byte length of message
 *     d -------------------------- input, RSA-CRT private key (p,q,dp,dq,u), every field is (n_bits/2+7)/8 bytes, big-endian.
 *     n -------------------------- input, RSA modulus n, (n_bits+7)/8 bytes, big-endian.
 *     n_bits --------------------- input, bit length of n
 *     cipher --------------------- input, RSA cipher, (n_bits+7)/8 bytes, big-endian.
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 */
uint32_t rsa_es_pkcs1_v1_5_crt_dec(uint8_t *msg, uint32_t *msg_bytes, const rsa_crt_private_key_st *d, 
        uint32_t n_bits, const uint8_t *cipher) 
{
    uint8_t em[RSA_MAX_BYTE_LEN];
    uint32_t tmp, ret = RSA_SUCCESS;

    //clear last word
    if(0U != (n_bits & 0x1FU))
    {
        tmp = GET_WORD_LEN(n_bits) - 1U;
        em[tmp] = 0U;
    }
    else
    {}

    if(RSA_SUCCESS == ret)
    {
        ret = RSA_CRTModExp_U8(cipher, d->p, d->q, d->dp, d->dq, d->u, em, n_bits);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = eme_pkcs1_v1_5_decode(msg, msg_bytes, (uint8_t *)em, GET_BYTE_LEN(n_bits));
    }
    else
    {}

    memset_(em, 0, sizeof(em));

    return ret;
}

#endif

