
#include "../../crypto_include/pke_config.h"


#ifdef SUPPORT_SM2

#include "./sm2_internal.h"
#include "../../crypto_include/pke/sm2.h"
#include "../../crypto_include/hash_hmac/hash_kdf.h"
#include "../../crypto_include/trng/trng.h"
#include "../../crypto_include/crypto_common/utility.h"



/* function: Generate SM2 public key from private key
 * parameters:
 *     priKey --------------------- input, private key, 32 bytes, big-endian
 *     pubKey --------------------- output, public key(0x04 + x + y), 65 bytes, big-endian
 * return:
 *     SM2_SUCCESS(success); other(error)
 * caution:
 */
uint32_t sm2_get_pubkey_from_prikey(const uint8_t priKey[32], uint8_t pubKey[65])
{
    uint32_t ret;

    if((NULL == priKey) || (NULL == pubKey))
    {
        ret = SM2_BUFFER_NULL;
    }
    else
    {
        ret = eccp_get_pubkey_from_prikey(sm2_curve, priKey, &pubKey[1]);
        if(PKE_SUCCESS == ret)
        {
            pubKey[0] = POINT_UNCOMPRESSED;

            ret = SM2_SUCCESS;
        }
        else
        {}
    }

    return ret;
}


/* function: Generate SM2 random Key pair
 * parameters:
 *     priKey --------------------- output, private key, 32 bytes, big-endian
 *     pubKey --------------------- output, public key(0x04 + x + y), 65 bytes, big-endian
 * return:
 *     SM2_SUCCESS(success); other(error)
 * caution:
 */
uint32_t sm2_getkey(uint8_t priKey[32], uint8_t pubKey[65])
{
    uint32_t ret;

#if 1
    if((NULL == priKey) || (NULL == pubKey))
    {
        ret = SM2_BUFFER_NULL;
    }
    else
    {
        ret = eccp_getkey(sm2_curve, priKey, &pubKey[1]);
        if(PKE_SUCCESS == ret)
        {
            pubKey[0] = POINT_UNCOMPRESSED;

            ret = SM2_SUCCESS;
        }
        else
        {}
    }

    return ret;
#else

    uint32_t k[SM2_WORD_LEN], tmp[SM2_WORD_LEN<<1];

    if(NULL == priKey || NULL == pubKey)
    {
        ret = SM2_BUFFER_NULL;
    }
    else
    {
        do {
            ret = get_rand((uint8_t *)k, SM2_BYTE_LEN);
            if(TRNG_SUCCESS != ret)
            {
                break;
            }
            else
            {
                //make sure k in [1, n-2]
                ret = uint32_integer_check(k, g_sm2p256v1_n_minus_1, SM2_WORD_LEN, PKE_ZERO_ALL, 
                        PKE_INTEGER_TOO_BIG, PKE_SUCCESS);
            }
        } while((PKE_ZERO_ALL == ret) || (PKE_INTEGER_TOO_BIG == ret));
    }

    if(PKE_SUCCESS == ret)
    {
#ifdef SM2_HIGH_SPEED
        ret = eccp_pointMul_base(sm2_curve, k, tmp, &tmp[SM2_WORD_LEN]);
#else
        ret = eccp_pointMul(sm2_curve, k, sm2_curve->eccp_Gx, sm2_curve->eccp_Gy, tmp, &tmp[SM2_WORD_LEN]);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        pubKey[0] = POINT_UNCOMPRESSED;
        u32little_to_u8big_256bits(k, priKey);
        u32little_to_u8big_256bits(tmp, &pubKey[1u]);
        u32little_to_u8big_256bits(&tmp[SM2_WORD_LEN], &pubKey[1u+SM2_BYTE_LEN]);
        ret = SM2_SUCCESS;
    }
    else
    {}

    return ret;
#endif
}


/* function: Generate SM2 Signature r(internal API)
 * parameters:
 *     e[8] ----------------------- input, e value, 8 words, little-endian
 *     k[8] ----------------------- input, random number k, 8 words, little-endian
 *     r[8] ----------------------- output, Signature r, 8 words, little-endian
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. 
 */
