
#include "../../crypto_include/pke_config.h"


#ifdef SUPPORT_RSASSA_PSS

#include "../../crypto_include/pke/rsa.h"
#include "../../crypto_include/hash_hmac/hash.h"
#include "../../crypto_include/crypto_common/utility.h"
#include "../../crypto_include/trng/trng.h"


/* function: get DB=Padding2||salt and H=Hash(Padding1||mHash||salt)
 * parameters:
 *     msg_hash_alg --------------- input, specific hash algorithm for message or Hash(message)
 *     salt ----------------------- input, salt
 *     salt_bytes ----------------- input, byte length of salt
 *     msg_digest ----------------- input, Hash(message), message is to be signed, here Hash is msg_hash_alg.
 *     msg_digest_bytes ----------- input, byte length of msg_digest or Hash(message)
 *     em ------------------------- output, big integer to be signed, big-endian.
 *     em_bytes ------------------- input, byte length of em, should be bit length of RSA modulus n minus 1.
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. it is recommended that msg_hash_alg and mgf_hash_alg are the same.
 *     2. if no salt prepared to input, please set salt to NULL, it will be generated inside.
 *     3. salt_bytes should be in [0, em_bytes-digest_bytes-2], it is recommended to use default value, digest
 *        length of hash algorithm msg_hash_alg or mgf_hash_alg.
 */
FLAG_STATIC uint32_t emsa_pss_encode_by_msg_digest_internal_step1(hash_alg_e msg_hash_alg, const uint8_t *salt, 
        uint32_t salt_bytes, const uint8_t *msg_digest, uint32_t msg_digest_bytes, uint8_t *em, uint32_t em_bytes)
{
    uint8_t *digest_p;
    uint32_t salt_offset;
    uint32_t ret = RSA_SUCCESS;
    hash_ctx_st hash_ctx[1];  

    if(RSA_SUCCESS == ret)
    {
        //set salt
        salt_offset = em_bytes - 1U - msg_digest_bytes - salt_bytes;
        if((NULL == salt) && (0U != salt_bytes))
        {
            ret = get_rand(&em[salt_offset], salt_bytes);
            if(TRNG_SUCCESS == ret)
            {
                ret = RSA_SUCCESS;
            }
            else
            {}
        }
        else
        {
            memcpy_(&em[salt_offset], salt, salt_bytes);
        }
    }
    else
    {}

    if(RSA_SUCCESS == ret)
    {
        //get DB = PS||0x01||salt, PS||0x01 is Padding2.
        memset_(em, 0, salt_offset - 1U);
        em[salt_offset - 1U] = 0x01;
    }
    else
    {}

    //set H(M') = H(padding1||mhash||salt)
    ret = hash_init(hash_ctx, msg_hash_alg);
    if(ret == HASH_SUCCESS)
    {
        digest_p = &em[salt_offset + salt_bytes];
        memset_(digest_p, 0, 8U);
        ret = hash_update(hash_ctx, digest_p, 8U);
    }
    else
    {}

    if(ret == HASH_SUCCESS)
    {
        ret = hash_update(hash_ctx, msg_digest, msg_digest_bytes);
    }
    else
    {}

    if((ret == HASH_SUCCESS) && (0U != salt_bytes))
    {
        ret = hash_update(hash_ctx, &em[salt_offset], salt_bytes);
    }
    else
    {}

    if(ret == HASH_SUCCESS)
    {
        ret = hash_final(hash_ctx, digest_p);
    }
    else
    {}

    return ret;
}


/* function: get EM=maskedDB||H||bc
 * parameters:
 *     mgf_hash_alg --------------- input, specific hash algorithm for MGF1
 *     msg_digest_bytes ----------- input, byte length of msg_digest or Hash(message)
 *     em ------------------------- output, big integer to be signed, big-endian.
 *     em_bits -------------------- input, bit length of em, should be bit length of RSA modulus n minus 1.
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. before calling this function, emsa_pss_encode_by_msg_digest_internal_step1 must be called
 *     2. it is recommended that msg_hash_alg and mgf_hash_alg are the same.
 */
FLAG_STATIC uint32_t emsa_pss_encode_by_msg_digest_internal_step2(hash_alg_e mgf_hash_alg,
        uint32_t msg_digest_bytes, uint8_t *em, uint32_t em_bits)
{
    uint32_t bits;
    uint32_t em_bytes = GET_BYTE_LEN(em_bits);
    uint32_t ret = RSA_SUCCESS;

    if(ret == HASH_SUCCESS)
    {
        //get maskDB
        ret = rsa_pkcs1_mgf1_with_xor_in(mgf_hash_alg, &em[em_bytes - 1U - msg_digest_bytes], msg_digest_bytes, em, em, em_bytes - 1U - msg_digest_bytes);
    }
    else
    {}

    if(ret == RSA_SUCCESS)
    {
        //clear MSB of maskDB
        bits = em_bits & 7U;
        if(0U != bits)
        {
            em[0] &= (uint8_t)((1U<<bits)-1U);
        }
        else
        {}

        //set last byte
        em[em_bytes-1U] = (uint8_t)0xBC;
    }
    else
    {}

    return ret;
}


