
#include "../../crypto_include/pke_config.h"


#ifdef SUPPORT_RSASSA_PKCS1_V1_5

#include "../../crypto_include/pke/rsa.h"
#include "../../crypto_include/pke/rsa_u8.h"
#include "../../crypto_include/hash_hmac/hash.h"
#include "../../crypto_include/crypto_common/utility.h"
#include "../../crypto_include/trng/trng.h"

#if 0
const uint8_t MD2_prefix[]       = {0x30,0x20,0x30,0x0C,0x06,0x08,0x2A,0x86,0x48,0x86,0xF7,0x0D,0x02,0x02,0x05,0x00,0x04,0x10,};
#endif

#ifdef SUPPORT_HASH_MD5
extern const uint8_t MD5_prefix[18];
const uint8_t MD5_prefix[18]         = {0x30,0x20,0x30,0x0C,0x06,0x08,0x2A,0x86,0x48,0x86,0xF7,0x0D,0x02,0x05,0x05,0x00,0x04,0x10};
#endif

#ifdef SUPPORT_HASH_SHA1
extern const uint8_t SHA_1_prefix[15];
const uint8_t SHA_1_prefix[15]       = {0x30,0x21,0x30,0x09,0x06,0x05,0x2B,0x0E,0x03,0x02,0x1A,0x05,0x00,0x04,0x14};
#endif

#ifdef SUPPORT_HASH_SHA224
extern const uint8_t SHA_224_prefix[19];
const uint8_t SHA_224_prefix[19]     = {0x30,0x2D,0x30,0x0D,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x04,0x05,0x00,0x04,0x1C};
#endif

#ifdef SUPPORT_HASH_SHA256
extern const uint8_t SHA_256_prefix[19];
const uint8_t SHA_256_prefix[19]     = {0x30,0x31,0x30,0x0D,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x01,0x05,0x00,0x04,0x20};
#endif

#ifdef SUPPORT_HASH_SHA384
extern const uint8_t SHA_384_prefix[19];
const uint8_t SHA_384_prefix[19]     = {0x30,0x41,0x30,0x0D,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x02,0x05,0x00,0x04,0x30};
#endif

#ifdef SUPPORT_HASH_SHA512
extern const uint8_t SHA_512_prefix[19];
const uint8_t SHA_512_prefix[19]     = {0x30,0x51,0x30,0x0D,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x03,0x05,0x00,0x04,0x40};
#endif

#ifdef SUPPORT_HASH_SHA512_224
extern const uint8_t SHA_512_224_prefix[19];
const uint8_t SHA_512_224_prefix[19] = {0x30,0x2D,0x30,0x0D,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x05,0x05,0x00,0x04,0x1C};
#endif

#ifdef SUPPORT_HASH_SHA512_256
extern const uint8_t SHA_512_256_prefix[19];
const uint8_t SHA_512_256_prefix[19] = {0x30,0x31,0x30,0x0D,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x06,0x05,0x00,0x04,0x20};
#endif


/* function: check rsassa pkcs1-v1.5 parameter
 * parameters:
 *     modulus -------------------- input, modulus
 *     exponent ------------------- input, exponent
 *     n_bits --------------------- input, bit length of n
 *     exp_bits ------------------- input, bit length of exponent
 *     msg ------------------------ input, message or digest of msg to be encode or decode
 *     signature ------------------ input, signature to sign or verify
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. modulus must be odd
 *     2. please make sure exp_bits <= n_bits <= OPERAND_MAX_BIT_LEN
 */
