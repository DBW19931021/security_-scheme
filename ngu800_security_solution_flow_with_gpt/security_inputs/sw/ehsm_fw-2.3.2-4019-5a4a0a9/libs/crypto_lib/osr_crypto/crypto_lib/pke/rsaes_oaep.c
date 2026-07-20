
#include "../../crypto_include/pke_config.h"


#ifdef SUPPORT_RSAES_OAEP

#include "../../crypto_include/pke/rsa_u8.h"
#include "../../crypto_include/hash_hmac/hash.h"
#include "../../crypto_include/crypto_common/utility.h"
#include "../../crypto_include/trng/trng.h"


/* function: check rsaes oaep parameter
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
FLAG_STATIC uint32_t rsa_es_oaep_param_check(const uint8_t *modulus, uint32_t n_bits, const void *exponent, uint32_t exp_bits,
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


/* function: RSA PKCS#1_v2.2 RSAES-OAEP encode with label digest
 * parameters:
 *     label_hash_alg ------------- input, specific hash algorithm for message or Hash(label)
 *     mgf_hash_alg --------------- input, specific hash algorithm for MGF1
 *     label_digest --------------- input, label digest of label_hash_alg
 *     seed ----------------------- input, Seed, occupy digest bytes of label_hash_alg 
 *     msg ------------------------ input, message to be encode
 *     msg_digest ----------------- input, message digest of label_hash_alg
 *     msg_bytes ------------------ input, byte length of message
 *     em ------------------------- output, big integer to be encrypt, big-endian.
 *     em_bytes ------------------- input, byte length of em, should be bit length of RSA modulus n
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. it is recommended that label_hash_alg and mgf_hash_alg are the same.
 *     2. if no seed prepared to input, please set seed to NULL, it will be generated inside.
 *     3. msg_bytes should be in [0, em_bytes-2*hLen-2], em_bytes is (n_bits+7)/8, hLen is digest length of hash algorithm msg_hash_alg
 *        it is recommended to use default value, digest length of hash algorithm msg_hash_alg or mgf_hash_alg.
 */
uint32_t eme_oaep_encode_by_label_digest(hash_alg_e label_hash_alg, hash_alg_e mgf_hash_alg, const uint8_t *label_digest, 
        const uint8_t *seed, const uint8_t *msg, uint32_t msg_bytes, uint8_t *em, uint32_t em_bytes)
{
    uint32_t ret = RSA_SUCCESS;
    uint32_t label_digest_bytes = (uint32_t)hash_get_digest_word_len(label_hash_alg)<<2;  //also byte length of seed
    uint8_t *seed_p = &em[1];
    uint8_t *db;

    if((NULL == label_digest) || (NULL == em) || (0U == label_digest_bytes))
    {
        ret = RSA_INPUT_INVALID;
    }
    else if(em_bytes < ((label_digest_bytes*2U) + 2U + msg_bytes))
    {
        ret = RSA_INPUT_INVALID;
    }
    else
    {
        //handle other
    }

    if(RSA_SUCCESS == ret)
    {
        //set first bytes of EM
        em[0] = 0x00;
        
        //set seed
        if(NULL == seed)
        {
            ret = get_rand(seed_p, label_digest_bytes);
            if(TRNG_SUCCESS == ret)
            {
                ret = RSA_SUCCESS;
            }
            else
            {}
        }
        else
        {
            memcpy_(seed_p, seed, label_digest_bytes);
        }
    }
    else
    {}
    
    if(RSA_SUCCESS == ret)
    {
        //get lhash is hash(L)
        db = &em[1U+label_digest_bytes];
        memcpy_(db, label_digest, label_digest_bytes);

        //set (PS||0x01), DB = lHash||PS||0x01||M, PS is all zero octets
        memset_(&db[label_digest_bytes], 0U, em_bytes-msg_bytes-(2U*label_digest_bytes)-2U);
        em[em_bytes - msg_bytes - 1u] = 0x01;
        memcpy_(&em[em_bytes - msg_bytes], msg, msg_bytes);

        //get maskedDB, EM = 0x00||maskedSeed||maskedDB
        ret = rsa_pkcs1_mgf1_with_xor_in(mgf_hash_alg, seed_p, label_digest_bytes, db, db, 
            em_bytes - 1U - label_digest_bytes);
    }
    else
    {}

    if(ret == RSA_SUCCESS)
    {
        //get maskedSeed, EM = 0x00||maskedSeed||maskedDB
        ret = rsa_pkcs1_mgf1_with_xor_in(mgf_hash_alg, db, em_bytes - 1U - label_digest_bytes, 
            seed_p, seed_p, label_digest_bytes);
    }
    else
    {}

    return ret;
}