/* function: RSA PKCS#1_v2.2 EMSA-PSS-ENCODE
 * parameters:
 *     msg_hash_alg --------------- input, specific hash algorithm for message or Hash(message)
 *     mgf_hash_alg --------------- input, specific hash algorithm for MGF1
 *     salt ----------------------- input, salt
 *     salt_bytes ----------------- input, byte length of salt
 *     msg_digest ----------------- input, Hash(message), message is to be signed, here Hash is msg_hash_alg.
 *     msg_digest_bytes ----------- input, byte length of msg_digest or Hash(message)
 *     em ------------------------- output, big integer to be signed, big-endian.
 *     em_bits -------------------- input, bit length of em, should be bit length of RSA modulus n minus 1.
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. it is recommended that msg_hash_alg and mgf_hash_alg are the same.
 *     2. if no salt prepared to input, please set salt to NULL, it will be generated inside.
 *     3. salt_bytes should be in [0, em_bytes-digest_bytes-2], it is recommended to use default value, digest
 *        length of hash algorithm msg_hash_alg or mgf_hash_alg.
 */
FLAG_STATIC uint32_t emsa_pss_encode_by_msg_digest(hash_alg_e msg_hash_alg, hash_alg_e mgf_hash_alg, const uint8_t *salt, 
        uint32_t salt_bytes, const uint8_t *msg_digest, uint32_t msg_digest_bytes, uint8_t *em, uint32_t em_bits)
{
    uint32_t em_bytes = GET_BYTE_LEN(em_bits);
    uint32_t ret = RSA_SUCCESS;

    if(0U == msg_digest_bytes)
    {
        ret = RSA_INPUT_INVALID;
    }
    else if(em_bytes < (msg_digest_bytes + salt_bytes + 2U))   //this is equal to em_bits < (msg_digest_bytes + salt_bytes)*8 + 9
    {
        ret = RSA_INPUT_INVALID;
    }
    else
    {
        //nothing to do, just for static analysis.
    }

    ret = emsa_pss_encode_by_msg_digest_internal_step1(msg_hash_alg,salt, 
        salt_bytes, msg_digest, msg_digest_bytes, em, em_bytes);
    if(RSA_SUCCESS == ret)
    {
        ret = emsa_pss_encode_by_msg_digest_internal_step2(mgf_hash_alg,
            msg_digest_bytes, em, em_bits);
    }
    else
    {}

    return ret;
}


/* function: check the validity of em structure
 * parameters:
 *     salt_bytes ----------------- input, byte length of salt
 *     msg_digest_bytes ----------- input, byte length of msg_digest or Hash(message)
 *     em ------------------------- input, big integer generated by RSA verified, big-endian.
 *     em_bits -------------------- input, bit length of em, should be bit length of RSA modulus n minus 1.
 *     observed_salt_bytes -------- output, the observed salt bytes, if salt_bytes is -1, the value is from parsing em,
 *                                  otherwise the value is salt_bytes
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. it is recommended that msg_hash_alg and mgf_hash_alg are the same.
 *     2. salt_bytes should be in [0, em_bytes-digest_bytes-2], it is recommended to use default value, digest
 *        length of hash algorithm msg_hash_alg or mgf_hash_alg. if salt_bytes is not known, please set it to -1
 */