FLAG_STATIC uint32_t rsa_ssa_pkcs1_v1_5_param_check(const uint8_t *modulus, uint32_t n_bits, const void *exponent, uint32_t exp_bits,
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


/* function: get EMSA-PKCS1-v1_5 the DER encoding T of the digest info value
 * parameters:
 *     alg ------------------------ input, specific hash algorithm for message or Hash(message)
 *     prefix_bytes --------------- output, byte length of digestInfo value
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure hash alg is valid
 */
FLAG_STATIC const uint8_t *get_pkcs1_v1_5_t_prefix(hash_alg_e alg, uint32_t *prefix_bytes)
{
    const uint8_t *prefix;

	switch(alg)
	{
#ifdef SUPPORT_HASH_MD5
	case HASH_MD5:
		prefix = MD5_prefix;
		*prefix_bytes = sizeof(MD5_prefix);
		break;
#endif

#ifdef SUPPORT_HASH_SHA1
	case HASH_SHA1:
		prefix = SHA_1_prefix;
		*prefix_bytes = sizeof(SHA_1_prefix);
		break;
#endif

#ifdef SUPPORT_HASH_SHA224
	case HASH_SHA224:
		prefix = SHA_224_prefix;
		*prefix_bytes = sizeof(SHA_224_prefix);
		break;
#endif

#ifdef SUPPORT_HASH_SHA256
	case HASH_SHA256:
		prefix = SHA_256_prefix;
		*prefix_bytes = sizeof(SHA_256_prefix);
		break;
#endif

#ifdef SUPPORT_HASH_SHA384
	case HASH_SHA384:
		prefix = SHA_384_prefix;
		*prefix_bytes = sizeof(SHA_384_prefix);
		break;
#endif

#ifdef SUPPORT_HASH_SHA512
	case HASH_SHA512:
		prefix = SHA_512_prefix;
		*prefix_bytes = sizeof(SHA_512_prefix);
		break;
#endif

#ifdef SUPPORT_HASH_SHA512_224
	case HASH_SHA512_224:
		prefix = SHA_512_224_prefix;
		*prefix_bytes = sizeof(SHA_512_224_prefix);
		break;
#endif

#ifdef SUPPORT_HASH_SHA512_256
	case HASH_SHA512_256:
		prefix = SHA_512_256_prefix;
		*prefix_bytes = sizeof(SHA_512_256_prefix);
		break;
#endif

	default:
		prefix = NULL;
		*prefix_bytes = 0;
                break;
	}

    return prefix;
}


/* function: RSA PKCS#1_v2.2 EMSA-PKCS1-v1_5 encoding with message digest
 * parameters:
 *     alg ------------------------ input, specific hash algorithm for message or Hash(message)
 *     msg_digest ----------------- input, Hash(message), message is to be signed, here Hash is msg_hash_alg.
 *     em ------------------------- output, big integer to be signed, big-endian.
 *     em_bytes ------------------- input, byte length of em, should be bit length of RSA modulus n minus 1.
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure hash alg is valid
 */
uint32_t emsa_pkcs_v1_5_encode_by_msg_digest(hash_alg_e alg, const uint8_t *msg_digest,
    uint8_t *em, uint32_t em_bytes)
{
	const uint8_t *prefix;
	uint32_t prefix_bytes;
	uint32_t ps_bytes;
    uint32_t ret = RSA_SUCCESS;
	uint32_t digest_bytes = (uint32_t)hash_get_digest_word_len(alg)<<2;

	if((0U == digest_bytes) || (NULL == msg_digest) || (NULL == em))
	{
		ret = RSA_INPUT_INVALID;
	}
    else
    {
        prefix = get_pkcs1_v1_5_t_prefix(alg, &prefix_bytes);
        if(em_bytes < ((prefix_bytes+digest_bytes)+11U))
        {
            ret = RSA_INPUT_INVALID;
        }
        else
        {}
    }

    if(RSA_SUCCESS == ret)
    {
        ps_bytes = em_bytes -3U-prefix_bytes-digest_bytes;

        //EM = 0x00 || 0x01 || PS || 0x00 || hashAlgID || hash(M)
        em[0] = 0x00;
        em[1] = 0x01;
        memset_(&em[2U], 0xFF, ps_bytes);
        em[2U+ps_bytes] = 0x00;
        memcpy_(&em[3U+ps_bytes], prefix, prefix_bytes);
        memcpy_(&em[em_bytes-digest_bytes], msg_digest, digest_bytes);
        if(HASH_SUCCESS == ret)
        {
            ret = RSA_SUCCESS;
        }
        else
        {}
    }
    else
    {}

	return ret;
}


/* function: RSA PKCS#1_v2.2 EMSA-PKCS1-v1_5 encoding with message
 * parameters:
 *     alg ------------------------ input, specific hash algorithm for message or Hash(message)
 *     msg ------------------------ input, message to be encoding
 *     msg_bytes ------------------ input, byte length of message
 *     em ------------------------- output, big integer to be signed, big-endian.
 *     em_bytes ------------------- input, byte length of em, should be bit length of RSA modulus n minus 1.
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure hash alg is valid
 */
uint32_t emsa_pkcs_v1_5_encode(hash_alg_e alg, const uint8_t *msg, uint32_t msg_bytes, uint8_t *em, uint32_t em_bytes)
{
	const uint8_t *prefix;
	uint32_t prefix_bytes;
	uint32_t ps_bytes;
    uint32_t ret = RSA_SUCCESS;
	uint32_t digest_bytes = (uint32_t)hash_get_digest_word_len(alg)<<2;

	if(0U == digest_bytes)
	{
		ret = RSA_INPUT_INVALID;
	}
    else
    {
        prefix = get_pkcs1_v1_5_t_prefix(alg, &prefix_bytes);
        if(em_bytes < ((prefix_bytes+digest_bytes)+11U))
        {
            ret = RSA_INPUT_INVALID;
        }
        else
        {}
    }

    if(RSA_SUCCESS == ret)
    {
        ps_bytes = em_bytes -3U-prefix_bytes-digest_bytes;
        
        em[0] = 0x00;
        em[1] = 0x01;
        memset_(&em[2U], 0xFF, ps_bytes);
        em[2U+ps_bytes] = 0x00;
        memcpy_(&em[3U+ps_bytes], prefix, prefix_bytes);
        ret = hash(alg, msg, msg_bytes, &em[em_bytes-digest_bytes]);
        if(HASH_SUCCESS == ret)
        {
            ret = RSA_SUCCESS;
        }
        else
        {}
    }
    else
    {}

	return ret;
}


/* function: RSA PKCS#1_v2.2 RSASSA-PKCS1-v1_5 with message digest
 * parameters:
 *     alg ------------------------ input, specific hash algorithm for message or Hash(message)
 *     msg_digest ----------------- input, Hash(message), message is to be verified, here Hash is msg_hash_alg.
 *     d -------------------------- input, RSA private key d, (n_bits+7)/8 bytes, big-endian.
 *     n -------------------------- input, RSA modulus n, (n_bits+7)/8 bytes, big-endian.
 *     n_bits --------------------- input, bit length of n
 *     signature ------------------ output, RSA signature, (n_bits+7)/8 bytes, big-endian.
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 */
uint32_t rsa_ssa_pkcs1_v1_5_sign_by_msg_digest(hash_alg_e alg, const uint8_t *msg_digest,
        const uint8_t *d, const uint8_t *n, uint32_t n_bits, uint8_t *signature)
{
    uint8_t em[RSA_MAX_BYTE_LEN];
    uint32_t ret;

    ret = rsa_ssa_pkcs1_v1_5_param_check(n, n_bits, d, n_bits, msg_digest, signature);
    if(RSA_SUCCESS == ret)
    {
        ret = emsa_pkcs_v1_5_encode_by_msg_digest(alg, msg_digest, (uint8_t *)em, GET_BYTE_LEN(n_bits));
    }
    else
    {}

    if(RSA_SUCCESS == ret)
    {
        ret = pke_modexp_U8(n, d, (uint8_t *)em, signature, n_bits, n_bits, 1);
    }
    else
    {}

    return ret;
}


/* function: RSA PKCS#1_v2.2 RSASSA-PKCS1-v1_5 with message digest
 * parameters:
 *     alg ------------------------ input, specific hash algorithm for message or Hash(message)
 *     msg_digest ----------------- input, Hash(message), message is to be verified, here Hash is msg_hash_alg.
 *     d -------------------------- input, RSA private key d, (n_bits+7)/8 bytes, big-endian.
 *     n -------------------------- input, RSA modulus n, (n_bits+7)/8 bytes, big-endian.
 *     n_bits --------------------- input, bit length of n
 *     signature ------------------ output, RSA signature, (n_bits+7)/8 bytes, big-endian.
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 */
uint32_t rsa_ssa_pkcs1_v1_5_crt_sign_by_msg_digest(hash_alg_e alg, const uint8_t *msg_digest,
        const rsa_crt_private_key_st *d, const uint8_t *n, uint32_t n_bits, uint8_t *signature)
{
    uint8_t em[RSA_MAX_BYTE_LEN];
    uint32_t ret;

    ret = rsa_ssa_pkcs1_v1_5_param_check(n, n_bits, d, n_bits, msg_digest, signature);
    if(RSA_SUCCESS == ret)
    {
        ret = emsa_pkcs_v1_5_encode_by_msg_digest(alg, msg_digest, (uint8_t *)em, GET_BYTE_LEN(n_bits));
    }
    else
    {}

    if(RSA_SUCCESS == ret)
    {
        ret = RSA_CRTModExp_U8(em, d->p, d->q, d->dp, d->dq, d->u, signature, n_bits);
    }
    else
    {}

    return ret;
}


/* function: RSA PKCS#1_v2.2 RSASSA-PKCS1-v1_5 with message
 * parameters:
 *     alg ------------------------ input, specific hash algorithm for message or Hash(message)
 *     msg ------------------------ input, message to be signed
 *     msg_bytes ------------------ input, byte length of message
 *     d -------------------------- input, RSA private key d, (n_bits+7)/8 bytes, big-endian.
 *     n -------------------------- input, RSA modulus n, (n_bits+7)/8 bytes, big-endian.
 *     n_bits --------------------- input, bit length of n
 *     signature ------------------ output, RSA signature, (n_bits+7)/8 bytes, big-endian.
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. msg_bytes should in [0, em_bytes-t_bytes-11], em_bytes is (em_bits+7)/8, 
 *        t_bytes=hash_bytes+hashAlgID_bytes, hashAlgID_bytes refer to get_pkcs1_v1_5_t_prefix().
 */
uint32_t rsa_ssa_pkcs1_v1_5_sign(hash_alg_e alg, const uint8_t *msg, uint32_t msg_bytes, 
        const uint8_t *d, const uint8_t *n, uint32_t n_bits, uint8_t *signature)
{
    uint8_t em[RSA_MAX_BYTE_LEN];
    uint32_t ret;

    ret = rsa_ssa_pkcs1_v1_5_param_check(n, n_bits, d, n_bits, msg, signature);
    if(RSA_SUCCESS == ret)
    {
        ret = emsa_pkcs_v1_5_encode(alg, msg, msg_bytes, em, GET_BYTE_LEN(n_bits));
    }
    else
    {}

    if(RSA_SUCCESS == ret)
    {
        ret = pke_modexp_U8(n, d, em, signature, n_bits, n_bits, 1);
    }
    else
    {}

    return ret;
}


/* function: RSA PKCS#1_v2.2 RSASSA-PKCS1-v1_5 with message(private key is CRT style)
 * parameters:
 *     alg ------------------------ input, specific hash algorithm for message or Hash(message)
 *     msg ------------------------ input, message to be signed
 *     msg_bytes ------------------ input, byte length of message
 *     d -------------------------- input, RSA-CRT private key (p,q,dp,dq,u), every field is (n_bits/2+7)/8 bytes, big-endian.
 *     n -------------------------- input, RSA modulus n, (n_bits+7)/8 bytes, big-endian.
 *     n_bits --------------------- input, bit length of n
 *     signature ------------------ output, RSA signature, (n_bits+7)/8 bytes, big-endian.
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. msg_bytes should in [0, em_bytes-t_bytes-11], em_bytes is (em_bits+7)/8, 
 *        t_bytes=hash_bytes+hashAlgID_bytes, hashAlgID_bytes refer to get_pkcs1_v1_5_t_prefix().
 */
uint32_t rsa_ssa_pkcs1_v1_5_crt_sign(hash_alg_e alg, const uint8_t *msg, uint32_t msg_bytes, 
        const rsa_crt_private_key_st *d, const uint8_t *n, uint32_t n_bits, uint8_t *signature)
{
    uint8_t em[RSA_MAX_BYTE_LEN];
    uint32_t ret;

    ret = rsa_ssa_pkcs1_v1_5_param_check(n, n_bits, d, n_bits, msg, signature);
    if(RSA_SUCCESS == ret)
    {
        ret = emsa_pkcs_v1_5_encode(alg, msg, msg_bytes, (uint8_t *)em, GET_BYTE_LEN(n_bits));
    }
    else
    {}

    if(RSA_SUCCESS == ret)
    {
        ret = RSA_CRTModExp_U8(em, d->p, d->q, d->dp, d->dq, d->u, signature, n_bits);
    }
    else
    {}

    return ret;
}


/* function: RSA PKCS#1_v2.2 RSASSA-PKCS1-v1_5 with message
 * parameters:
 *     alg ------------------------ input, specific hash algorithm for message or Hash(message)
 *     msg ------------------------ input, message to be verify
 *     msg_bytes ------------------ input, byte length of message
 *     e -------------------------- input, RSA public key e, (e_bits+7)/8 bytes, big-endian.
 *     e_bits --------------------- input, bit length of e
 *     n -------------------------- input, RSA modulus n, (n_bits+7)/8 bytes, big-endian.
 *     n_bits --------------------- input, bit length of n
 *     signature ------------------ input, RSA signature, (n_bits+7)/8 bytes, big-endian.
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. msg_bytes should in [0, em_bytes-t_bytes-11], em_bytes is (em_bits+7)/8, 
 *        t_bytes=hash_bytes+hashAlgID_bytes, hashAlgID_bytes refer to get_pkcs1_v1_5_t_prefix().
 */
uint32_t rsa_ssa_pkcs1_v1_5_verify(hash_alg_e alg, const uint8_t *msg, uint32_t msg_bytes, 
        const uint8_t *e, uint32_t e_bits, const uint8_t *n, uint32_t n_bits, const uint8_t *signature)
{
    uint8_t em1[RSA_MAX_BYTE_LEN];
    uint8_t em2[RSA_MAX_BYTE_LEN];
    uint32_t ret;

    ret = rsa_ssa_pkcs1_v1_5_param_check(n, n_bits, e, e_bits, msg, signature);
    if(RSA_SUCCESS == ret)
    {
        ret = emsa_pkcs_v1_5_encode(alg, msg, msg_bytes, (uint8_t *)em1, GET_BYTE_LEN(n_bits));
    }
    else
    {}

    if(RSA_SUCCESS == ret)
    {
        ret = pke_modexp_U8(n, e, signature, (uint8_t *)em2, n_bits, e_bits, 1);
        if(0U !=memcmp_(em1, em2, GET_BYTE_LEN(n_bits)))
        {
            ret = RSA_ERROR;
        }
        else
        {
            ret = RSA_SUCCESS;
        }
    }
    else
    {}

    return ret;
}


/* function: RSA PKCS#1_v2.2 RSASSA-PKCS1-v1_5 with message digest
 * parameters:
 *     alg ------------------------ input, specific hash algorithm for message or Hash(message)
 *     msg_digest ----------------- input, Hash(message), message is to be signed, here Hash is msg_hash_alg.
 *     e -------------------------- input, RSA public key e, (e_bits+7)/8 bytes, big-endian.
 *     e_bits --------------------- input, bit length of e
 *     n -------------------------- input, RSA modulus n, (n_bits+7)/8 bytes, big-endian.
 *     n_bits --------------------- input, bit length of n
 *     signature ------------------ input, RSA signature, (n_bits+7)/8 bytes, big-endian.
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. msg_bytes should in [0, em_bytes-t_bytes-11], em_bytes is (em_bits+7)/8, 
 *        t_bytes=hash_bytes+hashAlgID_bytes, hashAlgID_bytes refer to get_pkcs1_v1_5_t_prefix().
 */
uint32_t rsa_ssa_pkcs1_v1_5_verify_by_msg_digest(hash_alg_e alg, const uint8_t *msg_digest, const uint8_t *e, 
    uint32_t e_bits, const uint8_t *n, uint32_t n_bits, const uint8_t *signature)
{
    uint8_t em1[RSA_MAX_BYTE_LEN];
    uint8_t em2[RSA_MAX_BYTE_LEN];
    uint32_t ret;

    ret = rsa_ssa_pkcs1_v1_5_param_check(n, n_bits, e, e_bits, msg_digest, signature);
    if(RSA_SUCCESS == ret)
    {
        ret = emsa_pkcs_v1_5_encode_by_msg_digest(alg, msg_digest, (uint8_t *)em1, GET_BYTE_LEN(n_bits));
    }
    else
    {}

    if(RSA_SUCCESS == ret)
    {
        ret = pke_modexp_U8(n, e, signature, (uint8_t *)em2, n_bits, e_bits, 1);
        if(0U !=memcmp_(em1, em2, GET_BYTE_LEN(n_bits)))
        {
            ret = RSA_ERROR;
        }
        else
        {
            ret = RSA_SUCCESS;
        }
    }
    else
    {}

    return ret;
}
#endif