/* function: RSA PKCS#1_v2.2 RSAES-OAEP encode
 * parameters:
 *     label_hash_alg ------------- input, specific hash algorithm for message or Hash(label)
 *     mgf_hash_alg --------------- input, specific hash algorithm for MGF1
 *     label ---------------------- input, label
 *     label_bytes ---------------- input, byte length of label
 *     seed ----------------------- input, Seed, occupy digest bytes of label_hash_alg 
 *     msg ------------------------ input, message to be encode
 *     msg_bytes ------------------ input, byte length of message
 *     e -------------------------- input, RSA public key e, (e_bits+7)/8 bytes, big-endian.
 *     e_bits --------------------- input, bit length of e
 *     n -------------------------- input, RSA modulus n, (n_bits+7)/8 bytes, big-endian.
 *     n_bits --------------------- input, bit length of n
 *     cipher --------------------- output, RSA cipher, (n_bits+7)/8 bytes, big-endian.
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. it is recommended that label_hash_alg and mgf_hash_alg are the same.
 *     2. if no label prepared to input, please set label to NULL
 *     3. if no seed prepared to input, please set seed to NULL, it will be generated inside.
 *     4. msg_bytes should be in [1, em_bytes-2*hLen-2], em_bytes is (n_bits+7)/8, hLen is digest length of hash algorithm msg_hash_alg
 *        it is recommended to use default value, digest length of hash algorithm msg_hash_alg or mgf_hash_alg.
 */
uint32_t eme_oaep_encode(hash_alg_e label_hash_alg, hash_alg_e mgf_hash_alg, const uint8_t *label, 
        uint32_t label_bytes, const uint8_t *seed, const uint8_t *msg, uint32_t msg_bytes, uint8_t *em, uint32_t em_bytes)
{
    uint32_t ret;
    uint8_t label_digest[HASH_DIGEST_MAX_WORD_LEN<<2];

    //get lhash is hash(L)
    ret = hash(label_hash_alg, label, label_bytes, label_digest);
    if(HASH_SUCCESS == ret)
    {
        ret = RSA_SUCCESS;
    }
    else
    {}

    if(RSA_SUCCESS == ret)
    {
        ret = eme_oaep_encode_by_label_digest(label_hash_alg, mgf_hash_alg, label_digest, 
            seed, msg, msg_bytes, em, em_bytes);
    }
    else
    {}

    return ret;
}


/* function: RSA PKCS#1_v2.2 RSAES-OAEP decode with label digest
 * parameters:
 *     label_hash_alg ------------- input, specific hash algorithm for message or Hash(label)
 *     mgf_hash_alg --------------- input, specific hash algorithm for MGF1
 *     label_digest --------------- input, label digest of label_hash_alg
 *     msg ------------------------ output, message to be enrypt
 *     msg_bytes ------------------ output, byte length of message pointer
 *     em ------------------------- input, big integer to be parse, big-endian.
 *     em_bytes ------------------- input, byte length of em, should be bit length of RSA modulus n
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. it is recommended that label_hash_alg and mgf_hash_alg are the same.
 */