FLAG_STATIC uint32_t emsa_pss_verify_by_msg_digest_internal_step1(int32_t salt_bytes,
        uint32_t msg_digest_bytes, const uint8_t *em, uint32_t em_bits, uint32_t *observed_salt_bytes)
{
    uint32_t em_bytes = GET_BYTE_LEN(em_bits);
    uint32_t tmp, ret = RSA_SUCCESS;

    if(0U == msg_digest_bytes)
    {
        ret = RSA_INPUT_INVALID;
    }
    else
    {}

    if(RSA_SUCCESS == ret)
    {
        if(salt_bytes >= 0)
        {
            if(em_bytes < (msg_digest_bytes + (uint32_t)salt_bytes + 2U))   //this is equal to em_bits < (msg_digest_bytes + salt_bytes)*8 + 9
            {
                ret = RSA_INPUT_INVALID;
            }
            else
            {}

            *observed_salt_bytes = (uint32_t)salt_bytes;
        }
        else
        {
            if(em_bytes < (msg_digest_bytes + 2U))   //this is equal to em_bits < (msg_digest_bytes + 0)*8 + 9
            {
                ret = RSA_INPUT_INVALID;
            }
            else
            {}
        }
    }
    else
    {}

    //check LSB bytes
    if((RSA_SUCCESS == ret) && (em[em_bytes-1U] != (uint8_t)0xBC))
    {
        ret = RSA_INPUT_INVALID;
    }
    else
    {}

    if(RSA_SUCCESS == ret)
    {
        //first (8*em_bytes - em_bits) bits should be all 0
        tmp = em_bits & 7U;
        if((0U != tmp) && (((uint8_t)0) != (em[0] & (((uint8_t)0xFFU)<<tmp))))
        {
            ret = RSA_INPUT_INVALID;
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: check padding2 from DB
 * parameters:
 *     salt_bytes ----------------- input, byte length of salt
 *     msg_digest_bytes ----------- input, byte length of msg_digest or Hash(message)
 *     em ------------------------- input, big integer generated by RSA verified, big-endian.
 *     em_bits -------------------- input, bit length of em, should be bit length of RSA modulus n minus 1.
 *     observed_salt_bytes -------- output, the observed salt bytes, if salt_bytes is -1, the value is from parsing em,
 *                                  otherwise the value is salt_bytes
 *     salt_offset ---------------- output, the pointer to salt, value is the salt offset from em
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. before calling this function, emsa_pss_verify_by_msg_digest_internal_step1 must be called
 *     2. it is recommended that msg_hash_alg and mgf_hash_alg are the same.
 *     3. salt_bytes should be in [0, em_bytes-digest_bytes-2], it is recommended to use default value, digest
 *        length of hash algorithm msg_hash_alg or mgf_hash_alg. if salt_bytes is not known, please set it to -1
 */
FLAG_STATIC uint32_t emsa_pss_verify_db_check_padding2(int32_t salt_bytes, uint32_t msg_digest_bytes, const uint8_t *em, uint32_t em_bits, 
        uint32_t *observed_salt_bytes, uint32_t *salt_offset)
{
#if 0
    uint8_t buf[RSA_MAX_WORD_LEN<<2];
#else
    const uint8_t *buf = em;
#endif
    uint32_t em_bytes = GET_BYTE_LEN(em_bits);
    uint32_t i, tmp, ret = RSA_SUCCESS;
    
    //check padding2
    if(salt_bytes >= 0)
    {
        tmp = em_bytes-1U-msg_digest_bytes-1U-(uint32_t)salt_bytes;
        for(i=0U; i<tmp; i++)
        {
            if(((uint8_t)0) != buf[i])
            {
                ret = RSA_INPUT_INVALID;
                break;
            }
            else
            {}
        }

        if(RSA_SUCCESS == ret)
        {
            if(buf[i] != ((uint8_t)0x01))
            {
                ret = RSA_INPUT_INVALID;
            }
            else
            {
                *salt_offset = tmp + 1U;
            }
        }
        else
        {}
    }
    else
    {
        tmp = em_bytes-1U-msg_digest_bytes-1U;
        for(i=0U; i<tmp; i++)
        {
            if(((uint8_t)0) != buf[i])
            {
                break;
            }
            else
            {}
        }

        if(((uint8_t)0x01) == buf[i])
        {
            *salt_offset = i + 1U;
            *observed_salt_bytes = tmp - i;
        }
        else
        {
            ret = RSA_INPUT_INVALID;
        }
    }

    return ret;
}


/* function: recover DB and check padding2
 * parameters:
 *     mgf_hash_alg --------------- input, specific hash algorithm for MGF1
 *     salt_bytes ----------------- input, byte length of salt
 *     msg_digest_bytes ----------- input, byte length of msg_digest or Hash(message)
 *     em ------------------------- input, big integer generated by RSA verified, big-endian.
 *     em_bits -------------------- input, bit length of em, should be bit length of RSA modulus n minus 1.
 *     observed_salt_bytes -------- output, the observed salt bytes, if salt_bytes is -1, the value is from parsing em,
 *                                  otherwise the value is salt_bytes
 *     salt_offset ---------------- output, the pointer to salt, value is the salt offset from em
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. before calling this function, emsa_pss_verify_by_msg_digest_internal_step1 must be called
 *     2. it is recommended that msg_hash_alg and mgf_hash_alg are the same.
 *     3. salt_bytes should be in [0, em_bytes-digest_bytes-2], it is recommended to use default value, digest
 *        length of hash algorithm msg_hash_alg or mgf_hash_alg. if salt_bytes is not known, please set it to -1
 */
FLAG_STATIC uint32_t emsa_pss_verify_by_msg_digest_internal_step2(hash_alg_e mgf_hash_alg, int32_t salt_bytes,
        uint32_t msg_digest_bytes, uint8_t *em, uint32_t em_bits, uint32_t *observed_salt_bytes, uint32_t *salt_offset)
{
#if 0
    uint8_t buf[RSA_MAX_WORD_LEN<<2];
#else
    uint8_t *buf = em;
#endif
    uint32_t em_bytes = GET_BYTE_LEN(em_bits);
    uint32_t tmp, ret = RSA_SUCCESS;

    //recover DB
    ret = rsa_pkcs1_mgf1_with_xor_in(mgf_hash_alg, &em[em_bytes - 1U - msg_digest_bytes], msg_digest_bytes, em, buf,
            em_bytes - 1U - msg_digest_bytes);

    if(RSA_SUCCESS == ret)
    {
        tmp = em_bits & 7U;

        //clear first (8*em_bytes - em_bits) bits
        if(0u != tmp)
        {
            buf[0] &= (uint8_t)((1U<<tmp)-1U);
        }
        else
        {}
    }
    else
    {}

    //check padding2 from DB
    if(RSA_SUCCESS == ret)
    {
        ret = emsa_pss_verify_db_check_padding2(salt_bytes, msg_digest_bytes, em, em_bits, 
                observed_salt_bytes, salt_offset);
    }
    else
    {}

    return ret;
}

/* function: recover H(M')= hash(Padding1||mHash||salt) and check mHash
 * parameters:
 *     msg_hash_alg --------------- input, specific hash algorithm for message or Hash(message)
 *     salt ----------------------- input, salt
 *     salt_bytes ----------------- input, byte length of salt
 *     msg_digest ----------------- input, Hash(message), message is to be verified, here Hash is msg_hash_alg.
 *     msg_digest_bytes ----------- input, byte length of msg_digest or Hash(message)
 *     em ------------------------- input, big integer generated by RSA verified, big-endian.
 *     em_bits -------------------- input, bit length of em, should be bit length of RSA modulus n minus 1.
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. before calling this function, emsa_pss_verify_by_msg_digest_internal_step2 must be called
 *     2. it is recommended that msg_hash_alg and mgf_hash_alg are the same.
 *     3. salt_bytes should be in [0, em_bytes-digest_bytes-2], it is recommended to use default value, digest
 *        length of hash algorithm msg_hash_alg or mgf_hash_alg. if salt_bytes is not known, please set it to -1
 */
FLAG_STATIC uint32_t emsa_pss_verify_by_msg_digest_internal_step3(hash_alg_e msg_hash_alg, const uint8_t *salt, uint32_t salt_bytes,
        const uint8_t *msg_digest, uint32_t msg_digest_bytes, const uint8_t *em, uint32_t em_bits)
{
    uint8_t hash_msg[HASH_DIGEST_MAX_WORD_LEN<<2];
    uint32_t em_bytes = GET_BYTE_LEN(em_bits);
    uint32_t ret = RSA_SUCCESS;

    hash_ctx_st digest_ctx[1];
    if(RSA_SUCCESS == ret)
    {
        //get H(M')
        ret = hash_init(digest_ctx, msg_hash_alg);
    }
    else
    {}

    if(ret == HASH_SUCCESS)
    {
        memset_(hash_msg, 0, 8U);
        ret = hash_update(digest_ctx, hash_msg, 8U);
    }
    else
    {}

    if(ret == HASH_SUCCESS)
    {
        ret = hash_update(digest_ctx, msg_digest, msg_digest_bytes);
    }
    else
    {}

    if((ret == HASH_SUCCESS) && (0U != salt_bytes))
    {
        ret = hash_update(digest_ctx, salt, salt_bytes);
    }
    else
    {}

    if(ret == HASH_SUCCESS)
    {
        ret = hash_final(digest_ctx, hash_msg);
    }
    else
    {}

    if(ret == HASH_SUCCESS)
    {
        if(((uint8_t)0) != memcmp_(hash_msg, &em[em_bytes - 1U - msg_digest_bytes], msg_digest_bytes))
        {
            ret = RSA_INPUT_INVALID;
        }
        else
        {
            ret = RSA_SUCCESS;
        }
    }
    else
    {}

#if 0
    memset_(buf, 0, sizeof(buf));
#endif
    memset_(hash_msg, 0, sizeof(hash_msg));

    return ret;
}


/* function: RSA PKCS#1_v2.2 EMSA-PSS-VERIFY
 * parameters:
 *     msg_hash_alg --------------- input, specific hash algorithm for message or Hash(message)
 *     mgf_hash_alg --------------- input, specific hash algorithm for MGF1
 *     salt_bytes ----------------- input, byte length of salt
 *     msg_digest ----------------- input, Hash(message), message is to be verified, here Hash is msg_hash_alg.
 *     msg_digest_bytes ----------- input, byte length of msg_digest or Hash(message)
 *     em ------------------------- input, big integer generated by RSA verified, big-endian.
 *     em_bits -------------------- input, bit length of em, should be bit length of RSA modulus n minus 1.
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. it is recommended that msg_hash_alg and mgf_hash_alg are the same.
 *     2. salt_bytes should be in [0, em_bytes-digest_bytes-2], it is recommended to use default value, digest
 *        length of hash algorithm msg_hash_alg or mgf_hash_alg. if salt_bytes is not known, please set it to -1
 */
FLAG_STATIC uint32_t emsa_pss_verify_by_msg_digest(hash_alg_e msg_hash_alg, hash_alg_e mgf_hash_alg, int32_t salt_bytes,
        const uint8_t *msg_digest, uint32_t msg_digest_bytes, uint8_t *em, uint32_t em_bits)
{
    uint32_t salt_offset;
    uint32_t observed_salt_bytes = 0U;
    uint32_t ret = RSA_SUCCESS;

    //check the validity of em structure
    ret = emsa_pss_verify_by_msg_digest_internal_step1(salt_bytes,
        msg_digest_bytes, em, em_bits, &observed_salt_bytes);
    if(RSA_SUCCESS == ret)
    {
        //recover DB and check padding2
        ret = emsa_pss_verify_by_msg_digest_internal_step2(mgf_hash_alg, salt_bytes,
            msg_digest_bytes, em, em_bits, &observed_salt_bytes, &salt_offset);
    }
    else
    {}

    if(RSA_SUCCESS == ret)
    {
        //recover H(M')= hash(Padding1||mHash||salt) and check mHash
        ret = emsa_pss_verify_by_msg_digest_internal_step3(msg_hash_alg, &em[salt_offset], observed_salt_bytes,
            msg_digest, msg_digest_bytes, em, em_bits);
    }
    else
    {}

    return ret;
}


/* function: RSA PKCS#1_v2.2 RSASSA-PSS-SIGN with message digest
 * parameters:
 *     msg_hash_alg --------------- input, specific hash algorithm for message or Hash(message)
 *     mgf_hash_alg --------------- input, specific hash algorithm for MGF1
 *     salt ----------------------- input, salt
 *     salt_bytes ----------------- input, byte length of salt
 *     msg_digest ----------------- input, Hash(message), message is to be signed, here Hash is msg_hash_alg.
 *     d -------------------------- input, RSA private key d, (n_bits+7)/8 bytes, big-endian.
 *     n -------------------------- input, RSA modulus n, (n_bits+7)/8 bytes, big-endian.
 *     n_bits --------------------- input, bit length of n
 *     signature ------------------ output, RSA signature, (n_bits+7)/8 bytes, big-endian.
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. it is recommended that msg_hash_alg and mgf_hash_alg are the same.
 *     2. if no salt prepared to input, please set salt to NULL, it will be generated inside.
 *     3. salt_bytes should be in [0, em_bytes-digest_bytes-2], em_bytes is (em_bits+7)/8, em_bits is (n_bits-1).
 *        it is recommended to use default value, digest length of hash algorithm msg_hash_alg or mgf_hash_alg.
 */
uint32_t rsa_ssa_pss_sign_by_msg_digest(hash_alg_e msg_hash_alg, hash_alg_e mgf_hash_alg, const uint8_t *salt, 
        uint32_t salt_bytes, const uint8_t *msg_digest, const uint8_t *d, const uint8_t *n, uint32_t n_bits, 
        uint8_t *signature)
{
    uint8_t em[RSA_MAX_WORD_LEN<<2];
    uint32_t msg_digest_bytes;
    uint32_t tmp, ret = RSA_SUCCESS;

    //n can not be even
    tmp = GET_BYTE_LEN(n_bits);
    if(((uint8_t)0) == (n[tmp-1U] & ((uint8_t)1U)))
    {
        ret = RSA_INPUT_INVALID;
    }
    else
    {}

    if(RSA_SUCCESS == ret)
    {
        msg_digest_bytes = ((uint32_t)(hash_get_digest_word_len(msg_hash_alg)))<<2;

#if 1  //to support such as RSA1025
        em[0] = (uint8_t)0;
        ret = emsa_pss_encode_by_msg_digest(msg_hash_alg, mgf_hash_alg, salt, salt_bytes, msg_digest, msg_digest_bytes,
                (1U==(n_bits&7U))?&em[1]:em, n_bits-1U);
#else
        ret = emsa_pss_encode_by_msg_digest(msg_hash_alg, mgf_hash_alg, salt, salt_bytes, msg_digest, msg_digest_bytes,
                em, n_bits-1);
#endif
    }
    else
    {}

    if(RSA_SUCCESS == ret)
    {
        //signature := em^d mod n
        ret = pke_modexp_U8((const uint8_t *)n, (const uint8_t *)d, (const uint8_t *)em, signature, n_bits, n_bits, 1);
    }
    else
    {}

    memset_(em, 0, sizeof(em));

    return ret;
}


/* function: RSA PKCS#1_v2.2 RSASSA-PSS-SIGN with message
 * parameters:
 *     msg_hash_alg --------------- input, specific hash algorithm for message or Hash(message)
 *     mgf_hash_alg --------------- input, specific hash algorithm for MGF1
 *     salt ----------------------- input, salt
 *     salt_bytes ----------------- input, byte length of salt
 *     msg ------------------------ input, message to be signed
 *     msg_bytes ------------------ input, byte length of message
 *     d -------------------------- input, RSA private key d, (n_bits+7)/8 bytes, big-endian.
 *     n -------------------------- input, RSA modulus n, (n_bits+7)/8 bytes, big-endian.
 *     n_bits --------------------- input, bit length of n
 *     signature ------------------ output, RSA signature, (n_bits+7)/8 bytes, big-endian.
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. it is recommended that msg_hash_alg and mgf_hash_alg are the same.
 *     2. if no salt prepared to input, please set salt to NULL, it will be generated inside.
 *     3. salt_bytes should be in [0, em_bytes-digest_bytes-2], em_bytes is (em_bits+7)/8, em_bits is (n_bits-1).
 *        it is recommended to use default value, digest length of hash algorithm msg_hash_alg or mgf_hash_alg.
 */
uint32_t rsa_ssa_pss_sign(hash_alg_e msg_hash_alg, hash_alg_e mgf_hash_alg, const uint8_t *salt, 
        uint32_t salt_bytes, const uint8_t *msg, uint32_t msg_bytes, const uint8_t *d, const uint8_t *n, 
        uint32_t n_bits, uint8_t *signature)
{
    uint8_t msg_digest[HASH_DIGEST_MAX_WORD_LEN<<2];
    uint32_t ret;

    ret = hash(msg_hash_alg, msg, msg_bytes, msg_digest);
    if(HASH_SUCCESS == ret)
    {
        ret = rsa_ssa_pss_sign_by_msg_digest(msg_hash_alg, mgf_hash_alg, salt, salt_bytes, msg_digest, d, n, n_bits, signature);
        memset_(msg_digest, 0, sizeof(msg_digest));
    }
    else
    {}

    return ret;
}


/* function: RSA PKCS#1_v2.2 RSASSA-PSS-SIGN with message digest(private key is CRT style)
 * parameters:
 *     msg_hash_alg --------------- input, specific hash algorithm for message or Hash(message)
 *     mgf_hash_alg --------------- input, specific hash algorithm for MGF1
 *     salt ----------------------- input, salt
 *     salt_bytes ----------------- input, byte length of salt
 *     msg_digest ----------------- input, Hash(message), message is to be signed, here Hash is msg_hash_alg.
 *     d -------------------------- input, RSA-CRT private key (p,q,dp,dq,u), every field is (n_bits/2+7)/8 bytes, big-endian.
 *     n_bits --------------------- input, bit length of n
 *     signature ------------------ output, RSA signature, (n_bits+7)/8 bytes, big-endian.
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. it is recommended that msg_hash_alg and mgf_hash_alg are the same.
 *     2. if no salt prepared to input, please set salt to NULL, it will be generated inside.
 *     3. salt_bytes should be in [0, em_bytes-digest_bytes-2], em_bytes is (em_bits+7)/8, em_bits is (n_bits-1).
 *        it is recommended to use default value, digest length of hash algorithm msg_hash_alg or mgf_hash_alg.
 */
uint32_t rsa_ssa_pss_crt_sign_by_msg_digest(hash_alg_e msg_hash_alg, hash_alg_e mgf_hash_alg, const uint8_t *salt, 
        uint32_t salt_bytes, const uint8_t *msg_digest, const rsa_crt_private_key_st *d, uint32_t n_bits, 
        uint8_t *signature)
{
    uint32_t em_buf[RSA_MAX_WORD_LEN];
    uint8_t *em = (uint8_t *)em_buf;
    uint32_t p[RSA_MAX_WORD_LEN>>1];
    uint32_t q[RSA_MAX_WORD_LEN>>1];
    uint32_t dp[RSA_MAX_WORD_LEN>>1];
    uint32_t dq[RSA_MAX_WORD_LEN>>1];
    uint32_t u[RSA_MAX_WORD_LEN>>1];
    uint32_t msg_digest_bytes;
    uint32_t tmp, ret;

    msg_digest_bytes = ((uint32_t)(hash_get_digest_word_len(msg_hash_alg)))<<2;

    //clear last word
    if(0U != (n_bits & 0x1FU))
    {
        tmp = GET_WORD_LEN(n_bits) - 1U;
        em_buf[tmp] = 0;
    }
    else
    {}

#if 0  //to support such as RSA1025
    em[0] = (uint8_t)0;
    ret = emsa_pss_encode_by_msg_digest(msg_hash_alg, mgf_hash_alg, salt, salt_bytes, msg_digest, msg_digest_bytes,
            (1==(n_bits&7))?em+1:em, n_bits-1);
#else
    ret = emsa_pss_encode_by_msg_digest(msg_hash_alg, mgf_hash_alg, salt, salt_bytes, msg_digest, msg_digest_bytes,
            em, n_bits-1U);
#endif
    if(RSA_SUCCESS == ret)
    {
        reverse_byte_array(em, (uint8_t *)em, GET_BYTE_LEN(n_bits));

        //clear last word
        tmp = (n_bits>>1);
        if(0U != (tmp & 0x1FU))
        {
            tmp = GET_WORD_LEN(tmp) - 1U;
            p[tmp]  = 0;
            q[tmp]  = 0;
            dp[tmp] = 0;
            dq[tmp] = 0;
            u[tmp]  = 0;
        }
        else
        {}

        tmp = GET_BYTE_LEN(tmp);
        reverse_byte_array(d->p,  (uint8_t *)p,  tmp);
        reverse_byte_array(d->q,  (uint8_t *)q,  tmp);
        reverse_byte_array(d->dp, (uint8_t *)dp, tmp);
        reverse_byte_array(d->dq, (uint8_t *)dq, tmp);
        reverse_byte_array(d->u,  (uint8_t *)u,  tmp);

        //signature = em^d mod n
        ret = RSA_CRTModExp(em_buf, p, q, dp, dq, u, em_buf, n_bits);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        reverse_byte_array(em, (uint8_t *)signature, GET_BYTE_LEN(n_bits));
    }
    else
    {}

    memset_(em, 0, sizeof(em));

    return ret;
}


/* function: RSA PKCS#1_v2.2 RSASSA-PSS-SIGN with message(private key is CRT style)
 * parameters:
 *     msg_hash_alg --------------- input, specific hash algorithm for message or Hash(message)
 *     mgf_hash_alg --------------- input, specific hash algorithm for MGF1
 *     salt ----------------------- input, salt
 *     salt_bytes ----------------- input, byte length of salt
 *     msg ------------------------ input, message to be signed
 *     msg_bytes ------------------ input, byte length of message
 *     d -------------------------- input, RSA-CRT private key (p,q,dp,dq,u), every field is (n_bits/2+7)/8 bytes, big-endian.
 *     n_bits --------------------- input, bit length of n
 *     signature ------------------ output, RSA signature, (n_bits+7)/8 bytes, big-endian.
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. it is recommended that msg_hash_alg and mgf_hash_alg are the same.
 *     2. if no salt prepared to input, please set salt to NULL, it will be generated inside.
 *     3. salt_bytes should be in [0, em_bytes-digest_bytes-2], em_bytes is (em_bits+7)/8, em_bits is (n_bits-1).
 *        it is recommended to use default value, digest length of hash algorithm msg_hash_alg or mgf_hash_alg.
 */
uint32_t rsa_ssa_pss_crt_sign(hash_alg_e msg_hash_alg, hash_alg_e mgf_hash_alg, const uint8_t *salt, 
        uint32_t salt_bytes, const uint8_t *msg, uint32_t msg_bytes, const rsa_crt_private_key_st *d, 
        uint32_t n_bits, uint8_t *signature)
{
    uint8_t msg_digest[HASH_DIGEST_MAX_WORD_LEN<<2];
    uint32_t ret;

    ret = hash(msg_hash_alg, msg, msg_bytes, msg_digest);
    if(HASH_SUCCESS == ret)
    {
        ret = rsa_ssa_pss_crt_sign_by_msg_digest(msg_hash_alg, mgf_hash_alg, salt, salt_bytes, msg_digest, d, n_bits, signature);
        memset_(msg_digest, 0, sizeof(msg_digest));
    }
    else
    {}

    return ret;
}


/* function: RSA PKCS#1_v2.2 RSASSA-PSS-VERIFY with message digest
 * parameters:
 *     msg_hash_alg --------------- input, specific hash algorithm for message or Hash(message)
 *     mgf_hash_alg --------------- input, specific hash algorithm for MGF1
 *     salt_bytes ----------------- input, byte length of salt
 *     msg_digest ----------------- input, Hash(message), message is to be verified, here Hash is msg_hash_alg.
 *     e -------------------------- input, RSA public key e, (e_bits+7)/8 bytes, big-endian.
 *     e_bits --------------------- input, bit length of e
 *     n -------------------------- input, RSA modulus n, (n_bits+7)/8 bytes, big-endian.
 *     n_bits --------------------- input, bit length of n
 *     signature ------------------ input, RSA signature, (n_bits+7)/8 bytes, big-endian.
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. it is recommended that msg_hash_alg and mgf_hash_alg are the same.
 *     2. salt_bytes should be in [0, em_bytes-digest_bytes-2], em_bytes is (em_bits+7)/8, em_bits is (n_bits-1).
 *        it is recommended to use default value, digest length of hash algorithm msg_hash_alg or mgf_hash_alg.
 *        if salt_bytes is not known, please set it to -1
 */
uint32_t rsa_ssa_pss_verify_by_msg_digest(hash_alg_e msg_hash_alg, hash_alg_e mgf_hash_alg, int32_t salt_bytes,
        const uint8_t *msg_digest, const uint8_t *e, uint32_t e_bits, const uint8_t *n, uint32_t n_bits, 
        const uint8_t *signature)
{
    uint8_t em[RSA_MAX_WORD_LEN<<2];
    uint32_t msg_digest_bytes;
    uint32_t tmp, ret = RSA_SUCCESS;

    if((NULL == msg_digest) || (NULL == e) || (NULL == n) || (NULL == signature))
    {
        ret = RSA_BUFFER_NULL;
    }
    else if((e_bits < 2U) || (e_bits > n_bits) || (n_bits > RSA_MAX_BIT_LEN))
    {
        ret = RSA_INPUT_INVALID;
    }
    else
    {
        //nothing to do, just for static analysis.
    }

    if(RSA_SUCCESS == ret)
    {
        //n can not be even
        tmp = GET_BYTE_LEN(n_bits);
        if(((uint8_t)0) == (n[tmp-1U] & ((uint8_t)1U)))
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
        ret = pke_modexp_U8((const uint8_t *)n, (const uint8_t *)e, (const uint8_t *)signature, em, n_bits, e_bits, 1);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
#if 1  //to support such as RSA1025
        tmp = (n_bits&7U);
        if((1U == tmp) && ((((uint8_t)0) != em[0])))
        {
            ret = RSA_INPUT_INVALID;
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            msg_digest_bytes = ((uint32_t)(hash_get_digest_word_len(msg_hash_alg)))<<2;
            ret = emsa_pss_verify_by_msg_digest(msg_hash_alg, mgf_hash_alg, salt_bytes, msg_digest, msg_digest_bytes,
                    (1U==tmp)?&em[1]:em, n_bits-1U);
        }
        else
        {}
#else
        ret = emsa_pss_verify_by_msg_digest(msg_hash_alg, mgf_hash_alg, salt_bytes, msg_digest, msg_digest_bytes,
                    em, n_bits-1U);
#endif
    }
    else
    {}

    memset_(em, 0, sizeof(em));

    return ret;
}


/* function: RSA PKCS#1_v2.2 RSASSA-PSS-VERIFY with message
 * parameters:
 *     msg_hash_alg --------------- input, specific hash algorithm for message or Hash(message)
 *     mgf_hash_alg --------------- input, specific hash algorithm for MGF1
 *     salt_bytes ----------------- input, byte length of salt
 *     msg ------------------------ input, message to be verified
 *     msg_bytes ------------------ input, byte length of message
 *     e -------------------------- input, RSA public key e, (e_bits+7)/8 bytes, big-endian.
 *     e_bits --------------------- input, bit length of e
 *     n -------------------------- input, RSA modulus n, (n_bits+7)/8 bytes, big-endian.
 *     n_bits --------------------- input, bit length of n
 *     signature ------------------ input, RSA signature, (n_bits+7)/8 bytes, big-endian.
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. it is recommended that msg_hash_alg and mgf_hash_alg are the same.
 *     2. salt_bytes should be in [0, em_bytes-digest_bytes-2], em_bytes is (em_bits+7)/8, em_bits is (n_bits-1).
 *        it is recommended to use default value, digest length of hash algorithm msg_hash_alg or mgf_hash_alg.
 *        if salt_bytes is not known, please set it to -1
 */
uint32_t rsa_ssa_pss_verify(hash_alg_e msg_hash_alg, hash_alg_e mgf_hash_alg, int32_t salt_bytes, 
        const uint8_t *msg, uint32_t msg_bytes, const uint8_t *e, uint32_t e_bits, const uint8_t *n, 
        uint32_t n_bits, const uint8_t *signature)
{
    uint8_t msg_digest[HASH_DIGEST_MAX_WORD_LEN<<2];
    uint32_t ret = RSA_SUCCESS;

    if(NULL == msg)
    {
        ret = RSA_BUFFER_NULL;
    }
    else
    {}

    ret = hash(msg_hash_alg, msg, msg_bytes, msg_digest);
    if(HASH_SUCCESS == ret)
    {
        ret = rsa_ssa_pss_verify_by_msg_digest(msg_hash_alg, mgf_hash_alg, salt_bytes, msg_digest,
                e, e_bits, n, n_bits, signature);
    }
    else
    {}

    memset_(msg_digest, 0, sizeof(msg_digest));

    return ret;
}

#endif