FLAG_STATIC uint32_t sm2_sign_with_k_step_1(const uint32_t e[8], const uint32_t k[8], uint32_t r[8])
{
    uint32_t ret;
    uint32_t tmp[SM2_WORD_LEN];

    //make sure k in [1, n-1]
    ret = uint32_integer_check(k, sm2_curve->eccp_n, SM2_WORD_LEN, SM2_ZERO_ALL, SM2_INTEGER_TOO_BIG,
            PKE_SUCCESS);
    if(PKE_SUCCESS == ret)
    {
#ifdef SM2_HIGH_SPEED
        ret = eccp_pointMul_base(sm2_curve, k, tmp, NULL);
#else
        ret = eccp_pointMul(sm2_curve, k, sm2_curve->eccp_Gx, sm2_curve->eccp_Gy, tmp, NULL);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
#if (defined(PKE_LP) || defined(PKE_SECURE))
        ret = pke_load_modulus_and_pre_monts_256bits(sm2_curve->eccp_n, sm2_curve->eccp_n_h, sm2_curve->eccp_n_n0);
#else
        ret = pke_load_modulus_and_pre_monts_256bits(sm2_curve->eccp_n, sm2_curve->eccp_n_h);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //r = e + x1 mod n
        ret = pke_mod_add_sub_mul_256bits_internal(e, tmp, r, MICROCODE_MODADD);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //make sure r is not zero
        if(0u != uint32_BigNum_Check_Zero(r, SM2_WORD_LEN))
        {
            ret = SM2_ZERO_ALL;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //tmp = r + k mod n
        ret = pke_mod_add_sub_mul_256bits_internal(r, k, tmp, MICROCODE_MODADD);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //make sure r+k is not n
        if(0u != uint32_BigNum_Check_Zero(tmp, SM2_WORD_LEN))
        {
            ret = SM2_ZERO_ALL;
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: Generate SM2 Signature s(internal API)
 * parameters:
 *     k[8]   --------------------- input, random number k, 8 words, little-endian
 *     dA[8]  --------------------- input, private key, 8 words, little-endian
 *     r[8]   --------------------- input, Signature r, 8 words, little-endian
 *     s[8]   --------------------- output, Signature s, 8 words, little-endian
 * return:
 *     SM2_SUCCESS(success); other(error)
 * caution:
 *     1. e and dA can not be modified
 *     2. dA must be in [1, n-2]
 */
FLAG_STATIC uint32_t sm2_sign_with_k_step_2(const uint32_t k[8], const uint32_t dA[8], 
        const uint32_t r[8], uint32_t s[8])
{
    uint32_t tmp1[SM2_WORD_LEN], tmp2[SM2_WORD_LEN];
    uint32_t ret;

    //tmp1 =  r*dA mod n
#if (defined(PKE_LP) || defined(PKE_SECURE))
    pke_set_exe_cfg(PKE_EXE_CFG_ALL_NON_MONT);
#endif
    ret = pke_mod_add_sub_mul_256bits_internal(r, dA, tmp1, MICROCODE_MODMUL);
    if(PKE_SUCCESS == ret)
    {
        //tmp1 =  (k - r*dA) mod n
        ret = pke_mod_add_sub_mul_256bits_internal(k, tmp1, tmp1, MICROCODE_MODSUB);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //tmp2 = 1+dA
        uint32_copy_8_words(tmp2, dA);
        ret = uint32_big_num_little_endian_add_little(tmp2, SM2_WORD_LEN, 1u, (uint8_t)1);
        if(0u != ret)
        {
            ret = PKE_INTEGER_TOO_BIG;
        }
        else
        {
            ret = PKE_SUCCESS;
        }
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //tmp2 = (1+dA)^(-1) mod n
        ret = pke_modinv_256bits(tmp2, tmp2);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //s = ((1+dA)^(-1))*(k - r*dA) mod n
        ret = pke_mod_add_sub_mul_256bits_internal(tmp1, tmp2, s, MICROCODE_MODMUL);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //make sure s is not zero
        if(0u != uint32_BigNum_Check_Zero(s, SM2_WORD_LEN))
        {
            ret = SM2_ZERO_ALL;
        }
        else
        {
            ret = SM2_SUCCESS;
        }
    }
    else
    {}

    return ret;
}


/* function: Generate SM2 Signature r and s with rand k
 * parameters:
 *     e[8]   --------------------- input, e value, 8 words, little-endian
 *     k[8]   --------------------- input, random number k, 8 words, little-endian
 *     dA[8]  --------------------- input, private key, 8 words, little-endian
 *     r[8]   --------------------- output, Signature r, 8 words, little-endian
 *     s[8]   --------------------- output, Signature s, 8 words, little-endian
 * return:
 *     SM2_SUCCESS(success); other(error)
 * caution:
 *     1. e and dA can not be modified
 *     2. dA must be in [1, n-2]
 */
FLAG_STATIC uint32_t sm2_sign_with_k(const uint32_t e[8], const uint32_t k[8], const uint32_t dA[8], 
        uint32_t r[8], uint32_t s[8])
{
    uint32_t ret;

    ret = sm2_sign_with_k_step_1(e, k, r);
    if(PKE_SUCCESS == ret)
    {
        ret = sm2_sign_with_k_step_2(k, dA, r, s);
    }
    else
    {}

    return ret;
}


/* function: Generate SM2 Signature
 * parameters:
 *     E[32] ---------------------- input, E value, 32 bytes, big-endian
 *     rand_k[32] ----------------- input, random big integer k in signing, 32 bytes, big-endian,
 *                                  if you do not have this integer, please set this parameter to be NULL,
 *                                  it will be generated inside.
 *     priKey[32] ----------------- input, private key, 32 bytes, big-endian
 *     signature[64] -------------- output, Signature r and s, 64 bytes, big-endian
 * return:
 *     SM2_SUCCESS(success); other(error)
 * caution:
 *     1. if you do not have rand_k, please set the parameter to be NULL, it will be generated inside.
 */
uint32_t sm2_sign(const uint8_t E[32], const uint8_t rand_k[32], const uint8_t priKey[32], 
        uint8_t signature[64])
{
    uint32_t e_bn[SM2_WORD_LEN], k[SM2_WORD_LEN], dA[SM2_WORD_LEN], r[SM2_WORD_LEN], s[SM2_WORD_LEN];
    uint32_t ret;

    if((NULL == E) || (NULL == priKey) || (NULL == signature))
    {
        ret = SM2_BUFFER_NULL;
    }
    else
    {
        //get e_bn
        u8big_to_u32little_256bits(E, e_bn);

        //make sure priKey in [1, n-2]
        u8big_to_u32little_256bits(priKey, dA);
        ret = uint32_integer_check(dA, g_sm2p256v1_n_minus_1, SM2_WORD_LEN, SM2_ZERO_ALL, 
                SM2_INTEGER_TOO_BIG, SM2_SUCCESS);
    }

    if(SM2_SUCCESS == ret)
    {
        if(NULL == rand_k)
        {
            do {
                ret = get_rand((uint8_t *)k, SM2_BYTE_LEN);
                if(TRNG_SUCCESS != ret)
                {
                    break;
                }
                else
                {}

                ret = sm2_sign_with_k(e_bn, k, dA, r, s);
            } while((SM2_ZERO_ALL == ret) || (SM2_INTEGER_TOO_BIG == ret));
        }
        else
        {
            u8big_to_u32little_256bits(rand_k, k);
            ret = sm2_sign_with_k(e_bn, k, dA, r, s);
        }

        if(SM2_SUCCESS == ret)
        {
#ifndef SUPPORT_STATIC_ANALYSIS
            if(0u == (((uint32_t)signature) & 3u))
            {
                u8big_to_u32little_256bits((uint8_t *)r, (uint32_t *)signature);
                u8big_to_u32little_256bits((uint8_t *)s, (uint32_t *)(&signature[SM2_BYTE_LEN]));
            }
            else
#endif
            {
                u32little_to_u8big_256bits(r, signature);
                u32little_to_u8big_256bits(s, &signature[SM2_BYTE_LEN]);
            }
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: Verify SM2 Signature, get R(internal API)
 * parameters:
 *     E[32] ---------------------- input, E value, 32 bytes, U8 big-endian
 *     pubKey[65] ----------------- input, public key(0x04 + x + y), 65 bytes, U8 big-endian
 *     r[8] ----------------------- input, Signature r, 8 words, U32 little-endian
 *     s[8] ----------------------- input, Signature s, 8 words, U32 little-endian
 *     R[8] ----------------------- output, R = e+x1 mod n, 8 words, U32 little-endian
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 */
FLAG_STATIC uint32_t sm2_verify_internal(const uint8_t E[32], const uint8_t pubKey[65], 
        const uint32_t r[8], const uint32_t s[8], uint32_t R[8])
{
    uint32_t ret;
    uint32_t e_bn[SM2_WORD_LEN], tmp[SM2_WORD_LEN<<2];
    uint32_t *t = e_bn;

    //t = (r+s) mod n
    (void)pke_set_operand_width(256u);
    ret = pke_modadd_modsub_256bits(sm2_curve->eccp_n, r, s, t, MICROCODE_MODADD);
    if(PKE_SUCCESS == ret)
    {
        //if t is 0, refuse the signature
        if(0u != uint32_BigNum_Check_Zero(t, SM2_WORD_LEN))
        {
            ret = SM2_ZERO_ALL;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //get PA and check PA
        u8big_to_u32little_256bits(&pubKey[1u], &tmp[2u*SM2_WORD_LEN]);
        u8big_to_u32little_256bits(&pubKey[1u+SM2_BYTE_LEN], &tmp[3u*SM2_WORD_LEN]);
        ret = eccp_check_point(sm2_curve, &tmp[2u*SM2_WORD_LEN], &tmp[3u*SM2_WORD_LEN]);
        if(PKE_SUCCESS != ret)
        {
            ret = SM2_NOT_ON_CURVE;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
#ifdef SM2_HIGH_SPEED
        ret = eccp_pointMul_Shamir_safe_internal(sm2_curve,
                                        s, sm2_curve->eccp_Gx, sm2_curve->eccp_Gy,
                                        t, &tmp[2u*SM2_WORD_LEN], &tmp[3u*SM2_WORD_LEN],
                                        tmp, NULL);
#else
        //[s]G
        ret = eccp_pointMul_internal(sm2_curve, s, sm2_curve->eccp_Gx, sm2_curve->eccp_Gy, tmp, &tmp[SM2_WORD_LEN]);
        if(PKE_SUCCESS == ret)
        {
            //[t]PA
            ret = eccp_pointMul_internal(sm2_curve, t, &tmp[2u*SM2_WORD_LEN], &tmp[3u*SM2_WORD_LEN], &tmp[2u*SM2_WORD_LEN],
                                &tmp[3u*SM2_WORD_LEN]);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            //[s]G + [t]PA
            ret = eccp_pointAdd_safe_internal(sm2_curve, tmp, &tmp[SM2_WORD_LEN], &tmp[2u*SM2_WORD_LEN], &tmp[3u*SM2_WORD_LEN],
                                tmp, NULL);
        }
        else
        {}
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //R = e + x1 mod n
        u8big_to_u32little_256bits(E, e_bn);
        ret = pke_modadd_modsub_256bits(sm2_curve->eccp_n, e_bn, tmp, R, MICROCODE_MODADD);
    }
    else
    {}

    return ret;
}


/* function: Verify SM2 Signature
 * parameters:
 *     E[32] ---------------------- input, E value, 32 bytes, big-endian
 *     pubKey[65] ----------------- input, public key(0x04 + x + y), 65 bytes, big-endian
 *     signature[64] -------------- input, Signature r and s, 64 bytes, big-endian
 * return:
 *     SM2_SUCCESS(success, the signature is valid); other(error or the signature is invalid)
 * caution:
 */
uint32_t sm2_verify(const uint8_t E[32], const uint8_t pubKey[65], const uint8_t signature[64])
{
    uint32_t r[SM2_WORD_LEN], s[SM2_WORD_LEN];
    uint32_t *R = s;
    uint32_t ret;

    if((NULL == E) || (NULL == pubKey) || (NULL == signature))
    {
        ret = SM2_BUFFER_NULL;
    }
    else if(POINT_UNCOMPRESSED != pubKey[0])    //make sure pubKey[0] is POINT_UNCOMPRESSED
    {
        ret = SM2_INPUT_INVALID;
    }
    else
    {
        //make sure r in [1, n-1]
        u8big_to_u32little_256bits(signature, r);
        ret = uint32_integer_check(r, sm2_curve->eccp_n, SM2_WORD_LEN, SM2_ZERO_ALL, SM2_INTEGER_TOO_BIG, 
                PKE_SUCCESS);
        if(PKE_SUCCESS == ret)
        {
            //make sure s in [1, n-1]
            u8big_to_u32little_256bits(&signature[SM2_BYTE_LEN], s);
            ret = uint32_integer_check(s, sm2_curve->eccp_n, SM2_WORD_LEN, SM2_ZERO_ALL, SM2_INTEGER_TOO_BIG, 
                    PKE_SUCCESS);
        }
        else
        {}
    }

    if(PKE_SUCCESS == ret)
    {
        ret = sm2_verify_internal(E, pubKey, r, s, R);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //cmp
        if(0 != uint32_BigNumCmp(R, SM2_WORD_LEN, r, SM2_WORD_LEN))
        {
            ret = SM2_VERIFY_FAILED;
        }
        else
        {
            ret = SM2_SUCCESS;
        }
    }
    else
    {}

    return ret;
}


/* function: SM2 Encryption with rand k(internal API)
 * parameters:
 *     M -------------------------- input, plaintext, MByteLen bytes, U8 big-endian
 *     MByteLen ------------------- input, byte length of M
 *     k -------------------------- input, random number k, 8 words, U32 little-endian
 *     pubkey_x ------------------- input, x coordinate of public key point, 8 words, U32 little-endian
 *     pubkey_y ------------------- input, y coordinate of public key point, 8 words, U32 little-endian
 *     C1 ------------------------- output, C1 part of ciphertext, 65 bytes, U8 big-endian
 *     C2 ------------------------- output, C2 part of ciphertext, CByteLen-97 bytes, U8 big-endian
 *     C3 ------------------------- output, C3 part of ciphertext, 32 bytes, U8 big-endian
 *     CByteLen ------------------- output, byte length of C, should be MByteLen+97 if success
 * return:
 *     SM2_SUCCESS(success); other(error)
 * caution:
 *     1. M and C can be the same buffer
 *     2. please make sure pubkey_x and pubkey_y are valid
 */
FLAG_STATIC uint32_t sm2_encrypt_with_k_internal(const uint8_t *M, uint32_t MByteLen, 
        const uint32_t *k, const uint32_t *pubkey_x, const uint32_t *pubkey_y,
        uint8_t *C1, uint8_t *C2, uint8_t *C3, uint32_t *CByteLen)
{
    uint32_t ret;
    uint8_t counter_buf[4] = {0,0,0,1};
    const uint8_t *counter = counter_buf;
    uint32_t xy[SM2_WORD_LEN<<1];

    hash_node_st digest_node[3];  //since M and C may point the same address, please do not initialize hash_node here.

    //make sure k in [1, n-1]
    ret = uint32_integer_check(k, sm2_curve->eccp_n, SM2_WORD_LEN, SM2_ZERO_ALL, SM2_INTEGER_TOO_BIG,
            PKE_SUCCESS);
    if(PKE_SUCCESS == ret)
    {
        //get [k]G
#ifdef SM2_HIGH_SPEED
        ret = eccp_pointMul_base(sm2_curve, k, xy, &xy[SM2_WORD_LEN]);
#else
        ret = eccp_pointMul(sm2_curve, k, sm2_curve->eccp_Gx, sm2_curve->eccp_Gy, xy, xy+SM2_WORD_LEN);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //output C1
        C1[0] = POINT_UNCOMPRESSED;
        u32little_to_u8big_256bits(xy, &C1[1u]);
        u32little_to_u8big_256bits(&xy[SM2_WORD_LEN], &C1[1u+SM2_BYTE_LEN]);

        //get [k]PB
        ret = eccp_pointMul_internal(sm2_curve, k, pubkey_x, pubkey_y, xy, &xy[SM2_WORD_LEN]);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //get x2||y2
        u8big_to_u32little_256bits_self(xy);
        u8big_to_u32little_256bits_self(&xy[SM2_WORD_LEN]);

        //get C3
        digest_node[0].msg_addr  = (uint8_t *)xy;
        digest_node[0].msg_bytes = SM2_BYTE_LEN;
        digest_node[1].msg_addr  = M;
        digest_node[1].msg_bytes = MByteLen;
        digest_node[2].msg_addr  = (uint8_t *)(&xy[SM2_WORD_LEN]);
        digest_node[2].msg_bytes = SM2_BYTE_LEN;
        ret = hash_node_steps(HASH_SM3, digest_node, 3u, C3);
        if(HASH_SUCCESS == ret)
        {
            ret = PKE_SUCCESS;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //get C2
#if 0
        digest_node[0].msg_addr  = (uint8_t *)xy;
#endif
        digest_node[0].msg_bytes = SM2_BYTE_LEN<<1;
        digest_node[1].msg_addr  = counter;
        digest_node[1].msg_bytes = 4u;
        ret = ansi_x9_63_kdf_node_with_xor_in(HASH_SM3, digest_node, 2u, counter_buf, M, C2, MByteLen, 1u);
        if(HASH_SUCCESS == ret)
        {
            CByteLen[0] = MByteLen+1u+(3u*SM2_BYTE_LEN);

            ret = SM2_SUCCESS;
        }
        else if(HASH_OUTPUT_ZERO_ALL == ret)
        {
            ret = SM2_ZERO_ALL;
        }
        else
        {
            //nothing to do, just for static analysis.
        }
    }
    else
    {}

    return ret;
}


/* function: SM2 Encryption with rand k
 * parameters:
 *     M -------------------------- input, plaintext, MByteLen bytes, U8 big-endian
 *     MByteLen ------------------- input, byte length of M
 *     k[8] ----------------------- input, random number k, 8 words, U32 little-endian
 *     pubkey_x ------------------- input, x coordinate of public key point, 8 words, U32 little-endian
 *     pubkey_y ------------------- input, y coordinate of public key point, 8 words, U32 little-endian
 *     order ---------------------- input, either SM2_C1C3C2 or SM2_C1C2C3
 *     C -------------------------- output, ciphertext, CByteLen bytes, U8 big-endian
 *     CByteLen ------------------- output, byte length of C, should be MByteLen+97 if success
 * return:
 *     SM2_SUCCESS(success); other(error)
 * caution:
 *     1. M and C can be the same buffer
 *     2. please make sure pubkey_x and pubkey_y are valid
 */
FLAG_STATIC uint32_t sm2_encrypt_with_k(const uint8_t *M, uint32_t MByteLen, const uint32_t *k,
        const uint32_t *pubkey_x, const uint32_t *pubkey_y, sm2_cipher_order_e order,
        uint8_t *C, uint32_t *CByteLen)
{
    const uint8_t *plain = M;
    uint8_t *C2, *C3;
    uint32_t i;
    uint32_t ret;

    if((NULL == M) || (NULL == k) || (NULL == pubkey_x) || (NULL == pubkey_y) || (NULL == C) || (NULL == CByteLen))
    {
        ret = SM2_BUFFER_NULL;
    }
    else if((0u == MByteLen) || (order > SM2_C1C2C3))
    {
        ret = SM2_INPUT_INVALID;
    }
    else
    {
        ret = SM2_SUCCESS;
    }

    if(SM2_SUCCESS == ret)
    {
        C2 = &C[1u+(2u*SM2_BYTE_LEN) + ((SM2_C1C2C3 == order)?0u:SM2_BYTE_LEN)];
        C3 = &C[1u+(2u*SM2_BYTE_LEN) + ((SM2_C1C2C3 == order)?MByteLen:0u)];

        //not support M and C crossing, but support M = C
        if(M > C)
        {
            if((&C[MByteLen+1u+(3u*SM2_BYTE_LEN)]) > M)
            {
                ret = SM2_INPUT_INVALID;
            }
            else
            {}
        }
        else if(M < C)
        {
            if((&M[MByteLen]) > C)
            {
                ret = SM2_INPUT_INVALID;
            }
            else
            {}
        }
        else  //M = C
        {
            //move M to C2, and now M = C2
            i = MByteLen;
            while(i>0u)
            {
                --i;
                C2[i] = M[i];
            }

            plain = C2;
        }
    }
    else
    {}

    if(SM2_SUCCESS == ret)
    {
        ret = sm2_encrypt_with_k_internal(plain, MByteLen, k, pubkey_x, pubkey_y,
                C, C2, C3, CByteLen);
    }
    else
    {}

    return ret;
}


/* function: SM2 Encryption
 * parameters:
 *     M -------------------------- input, plaintext, MByteLen bytes, big-endian
 *     MByteLen ------------------- input, byte length of M
 *     rand_k[32] ----------------- input, random big integer k in encrypting, 32 bytes, big-endian,
 *                                  if you do not have this integer, please set this parameter to be NULL,
 *                                  it will be generated inside.
 *     pubKey[65] ----------------- input, public key, 65 bytes, big-endian
 *     order ---------------------- input, either SM2_C1C3C2 or SM2_C1C2C3
 *     C -------------------------- output, ciphertext, CByteLen bytes, big-endian
 *     CByteLen ------------------- output, byte length of C, should be MByteLen+97 if success
 * return:
 *     SM2_SUCCESS(success); other(error)
 * caution:
 *     1. M and C can be the same buffer
 *     2. if you do not have rand_k, please set the parameter to be NULL, it will be generated inside.
 *     3. please make sure pubKey is valid
 */
uint32_t sm2_encrypt(const uint8_t *M, uint32_t MByteLen, const uint8_t rand_k[32], 
        const uint8_t pubKey[65], sm2_cipher_order_e order, uint8_t *C, uint32_t *CByteLen)
{
    uint32_t k[SM2_WORD_LEN];
    uint32_t pubkey_x[SM2_WORD_LEN],pubkey_y[SM2_WORD_LEN];
    uint32_t ret;

    if(NULL == pubKey)
    {
        ret = SM2_BUFFER_NULL;
    }
    else if(POINT_UNCOMPRESSED != pubKey[0])
    {
        ret = SM2_INPUT_INVALID;
    }
    else
    {
        u8big_to_u32little_256bits(&pubKey[1u], pubkey_x);
        u8big_to_u32little_256bits(&pubKey[1u+SM2_BYTE_LEN], pubkey_y);
        ret = eccp_check_point(sm2_curve, pubkey_x, pubkey_y);
        if(PKE_SUCCESS != ret)
        {
            ret = SM2_NOT_ON_CURVE;
        }
        else
        {}
    }

    if(PKE_SUCCESS == ret)
    {
        if(NULL == rand_k)
        {
            do {
                ret = get_rand((uint8_t *)k, SM2_BYTE_LEN);
                if(TRNG_SUCCESS != ret)
                {
                    break;
                }
                else
                {}

                ret = sm2_encrypt_with_k(M, MByteLen, k, pubkey_x, pubkey_y, order, C, CByteLen);
            } while((SM2_ZERO_ALL == ret) || (SM2_INTEGER_TOO_BIG == ret));
        }
        else
        {
            u8big_to_u32little_256bits(rand_k, k);
            ret = sm2_encrypt_with_k(M, MByteLen, k, pubkey_x, pubkey_y, order, C, CByteLen);
        }
    }
    else
    {}

    return ret;
}


/* function: SM2 Decryption(internal API)
 * parameters:
 *     xy ------------------------- input, x & y coordinates of C1, 8+8 words, U32 little-endian
 *     C -------------------------- input, ciphertext, CByteLen bytes, big-endian
 *     C2_bytes ------------------- input, byte length of C2
 *     priKey --------------------- input, private key, 32 bytes, big-endian
 *     order ---------------------- input, either SM2_C1C3C2 or SM2_C1C2C3
 *     M -------------------------- output, plaintext, MByteLen bytes, big-endian
 *     MByteLen ------------------- output, byte length of M, should be CByteLen-97 if success
 * return:
 *     SM2_SUCCESS(success); other(error)
 * caution:
 *     1. M and C can be the same buffer
 */
FLAG_STATIC uint32_t sm2_decrypt_internal(uint32_t *xy, const uint8_t *C, uint32_t C2_bytes, 
        const uint8_t priKey[32], sm2_cipher_order_e order, uint8_t *M, uint32_t *MByteLen)
{
    uint32_t ret, i;
    uint8_t counter_buf[4] = {0,0,0,1};
    const uint8_t *counter = counter_buf;
    uint32_t dA[SM2_WORD_LEN];
    uint8_t digest[SM2_BYTE_LEN];
    uint8_t C3_buf[SM2_BYTE_LEN];
    const uint8_t *C2;
    const uint8_t *C3;

    hash_node_st digest_node[3];

    digest_node[0].msg_addr  = (uint8_t *)xy;
    digest_node[0].msg_bytes = SM2_BYTE_LEN<<1;
    digest_node[1].msg_addr  = counter;
    digest_node[1].msg_bytes = 4U;
    digest_node[2].msg_addr  = (uint8_t *)(&xy[SM2_WORD_LEN]);
    digest_node[2].msg_bytes = SM2_BYTE_LEN;

    C2 = &C[1u+(2u*SM2_BYTE_LEN) +((SM2_C1C2C3 == order)?0u:SM2_BYTE_LEN)];
    C3 = &C[1u+(2u*SM2_BYTE_LEN) +((SM2_C1C2C3 == order)?C2_bytes:0u)];

    if(M == C)  //M = C
    {
        //keep C3
        memcpy_(C3_buf, C3, SM2_BYTE_LEN);
        C3 = C3_buf;

        //move C2 to M, and now M = C2
        for(i=0; i<C2_bytes; i++)
        {
            M[i] = C2[i];
        }

        C2 = M;
    }
    else
    {}

    //make sure priKey in [1, n-2]
    u8big_to_u32little_256bits(priKey, dA);
    ret = uint32_integer_check(dA, g_sm2p256v1_n_minus_1, SM2_WORD_LEN, SM2_ZERO_ALL, 
            SM2_INTEGER_TOO_BIG, PKE_SUCCESS);
    if(PKE_SUCCESS == ret)
    {
        //[dA]C1
        ret = eccp_pointMul_internal(sm2_curve, dA, xy, &xy[SM2_WORD_LEN], xy, &xy[SM2_WORD_LEN]);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        u8big_to_u32little_256bits_self(xy);
        u8big_to_u32little_256bits_self(&xy[SM2_WORD_LEN]);

        ret = ansi_x9_63_kdf_node_with_xor_in(HASH_SM3, digest_node, 2u, counter_buf, C2, M, C2_bytes, 1u);
        if(HASH_SUCCESS == ret)
        {
            ret = PKE_SUCCESS;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
#if 0
        digest_node[0].msg_addr  = (uint8_t *)xy;
#endif
        digest_node[0].msg_bytes = SM2_BYTE_LEN;
        digest_node[1].msg_addr  = M;
        digest_node[1].msg_bytes = C2_bytes;
#if 0
        digest_node[2].msg_addr  = (uint8_t *)(&xy[SM2_WORD_LEN]);
        digest_node[2].msg_bytes = SM2_BYTE_LEN;
#endif
        ret = hash_node_steps(HASH_SM3, digest_node, 3u, digest);
        if(HASH_SUCCESS == ret)
        {
            if(((uint8_t)0) != memcmp_(C3, digest, SM2_BYTE_LEN))
            {
                ret = SM2_DECRYPT_VERIFY_FAILED;
            }
            else
            {
                *MByteLen = C2_bytes;
                ret = SM2_SUCCESS;
            }
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: SM2 Decryption
 * parameters:
 *     C -------------------------- input, ciphertext, CByteLen bytes, big-endian
 *     CByteLen ------------------- input, byte length of C, make sure MByteLen>97
 *     priKey --------------------- input, private key, 32 bytes, big-endian
 *     order ---------------------- input, either SM2_C1C3C2 or SM2_C1C2C3
 *     M -------------------------- output, plaintext, MByteLen bytes, big-endian
 *     MByteLen ------------------- output, byte length of M, should be CByteLen-97 if success
 * return:
 *     SM2_SUCCESS(success); other(error)
 * caution:
 *     1. M and C can be the same buffer
 */
uint32_t sm2_decrypt(const uint8_t *C, uint32_t CByteLen, const uint8_t priKey[32],
        sm2_cipher_order_e order, uint8_t *M, uint32_t *MByteLen)
{
    uint32_t ret;
    uint32_t C2_bytes;
    uint32_t xy[SM2_WORD_LEN<<1];

    if((NULL == C) || (NULL == priKey) || (NULL == M) || (NULL == MByteLen))
    {
        ret = SM2_BUFFER_NULL;
    }
    else if((CByteLen <= (1u+(3u*SM2_BYTE_LEN))) || (order > SM2_C1C2C3))  //97 = 1+3*ECCP_BYTELEN
    {
        ret = SM2_INPUT_INVALID;
    }
    else
    {
        C2_bytes = CByteLen-1u-(3u*SM2_BYTE_LEN);
        ret = PKE_SUCCESS;
    }

    if(PKE_SUCCESS == ret)
    {
        //not support M and C crossing, but support M = C
        if(M > C)
        {
            if((&C[CByteLen]) > M)
            {
                ret = SM2_INPUT_INVALID;
            }
            else
            {}
        }
        else if(M < C)
        {
            if((&M[C2_bytes]) > C)
            {
                ret = SM2_INPUT_INVALID;
            }
            else
            {}
        }
        else  //M = C
        {
            //nothing to do, just for static analysis.
        }

        //make sure C1 is on the SM2 curve
        u8big_to_u32little_256bits(&C[1u], xy);
        u8big_to_u32little_256bits(&C[1u+SM2_BYTE_LEN], &xy[SM2_WORD_LEN]);
        ret = eccp_check_point(sm2_curve, xy, &xy[SM2_WORD_LEN]);
        if(PKE_SUCCESS != ret)
        {
            ret = SM2_NOT_ON_CURVE;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm2_decrypt_internal(xy, C, C2_bytes, priKey, order, M, MByteLen);
    }
    else
    {}

    return ret;
}


/* function: SM2 Key Exchange check input
 * parameters:
 *     role ----------------------- input, SM2_Role_Sponsor - sponsor, SM2_Role_Responsor - responsor
 *     dA ------------------------- input, pointer to local's permanent private key, 32 bytes, U8 big-endian
 *     PB ------------------------- input, pointer to peer's permanent public key, 65 bytes, U8 big-endian
 *     rA ------------------------- input, pointer to local's temporary private key, 32 bytes, U8 big-endian
 *     RA ------------------------- input, pointer to local's temporary public key, 65 bytes, U8 big-endian
 *     RB ------------------------- input, pointer to peer's temporary public key, 65 bytes, U8 big-endian
 *     ZA ------------------------- input, pointer to local's Z value, 32 bytes, U8 big-endian
 *     ZB ------------------------- input, pointer to peer's Z value, 32 bytes, U8 big-endian
 *     kByteLen ------------------- input, byte length of output key, can not be zero
 *     KA ------------------------- input, pointer to the output key
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution: 
 *     1. 
 */
FLAG_STATIC uint32_t sm2_exchangekey_check_input(sm2_exchange_role_e role,
                        const uint8_t *dA, const uint8_t *PB,
                        const uint8_t *rA, const uint8_t *RA,
                        const uint8_t *RB,
                        const uint8_t *ZA, const uint8_t *ZB,
                        uint32_t kByteLen,
                        const uint8_t *KA)
{
    uint32_t ret;

    if((NULL == dA) || (NULL == PB) || (NULL == rA) || (NULL == RA) || (NULL == RB))
    {
        ret = SM2_BUFFER_NULL;
    }
    else if((NULL == ZA) || (NULL == ZB) || (NULL == KA))
    {
        ret = SM2_BUFFER_NULL;
    }
    else if(role > SM2_Role_Responsor)
    {
        ret = SM2_EXCHANGE_ROLE_INVALID;
    }
    else if(0u == kByteLen)
    {
        ret = SM2_INPUT_INVALID;
    }
    else
    {
        ret = PKE_SUCCESS;
    }

    if(PKE_SUCCESS == ret)
    {
        if((POINT_UNCOMPRESSED != PB[0]) || (POINT_UNCOMPRESSED != RA[0]) || (POINT_UNCOMPRESSED != RB[0]))
        {
            ret = SM2_INPUT_INVALID;
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: SM2 Key Exchange step 1(internal API)
 * parameters:
 *     dA ------------------------- input, local's permanent private key, 32 bytes, U8 big-endian
 *     rA ------------------------- input, local's temporary private key, 32 bytes, U8 big-endian
 *     RA ------------------------- input, local's temporary public key, 65 bytes, U8 big-endian
 *     tA ------------------------- output, tA = (dA + x1*rA) mod n, 8 words, U32 little-endian
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution: 
 *     1. 
 */
FLAG_STATIC uint32_t sm2_exchangekey_internal_setp_1(const uint8_t *dA, const uint8_t *rA, 
        const uint8_t *RA, uint32_t *tA)
{
    uint32_t ret;
    uint32_t x1[SM2_WORD_LEN];

    u8big_to_u32little_256bits(&RA[1u], x1);
    u8big_to_u32little_256bits(&RA[1u+SM2_BYTE_LEN], tA);
    if(PKE_SUCCESS != eccp_check_point(sm2_curve, x1, tA))
    {
        ret = SM2_NOT_ON_CURVE;
    }
    else
    {
        //get x1
        uint32_clear(&x1[SM2_WORD_LEN>>1], SM2_WORD_LEN>>1);
        x1[(SM2_WORD_LEN>>1)-1u] |= 0x80000000u;

        //make sure rA in [1, n-2]
        u8big_to_u32little_256bits(rA, tA);
        ret = uint32_integer_check(tA, g_sm2p256v1_n_minus_1, SM2_WORD_LEN, SM2_ZERO_ALL, 
                SM2_INTEGER_TOO_BIG, PKE_SUCCESS);
    }

    if(PKE_SUCCESS == ret)
    {
#if (defined(PKE_LP) || defined(PKE_SECURE))
        ret = pke_load_modulus_and_pre_monts_256bits(sm2_curve->eccp_n, sm2_curve->eccp_n_h, sm2_curve->eccp_n_n0);
#else
        ret = pke_load_modulus_and_pre_monts_256bits(sm2_curve->eccp_n, sm2_curve->eccp_n_h);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //tA = x1*rA mod n
#if (defined(PKE_LP) || defined(PKE_SECURE))
        pke_set_exe_cfg(PKE_EXE_CFG_ALL_NON_MONT);
#endif
        ret = pke_mod_add_sub_mul_256bits_internal(x1, tA, tA, MICROCODE_MODMUL);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //make sure dA in [1, n-2]
        u8big_to_u32little_256bits(dA, x1);
        ret = uint32_integer_check(x1, g_sm2p256v1_n_minus_1, SM2_WORD_LEN, SM2_ZERO_ALL, 
                SM2_INTEGER_TOO_BIG, PKE_SUCCESS);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //tA = (dA + x1*rA) mod n, and it must not be 0
        ret = pke_mod_add_sub_mul_256bits_internal(tA, x1, tA, MICROCODE_MODADD);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        if(0u != uint32_BigNum_Check_Zero(tA, SM2_WORD_LEN))
        {
            ret = SM2_ZERO_ALL;
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: SM2 Key Exchange step 2(internal API)
 * parameters:
 *     RB ------------------------- input, peer's temporary public key, 65 bytes, U8 big-endian
 *     PB ------------------------- input, peer's permanent public key, 65 bytes, U8 big-endian
 *     tA ------------------------- input, tA = (dA + x1*rA) mod n, 8 words, U32 little-endian
 *     xy ------------------------- input, x||y, x and y coordiantes of the point U, 8+8=16 words, U32 little-endian
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution: 
 *     1. 
 */
FLAG_STATIC uint32_t sm2_exchangekey_internal_setp_2(const uint8_t *RB, const uint8_t *PB, 
        const uint32_t *tA, uint32_t *xy)
{
    uint32_t ret;
    uint32_t x2[SM2_WORD_LEN], tmp[SM2_WORD_LEN<<1];

    //make sure RB on the SM2 curve
    u8big_to_u32little_256bits(&RB[1u], tmp);
    u8big_to_u32little_256bits(&RB[1u+SM2_BYTE_LEN], &tmp[SM2_WORD_LEN]);
    ret = eccp_check_point(sm2_curve, tmp, &tmp[SM2_WORD_LEN]);
    if(PKE_SUCCESS != ret)
    {
        ret = SM2_NOT_ON_CURVE;
    }
    else
    {
        uint32_copy(x2, tmp, SM2_WORD_LEN>>1);
        uint32_clear(&x2[SM2_WORD_LEN>>1], SM2_WORD_LEN>>1);
        x2[(SM2_WORD_LEN>>1)-1u] |= 0x80000000u;

#ifdef SM2_HIGH_SPEED
#if 0
        ret = pke_set_modulus_and_pre_monts(sm2_curve->eccp_n, sm2_curve->eccp_n_h, SM2_BIT_LEN);
#else
        ret = pke_load_modulus_and_pre_monts_256bits(sm2_curve->eccp_n, sm2_curve->eccp_n_h);
#endif

        if(PKE_SUCCESS == ret)
        {
            //x2 = tA*x2 mod n
            ret = pke_mod_add_sub_mul_256bits_internal(tA, x2, x2, MICROCODE_MODMUL);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            //get PB point and verify
            u8big_to_u32little_256bits(&PB[1u], xy);
            u8big_to_u32little_256bits(&PB[1u+SM2_BYTE_LEN], &xy[SM2_WORD_LEN]);
            ret = eccp_check_point(sm2_curve, xy, &xy[SM2_WORD_LEN]);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            //[tA]PB +[tA*x2 mod n]RB
            ret = eccp_pointMul_Shamir_safe_internal(sm2_curve,
                                            x2, tmp, &tmp[SM2_WORD_LEN],
                                            tA, xy, &xy[SM2_WORD_LEN],
                                            xy, &xy[SM2_WORD_LEN]);
        }
        else
        {}
#else
        ret = eccp_pointMul_internal(sm2_curve, x2, tmp, &tmp[SM2_WORD_LEN], tmp, &tmp[SM2_WORD_LEN]);
        if(PKE_SUCCESS == ret)
        {
            //get PB point(caution: do not delete this)
            u8big_to_u32little_256bits(&PB[1u], xy);
            u8big_to_u32little_256bits(&PB[1u+SM2_BYTE_LEN], &xy[SM2_WORD_LEN]);
            ret = eccp_check_point_internal(sm2_curve, xy, &xy[SM2_WORD_LEN]);
            if(PKE_SUCCESS != ret)
            {
                ret = SM2_NOT_ON_CURVE;
            }
            else
            {}
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            ret = eccp_pointAdd_safe_internal(sm2_curve, tmp, tmp+SM2_WORD_LEN, xy, &xy[SM2_WORD_LEN],
                    xy, &xy[SM2_WORD_LEN]);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            ret = eccp_pointMul_internal(sm2_curve, tA, xy, &xy[SM2_WORD_LEN], xy, &xy[SM2_WORD_LEN]);
        }
        else
        {}
#endif
    }

    return ret;
}


/* function: SM2 Key Exchange step 3(internal API)
 * parameters:
 *     role ----------------------- input, SM2_Role_Sponsor - sponsor, SM2_Role_Responsor - responsor
 *     xy ------------------------- input, x||y, x and y coordiantes of the point U, 8+8=16 words, U32 little-endian
 *     RA ------------------------- input, local's temporary public key, 65 bytes, U8 big-endian
 *     RB ------------------------- input, peer's temporary public key, 65 bytes, U8 big-endian
 *     ZA ------------------------- input, local's Z value, 32 bytes, U8 big-endian
 *     ZB ------------------------- input, peer's Z value, 32 bytes, U8 big-endian
 *     kByteLen ------------------- input, byte length of output key, can not be zero
 *     KA ------------------------- output, output key
 *     S1 ------------------------- output, sponsor's S1, or responsor's S2, 32 bytes, U8 big-endian
 *     SA ------------------------- output, sponsor's SA, or responsor's SB, 32 bytes, U8 big-endian
 * return:
 *     SM2_SUCCESS(success); other(error)
 * caution: 
 *     1. 
 */
FLAG_STATIC uint32_t sm2_exchangekey_internal_setp_3(sm2_exchange_role_e role,
                        uint32_t *xy, const uint8_t *RA,
                        const uint8_t *RB,
                        const uint8_t *ZA, const uint8_t *ZB,
                        uint32_t kByteLen,
                        uint8_t *KA, uint8_t *S1, uint8_t *SA)
{
    uint32_t ret;
    uint8_t counter_buf[4] = {0,0,0,1};
    const uint8_t *counter = counter_buf;
    uint32_t tmp[SM2_WORD_LEN];

    hash_node_st digest_node[5];

    //xU||yU
    u8big_to_u32little_256bits_self(xy);
    u8big_to_u32little_256bits_self(&xy[SM2_WORD_LEN]);

    digest_node[0].msg_addr  = (uint8_t *)xy;
    digest_node[0].msg_bytes = SM2_BYTE_LEN<<1;
    if(SM2_Role_Sponsor == role)
    {
        digest_node[1].msg_addr  = ZA;
        digest_node[2].msg_addr  = ZB;
    }
    else
    {
        digest_node[1].msg_addr  = ZB;
        digest_node[2].msg_addr  = ZA;
    }
    digest_node[1].msg_bytes = SM2_BYTE_LEN;
    digest_node[2].msg_bytes = SM2_BYTE_LEN;
    digest_node[3].msg_addr  = counter;
    digest_node[3].msg_bytes = 4u;

    //KA
    ret = ansi_x9_63_kdf_node(HASH_SM3, digest_node, 4u, counter_buf, KA, kByteLen, NULL, 0u);
    if((HASH_SUCCESS == ret) && (NULL != S1) && (NULL != SA))
    {
        //t1 := hash(xu||ZA||ZB||x1||y1||x2||y2)
        digest_node[0].msg_addr  = (uint8_t *)xy;
        digest_node[0].msg_bytes = SM2_BYTE_LEN;

        if(SM2_Role_Sponsor == role)
        {
            digest_node[1].msg_addr  = ZA;
            digest_node[2].msg_addr  = ZB;
            digest_node[3].msg_addr  = &RA[1u];
            digest_node[4].msg_addr  = &RB[1u];
        }
        else
        {
            digest_node[1].msg_addr  = ZB;
            digest_node[2].msg_addr  = ZA;
            digest_node[3].msg_addr  = &RB[1u];
            digest_node[4].msg_addr  = &RA[1u];
        }

        digest_node[1].msg_bytes = SM2_BYTE_LEN;
        digest_node[2].msg_bytes = SM2_BYTE_LEN;
        digest_node[3].msg_bytes = SM2_BYTE_LEN<<1;
        digest_node[4].msg_bytes = SM2_BYTE_LEN<<1;

        ret = hash_node_steps(HASH_SM3, digest_node, 5u, (uint8_t *)tmp);
        if(HASH_SUCCESS == ret)
        {
            //get SA := hash(0x03||yu||t1)
            ((uint8_t *)(xy))[SM2_BYTE_LEN-1u] = (uint8_t)0x03;
            digest_node[0].msg_addr  = &(((uint8_t *)(xy))[SM2_BYTE_LEN-1u]);
            digest_node[0].msg_bytes = SM2_BYTE_LEN+1u;
            digest_node[1].msg_addr  = (uint8_t *)tmp;
            digest_node[1].msg_bytes = SM2_BYTE_LEN;
            if(SM2_Role_Sponsor == role)
            {
                ret = hash_node_steps(HASH_SM3, digest_node, 2u, (uint8_t *)SA);
            }
            else
            {
                ret = hash_node_steps(HASH_SM3, digest_node, 2u, (uint8_t *)S1);
            }
        }
        else
        {}

        if(HASH_SUCCESS == ret)
        {
            //get S1 := hash(0x02||yu||t1)
            ((uint8_t *)(xy))[SM2_BYTE_LEN-1u] = (uint8_t)0x02;
            if(SM2_Role_Sponsor == role)
            {
                ret = hash_node_steps(HASH_SM3, digest_node, 2u, (uint8_t *)S1);
            }
            else
            {
                ret = hash_node_steps(HASH_SM3, digest_node, 2u, (uint8_t *)SA);
            }
        }
        else
        {}
    }
    else
    {}

    if(HASH_SUCCESS == ret)
    {
        ret = SM2_SUCCESS;
    }
    else
    {}

    return ret;
}


/* function: SM2 Key Exchange
 * parameters:
 *     role ----------------------- input, SM2_Role_Sponsor - sponsor, SM2_Role_Responsor - responsor
 *     dA[32] --------------------- input, local's permanent private key
 *     PB[65] --------------------- input, peer's permanent public key
 *     rA[32] --------------------- input, local's temporary private key
 *     RA[65] --------------------- input, local's temporary public key
 *     RB[65] --------------------- input, peer's temporary public key
 *     ZA[32] --------------------- input, local's Z value
 *     ZB[32] --------------------- input, peer's Z value
 *     kByteLen ------------------- input, byte length of output key, should be less than (2^32 - 1)bit
 *     KA[kByteLen] --------------- output, output key
 *     S1[32] --------------------- output, sponsor's S1, or responsor's S2, this is optional
 *     SA[32] --------------------- output, sponsor's SA, or responsor's SB, this is optional
 * return:
 *     SM2_SUCCESS(success); other(error)
 * caution: 
 *     1. please make sure the inputs are valid
 *     2. S1 and SA are optional, if you don't need, please set S1 and SA as NULL
 *     3. in case that S1(S2) and SA(SB) exist, if S1=SB,S2=SA, then exchange success.
 */
uint32_t sm2_exchangekey(sm2_exchange_role_e role,
                        const uint8_t *dA, const uint8_t *PB,
                        const uint8_t *rA, const uint8_t *RA,
                        const uint8_t *RB,
                        const uint8_t *ZA, const uint8_t *ZB,
                        uint32_t kByteLen,
                        uint8_t *KA, uint8_t *S1, uint8_t *SA)
{
    uint32_t ret;
    uint32_t tA[SM2_WORD_LEN], xy[SM2_WORD_LEN<<1];

    ret = sm2_exchangekey_check_input(role, dA, PB, rA, RA, RB, ZA, ZB, kByteLen, KA);
    if(PKE_SUCCESS == ret)
    {
        ret = sm2_exchangekey_internal_setp_1(dA, rA, RA, tA);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm2_exchangekey_internal_setp_2(RB, PB, tA, xy);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm2_exchangekey_internal_setp_3(role, xy, RA, RB, ZA, ZB, kByteLen, KA, S1, SA);
    }
    else
    {}

    return ret;
}

#endif