uint32_t eme_oaep_decode_by_label_digest(hash_alg_e label_hash_alg, hash_alg_e mgf_hash_alg, const uint8_t *label_digest, 
        uint8_t *msg, uint32_t *msg_bytes, const uint8_t *em, uint32_t em_bytes)
{
    uint32_t ret = RSA_SUCCESS, i;
    uint32_t label_digest_bytes = (uint32_t)hash_get_digest_word_len(label_hash_alg)<<2;  //also byte length of seed
    uint8_t seed[HASH_DIGEST_MAX_WORD_LEN<<2];
    uint8_t db[RSA_MAX_WORD_LEN<<2];
    
    if((NULL == label_digest) || (NULL == msg) || (NULL == msg_bytes) || ((em[0] != (uint8_t)0x00)))
    {
        ret = RSA_INPUT_INVALID;
    }
    else
    {}

    if(RSA_SUCCESS == ret)
    {
        //get Seed = MGF1 xor MFG1(maskedDB, hLen)
        ret = rsa_pkcs1_mgf1_with_xor_in(mgf_hash_alg, &em[1U+label_digest_bytes], 
            em_bytes - 1U - label_digest_bytes, NULL, seed, label_digest_bytes);
    }
    else
    {}
    
    if(ret == RSA_SUCCESS)
    {
        //get DB = maskedDB xor MGF1(seed, k-hLen-1)
        uint8_XOR(seed, &em[1], seed, label_digest_bytes);
        ret = rsa_pkcs1_mgf1_with_xor_in(mgf_hash_alg, seed, label_digest_bytes, (uint8_t *)&em[1U+label_digest_bytes], 
            db, em_bytes - 1U - label_digest_bytes);
    }
    else
    {}

    if(ret == RSA_SUCCESS)
    {
        //check lHash, DB = lHash||PS||0x01||M
        if(0U != memcmp_(label_digest, db, label_digest_bytes))
        {
            ret = RSA_INPUT_INVALID;
        }
        else
        {}
    }
    else
    {}
      
    if(RSA_SUCCESS == ret)
    {
        //get the location of 0x01, DB = lHash||PS||0x01||M
        for(i=label_digest_bytes; i<(em_bytes-label_digest_bytes-1U); i++)    
        {
            if(db[i] != (uint8_t)0x00)
            {
                break;
            }
            else
            {}
        }

        //check the location of 0x01
        if(((em_bytes-label_digest_bytes-1U) == i) || (db[i] != (uint8_t)0x01))
        {
            ret = RSA_INPUT_INVALID;
        }
        else
        {
            //get the msg and msg bytes
            *msg_bytes = em_bytes-label_digest_bytes-1U - (i+1U);
            memcpy_(msg, &db[i+1U], *msg_bytes);
        }
    }

    return ret;
}


/* function: RSA PKCS#1_v2.2 RSAES-OAEP decode
 * parameters:
 *     label_hash_alg ------------- input, specific hash algorithm for message or Hash(label)
 *     mgf_hash_alg --------------- input, specific hash algorithm for MGF1
 *     label ---------------------- input, label
 *     label_bytes ---------------- input, byte length of label
 *     msg ------------------------ output, message to be enrypt
 *     msg_bytes ------------------ output, byte length of message pointer
 *     em ------------------------- input, big integer to be parse, big-endian.
 *     em_bytes ------------------- input, byte length of em, should be bit length of RSA modulus n
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. it is recommended that label_hash_alg and mgf_hash_alg are the same.
 */
uint32_t eme_oaep_decode(hash_alg_e label_hash_alg, hash_alg_e mgf_hash_alg, const uint8_t *label, 
        uint32_t label_bytes, uint8_t *msg, uint32_t *msg_bytes, const uint8_t *em, uint32_t em_bytes)
{
    uint32_t ret = RSA_SUCCESS;
    uint8_t label_digest[HASH_DIGEST_MAX_WORD_LEN<<2];
    
    //check lHash, DB = lHash||PS||0x01||M
    ret = hash(label_hash_alg, label, label_bytes, label_digest);
    if(HASH_SUCCESS == ret)
    {
        ret = RSA_SUCCESS;
    }
    else
    {}

    if(RSA_SUCCESS == ret)
    {
        ret = eme_oaep_decode_by_label_digest(label_hash_alg, mgf_hash_alg, label_digest, 
            msg, msg_bytes, em, em_bytes);
    }
    else
    {}

    return ret;
}


/* function: RSA PKCS#1_v2.2 RSAES-OAEP-ENCRYPT with label digest
 * parameters:
 *     label_hash_alg ------------- input, specific hash algorithm for message or Hash(label)
 *     mgf_hash_alg --------------- input, specific hash algorithm for MGF1
 *     label_digest --------------- input, label digest of label_hash_alg
 *     seed ----------------------- input, Seed, occupy digest bytes of label_hash_alg 
 *     msg ------------------------ input, message to be enrypt
 *     msg_bytes ------------------ input, byte length of message
 *     e -------------------------- input, RSA public key e, (e_bits+7)/8 bytes, big-endian.
 *     e_bits --------------------- input, bit length of e
 *     n -------------------------- input, RSA modulus n, (n_bits+7)/8 bytes, big-endian.
 *     n_bits --------------------- input, bit length of n
 *     cipher --------------------- output, RSA cipher, (n_bits+7)/8 bytes, big-endian.
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. it is recommended that label_hash_alg and mgf_hash_alg are the same.
 *     2. if no seed prepared to input, please set seed to NULL, it will be generated inside.
 *     3. msg_bytes should be in [1, em_bytes-2*hLen-2], em_bytes is (n_bits+7)/8, hLen is digest length of hash algorithm msg_hash_alg
 *        it is recommended to use default value, digest length of hash algorithm msg_hash_alg or mgf_hash_alg.
 */
uint32_t rsa_es_oaep_enc_by_label_digest(hash_alg_e label_hash_alg, hash_alg_e mgf_hash_alg, const uint8_t *label_digest, 
        const uint8_t *seed, const uint8_t *msg, uint32_t msg_bytes, const uint8_t *e, 
        uint32_t e_bits, const uint8_t *n, uint32_t n_bits, uint8_t *cipher)
{
    uint8_t em[RSA_MAX_BYTE_LEN];
    uint32_t ret;

    ret = rsa_es_oaep_param_check(n, n_bits, e, e_bits, msg, cipher);
    if(RSA_SUCCESS == ret)
    {
        ret = eme_oaep_encode_by_label_digest(label_hash_alg, mgf_hash_alg, label_digest, seed, msg, msg_bytes, em, GET_BYTE_LEN(n_bits));
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


/* function: RSA PKCS#1_v2.2 RSAES-OAEP-ENCRYPT with message
 * parameters:
 *     label_hash_alg ------------- input, specific hash algorithm for message or Hash(label)
 *     mgf_hash_alg --------------- input, specific hash algorithm for MGF1
 *     label ---------------------- input, label
 *     label_bytes ---------------- input, byte length of label
 *     seed ----------------------- input, Seed, occupy digest bytes of label_hash_alg 
 *     msg ------------------------ input, message to be enrypt
 *     msg_bytes ------------------ input, byte length of message
 *     e -------------------------- input, RSA public key e, (e_bits+7)/8 bytes, big-endian.
 *     e_bits --------------------- input, bit length of e
 *     n -------------------------- input, RSA modulus n, (n_bits+7)/8 bytes, big-endian.
 *     n_bits --------------------- input, bit length of n
 *     cipher --------------------- output, RSA cipher, (n_bits+7)/8 bytes, big-endian.
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. it is recommended that label_hash_alg and mgf_hash_alg are the same.
 *     2. if no label prepared to input, please set label to NULL
 *     3. if no seed prepared to input, please set seed to NULL, it will be generated inside.
 *     4. msg_bytes should be in [0, em_bytes-2*hLen-2], em_bytes is (n_bits+7)/8, hLen is digest length of hash algorithm msg_hash_alg
 *        it is recommended to use default value, digest length of hash algorithm msg_hash_alg or mgf_hash_alg.
 */
uint32_t rsa_es_oaep_enc(hash_alg_e label_hash_alg, hash_alg_e mgf_hash_alg, const uint8_t *label, 
        uint32_t label_bytes, const uint8_t *seed, const uint8_t *msg, uint32_t msg_bytes, const uint8_t *e, 
        uint32_t e_bits, const uint8_t *n, uint32_t n_bits, uint8_t *cipher)
{
    uint8_t em[RSA_MAX_BYTE_LEN];
    uint32_t ret;

    ret = rsa_es_oaep_param_check(n, n_bits, e, e_bits, msg, cipher);
    if(RSA_SUCCESS == ret)
    {
        ret = eme_oaep_encode(label_hash_alg, mgf_hash_alg, label, label_bytes, seed, msg, msg_bytes, em, GET_BYTE_LEN(n_bits));
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


/* function: RSA PKCS#1_v2.2 RSAES-OAEP-DECRYPT with message
 * parameters:
 *     label_hash_alg ------------- input, specific hash algorithm for message or Hash(label)
 *     mgf_hash_alg --------------- input, specific hash algorithm for MGF1
 *     label ---------------------- input, label
 *     label_bytes ---------------- input, byte length of label
 *     msg ------------------------ output, message to be enrypt
 *     msg_bytes ------------------ output, byte length of message pointer
 *     d -------------------------- input, RSA private key d, (n_bits+7)/8 bytes, big-endian.
 *     n -------------------------- input, RSA modulus n, (n_bits+7)/8 bytes, big-endian.
 *     n_bits --------------------- input, bit length of n
 *     cipher --------------------- input, RSA cipher, (n_bits+7)/8 bytes, big-endian.
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. it is recommended that label_hash_alg and mgf_hash_alg are the same.
 */
uint32_t rsa_es_oaep_dec(hash_alg_e label_hash_alg, hash_alg_e mgf_hash_alg, const uint8_t *label, 
        uint32_t label_bytes, uint8_t *msg, uint32_t *msg_bytes, const uint8_t *d, 
        const uint8_t *n, uint32_t n_bits, const uint8_t *cipher)
{
    uint8_t em[RSA_MAX_BYTE_LEN];
    uint32_t ret;

    ret = rsa_es_oaep_param_check(n, n_bits, d, n_bits, msg, cipher);
    if(RSA_SUCCESS == ret)
    {
        ret = pke_modexp_U8(n, d, cipher, em, n_bits, n_bits, 1);
    }
    else
    {}

    if(RSA_SUCCESS == ret)
    {
        ret = eme_oaep_decode(label_hash_alg, mgf_hash_alg, label, label_bytes, msg, msg_bytes, em, GET_BYTE_LEN(n_bits));
    }
    else
    {}

    return ret;
}


/* function: RSA PKCS#1_v2.2 RSAES-OAEP-DECRYPT with label digest
 * parameters:
 *     label_hash_alg ------------- input, specific hash algorithm for message or Hash(label)
 *     mgf_hash_alg --------------- input, specific hash algorithm for MGF1
 *     label_digest --------------- input, label digest of label_hash_alg
 *     msg ------------------------ output, message to be enrypt
 *     msg_bytes ------------------ output, byte length of message pointer
 *     d -------------------------- input, RSA private key d, (n_bits+7)/8 bytes, big-endian.
 *     n -------------------------- input, RSA modulus n, (n_bits+7)/8 bytes, big-endian.
 *     n_bits --------------------- input, bit length of n
 *     cipher --------------------- input, RSA cipher, (n_bits+7)/8 bytes, big-endian.
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. it is recommended that label_hash_alg and mgf_hash_alg are the same.
 */
uint32_t rsa_es_oaep_dec_by_label_digest(hash_alg_e label_hash_alg, hash_alg_e mgf_hash_alg, const uint8_t *label_digest, 
        uint8_t *msg, uint32_t *msg_bytes, const uint8_t *d, 
        const uint8_t *n, uint32_t n_bits, const uint8_t *cipher)
{
    uint8_t em[RSA_MAX_BYTE_LEN];
    uint32_t ret;

    ret = rsa_es_oaep_param_check(n, n_bits, d, n_bits, msg, cipher);
    if(RSA_SUCCESS == ret)
    {
        ret = pke_modexp_U8(n, d, cipher, em, n_bits, n_bits, 1);
    }
    else
    {}

    if(RSA_SUCCESS == ret)
    {
        ret = eme_oaep_decode_by_label_digest(label_hash_alg, mgf_hash_alg, label_digest, msg, msg_bytes, em, GET_BYTE_LEN(n_bits));
    }
    else
    {}

    return ret;
}


/* function: RSA PKCS#1_v2.2 RSAES-OAEP-DECRYPT with label digest(private key is CRT style)
 * parameters:
 *     label_hash_alg ------------- input, specific hash algorithm for message or Hash(label)
 *     mgf_hash_alg --------------- input, specific hash algorithm for MGF1
 *     label_digest --------------- input, label digest of label_hash_alg
 *     label_bytes ---------------- input, byte length of label
 *     msg ------------------------ output, message to be enrypt
 *     msg_bytes ------------------ output, byte length of message pointer
 *     d -------------------------- input, RSA-CRT private key (p,q,dp,dq,u), every field is (n_bits/2+7)/8 bytes, big-endian.
 *     n -------------------------- input, RSA modulus n, (n_bits+7)/8 bytes, big-endian.
 *     n_bits --------------------- input, bit length of n
 *     cipher --------------------- input, RSA cipher, (n_bits+7)/8 bytes, big-endian.
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. it is recommended that label_hash_alg and mgf_hash_alg are the same.
 */
uint32_t rsa_es_oaep_crt_dec_by_label_digest(hash_alg_e label_hash_alg, hash_alg_e mgf_hash_alg, const uint8_t *label_digest, 
        uint8_t *msg, uint32_t *msg_bytes, const rsa_crt_private_key_st *d, 
        const uint8_t *n, uint32_t n_bits, const uint8_t *cipher)
{
    uint8_t em[RSA_MAX_BYTE_LEN];
    uint32_t ret;

    ret = rsa_es_oaep_param_check(n, n_bits, d, n_bits, msg, cipher);
    if(RSA_SUCCESS == ret)
    {
        ret = RSA_CRTModExp_U8(cipher, d->p, d->q, d->dp, d->dq, d->u, em, n_bits);
    }
    else
    {}

    if(RSA_SUCCESS == ret)
    {
        ret = eme_oaep_decode_by_label_digest(label_hash_alg, mgf_hash_alg, label_digest, msg, msg_bytes, em, GET_BYTE_LEN(n_bits));
    }
    else
    {}

    return ret;
}


/* function: RSA PKCS#1_v2.2 RSAES-OAEP-DECRYPT with message(private key is CRT style)
 * parameters:
 *     label_hash_alg ------------- input, specific hash algorithm for message or Hash(label)
 *     mgf_hash_alg --------------- input, specific hash algorithm for MGF1
 *     label ---------------------- input, label
 *     label_bytes ---------------- input, byte length of label
 *     msg ------------------------ output, message to be enrypt
 *     msg_bytes ------------------ output, byte length of message pointer
 *     d -------------------------- input, RSA-CRT private key (p,q,dp,dq,u), every field is (n_bits/2+7)/8 bytes, big-endian.
 *     n -------------------------- input, RSA modulus n, (n_bits+7)/8 bytes, big-endian.
 *     n_bits --------------------- input, bit length of n
 *     cipher --------------------- input, RSA cipher, (n_bits+7)/8 bytes, big-endian.
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. it is recommended that label_hash_alg and mgf_hash_alg are the same.
 */
uint32_t rsa_es_oaep_crt_dec(hash_alg_e label_hash_alg, hash_alg_e mgf_hash_alg, const uint8_t *label, 
        uint32_t label_bytes, uint8_t *msg, uint32_t *msg_bytes, const rsa_crt_private_key_st *d, 
        const uint8_t *n, uint32_t n_bits, const uint8_t *cipher)
{
    uint8_t em[RSA_MAX_BYTE_LEN];
    uint32_t ret = RSA_SUCCESS;

    ret = rsa_es_oaep_param_check(n, n_bits, d, n_bits, msg, cipher);
    if(RSA_SUCCESS == ret)
    {
        ret = RSA_CRTModExp_U8(cipher, d->p, d->q, d->dp, d->dq, d->u, em, n_bits);
    }
    else
    {}

    if(RSA_SUCCESS == ret)
    {
        ret = eme_oaep_decode(label_hash_alg, mgf_hash_alg, label, label_bytes, msg, msg_bytes, em, GET_BYTE_LEN(n_bits));
    }
    else
    {}

    return ret;
}
#endif

