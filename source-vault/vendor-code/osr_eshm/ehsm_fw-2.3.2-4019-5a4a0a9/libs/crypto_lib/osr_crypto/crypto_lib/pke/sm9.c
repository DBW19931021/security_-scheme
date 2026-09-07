
#include "../../crypto_include/pke_config.h"


#ifdef SUPPORT_SM9

#include "./sm9_internal.h"
#include "../../crypto_include/pke/sm9.h"
#include "../../crypto_include/hash_hmac/hash_kdf.h"
#include "../../crypto_include/trng/trng.h"
#include "../../crypto_include/crypto_common/utility.h"



/* function: Generate KGC's master public key from master private key for SM9 sign-verify system.
 * parameters:
 *     ks ------------------------- input, KGC's master private key, 32 bytes, big-endian
 *     Ppub_s --------------------- output, KGC's master public key(x||y), 128 bytes, big-endian
 * return: SM9_SUCCESS(success), other(error)
 * caution:
 */
uint32_t sm9_sign_gen_mastPubKey_from_mastPriKey(const uint8_t ks[32], uint8_t Ppub_s[128])
{
    uint32_t *k;
    uint32_t ret;

    if((NULL == ks) || (NULL == Ppub_s))
    {
        ret = SM9_BUFFER_NULL;
    }
    else
    {
        k = (uint32_t *)rPKE_A(3u,SM9_STEPS);
        u8big_to_u32little_256bits(ks, k);

        //make sure sysPriKey in [1, n-1]
        ret = uint32_integer_check(k, sm9_curve->eccp_n, SM9_BASE_WORD_LEN, SM9_ZERO_ALL, SM9_INTEGER_TOO_BIG, SM9_SUCCESS);
        if(SM9_SUCCESS == ret)
        {
            ret = sm9_fp2_pointMul_s(sm9_curve, k, sm9_G2_Px0_Px1, sm9_G2_Py0_Py1, NULL, NULL);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            k = (uint32_t *)rPKE_A(4u,SM9_STEPS);
            (void)sm9_fp2_eccp_point_u32little_2_u8big(k, &k[2u*SM9_BASE_WORD_LEN], Ppub_s);
            ret = SM9_SUCCESS;
        }
        else
        {}
    }

    return ret;
}


/* function: Generate random KGC's master key pair for SM9 sign-verify system.
 * parameters:
 *     ks ------------------------- output, KGC's master private key, 32 bytes, big-endian
 *     Ppub_s --------------------- output, KGC's master public key(x||y), 128 bytes, big-endian
 * return: SM9_SUCCESS(success), other(error)
 * caution:
 */
uint32_t sm9_sign_gen_mastKeyPair(uint8_t ks[32], uint8_t Ppub_s[128])
{
    uint32_t ret;

    if((NULL == ks) || (NULL == Ppub_s))
    {
        ret = SM9_BUFFER_NULL;
    }
    else
    {
        do {
            ret = get_rand(ks, 32u);
            if(TRNG_SUCCESS != ret)
            {
                break;
            }
            else
            {}

            ret = sm9_sign_gen_mastPubKey_from_mastPriKey(ks, Ppub_s);
        } while ((SM9_ZERO_ALL == ret) || (SM9_INTEGER_TOO_BIG == ret));
    }

    return ret;
}


/* function: Generate user's private key for SM9 sign-verify system(internal API)
 * parameters:
 *     ks ------------------------- input, KGC's master private key, 8 words, U32 little-endian
 *     h1 ------------------------- input, h1=H1(IDA||hid,N), 8 words, U32 little-endian
 *     dsA ------------------------ output, user A's private key, 64 bytes, U8 big-endian
 * return: SM9_SUCCESS(success), other(error)
 * caution:
 *     1. 
 */
FLAG_STATIC uint32_t sm9_sign_gen_userPriKey_internal(uint32_t ks[8], uint32_t h1[8], 
        uint8_t dsA[64])
{
    uint32_t ret;

    ret = pke_load_modulus_and_pre_monts_256bits(sm9_curve->eccp_n, sm9_curve->eccp_n_h);
    if(PKE_SUCCESS == ret)
    {
        ret = pke_mod_add_sub_mul_256bits_internal(h1, ks, h1, MICROCODE_MODADD);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        if(0u != uint32_BigNum_Check_Zero(h1, 8u))
        {
            ret = SM9_ZERO_ALL;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = pke_modinv_256bits(h1, h1);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = pke_mod_add_sub_mul_256bits_internal(h1, ks, h1, MICROCODE_MODMUL);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = eccp_pointMul(sm9_curve, h1, sm9_curve->eccp_Gx, sm9_curve->eccp_Gy, ks, h1);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)sm9_fp_eccp_point_u32little_2_u8big(ks, h1, dsA);
        ret = SM9_SUCCESS;
    }
    else
    {}

    return ret;
}


/* function: Generate user's private key for SM9 sign-verify system.
 * parameters:
 *     IDA ------------------------ input, identify of user A
 *     IDA_bytes ------------------ input, bytes length of the IDA
 *     hid ------------------------ input, user private key generation function identity, published
 *                                         by KGC, default value is 0x01, one byte.
 *     ks ------------------------- input, KGC's master private key, 32 bytes, big-endian
 *     dsA ------------------------ output, user A's private key, 64 bytes, big-endian
 * return: SM9_SUCCESS(success), other(error)
 * caution:
 */
uint32_t sm9_sign_gen_userPriKey(const uint8_t *IDA, uint32_t IDA_bytes, uint8_t hid, 
        const uint8_t ks[32], uint8_t dsA[64])
{
    uint32_t ret;
    uint32_t tmp_ks[SM9_BASE_WORD_LEN], h1[SM9_BASE_WORD_LEN];
    uint8_t hid_value = hid;

    if((NULL == IDA) || (NULL == ks) || (NULL == dsA))
    {
        ret = SM9_BUFFER_NULL;
    }
    else if(0u == IDA_bytes)
    {
        ret = SM9_INPUT_INVALID;
    }
    else
    {
        u8big_to_u32little_256bits(ks, tmp_ks);

        //make sure ks in [1, n-1]
        ret = uint32_integer_check(tmp_ks, sm9_curve->eccp_n, SM9_BASE_WORD_LEN, SM9_ZERO_ALL, 
                SM9_INTEGER_TOO_BIG, PKE_SUCCESS);
        if(PKE_SUCCESS == ret)
        {
            ret = sm9_h1_h2((uint8_t)1, IDA, IDA_bytes, (uint8_t *)(&hid_value), 1u, h1);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            ret = sm9_sign_gen_userPriKey_internal(tmp_ks, h1, dsA);
        }
        else
        {}
    }

    uint32_clear(tmp_ks, (sizeof(tmp_ks))>>2);
    uint32_clear(h1, (sizeof(h1))>>2);

    return ret;
}


/* function: SM9 sign with r, get el and h
 * parameters:
 *     fg ------------------------- input, the value of e(P1, Ppub_s)
 *     r -------------------------- input, random integer r in signing
 *     msg ------------------------ input, message to be signed
 *     msg_bytes ------------------ input, bytes length of message
 *     el ------------------------- output, el = (r-h) mod n
 *     h -------------------------- output, partial signature result h
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_sign_with_r(const uint32_t *fg, const uint32_t *r, 
        const uint8_t *msg, uint32_t msg_bytes, uint32_t el[8], uint8_t h[32])
{
    uint32_t h2rf_para[8u*12u];
    uint32_t ret;

    //make sure r in [1, n-1]
    ret = uint32_integer_check(r, sm9_curve->eccp_n, SM9_BASE_WORD_LEN, SM9_ZERO_ALL, 
            SM9_INTEGER_TOO_BIG, PKE_SUCCESS);
    if(PKE_SUCCESS == ret)
    {
        //get w := g^r
        ret = sm9_fp12_exp_s(fg, r, NULL);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        sm9_fp12_in_pke_ram_u32little_2_u8big((uint8_t *)h2rf_para);

        //get h := H2(msg||w, N)
        ret = sm9_h1_h2((uint8_t)2, msg, msg_bytes, (uint8_t *)h2rf_para, 32u*12u, el);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        u32little_to_u8big_256bits(el, h);

        //el = (r-h) mod n
        ret = pke_modadd_modsub_256bits(sm9_curve->eccp_n, r, el, el, MICROCODE_MODSUB);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        if(0u != uint32_BigNum_Check_Zero(el, SM9_BASE_WORD_LEN))
        {
            ret = SM9_ZERO_ALL;
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: SM9 sign(internal API)
 * parameters:
 *     msg ------------------------ input, message to be signed
 *     msg_bytes ------------------ input, bytes length of message
 *     fp12g ---------------------- input, the value of e(P1, Ppub_s), if set to null, it will be calculated within the function
 *     Ppub_s --------------------- input, KGC's master public key
 *     r -------------------------- input, random big integer r in signing, 32 bytes, big-endian,
 *                                  if you do not have this integer, please set this parameter to be NULL,
 *                                  it will be generated inside.
 *     el ------------------------- output, el = (r-h) mod n
 *     h -------------------------- output, partial signature result h
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. 
 */
FLAG_STATIC uint32_t sm9_sign_internal(const uint8_t *msg, uint32_t msg_bytes, 
        const uint8_t *fp12g, const uint8_t Ppub_s[128], const uint8_t r[32], 
        uint32_t el[8], uint8_t h[32])
{
    uint32_t ret;
    uint32_t tmp_fp12g[8*12];
    uint32_t tmp_r[8];
#if 0
    uint32_t Qx[16], Qy[16];
#else
#ifndef SUPPORT_STATIC_ANALYSIS
#define Qx  (tmp_fp12g)
#define Qy  (&tmp_fp12g[2u*SM9_BASE_WORD_LEN])
#else
uint32_t *Qx = tmp_fp12g;
uint32_t *Qy = &tmp_fp12g[2u*SM9_BASE_WORD_LEN];
#endif
#endif

    if(NULL == fp12g)
    {
        if(NULL == Ppub_s)
        {
            ret = SM9_BUFFER_NULL;
        }
        else
        {
            //check Ppub_s
            (void)sm9_fp2_eccp_point_u8big_2_u32little(Ppub_s, Qx, Qy);
            ret = sm9_fp2_check_point_internal(sm9_curve, Qx, Qy);
            if(PKE_SUCCESS != ret)
            {
                ret = SM9_NOT_ON_CURVE;
            }
            else
            {
                ret = sm9_pairing(sm9_curve->eccp_Gx, sm9_curve->eccp_Gy, Qx, Qy, tmp_fp12g);
            }
        }
    }
    else
    {
        (void)sm9_fp12_u8big_2_u32little(fp12g, (uint32_t *)tmp_fp12g);
        ret = PKE_SUCCESS;
    }

    if(PKE_SUCCESS == ret)
    {
        if(NULL == r)
        {
            do {
                ret = get_rand((uint8_t *)tmp_r, 32u);
                if(TRNG_SUCCESS != ret)
                {
                    break;
                }
                else
                {}

                ret = sm9_sign_with_r(tmp_fp12g, tmp_r, msg, msg_bytes, el, h);
            } while((SM9_ZERO_ALL == ret) || (SM9_INTEGER_TOO_BIG == ret));
        }
        else
        {
            u8big_to_u32little_256bits(r, tmp_r);
            ret = sm9_sign_with_r(tmp_fp12g, tmp_r, msg, msg_bytes, el, h);
        }
    }
    else
    {}

#ifndef SUPPORT_STATIC_ANALYSIS
#undef Qx
#undef Qy
#endif

    uint32_clear(tmp_fp12g, (sizeof(tmp_fp12g))>>2);
    uint32_clear(tmp_r, (sizeof(tmp_r))>>2);

    return ret;
}


/* function: SM9 sign
 * parameters:
 *     msg ------------------------ input, message to be signed
 *     msg_bytes ------------------ input, bytes length of message
 *     fp12g ---------------------- input, the value of e(P1, Ppub_s), if set to null, it will be calculated within the function
 *     Ppub_s --------------------- input, KGC's master public key
 *     dsA ------------------------ input, signer's private key
 *     r -------------------------- input, random big integer r in signing, 32 bytes, big-endian,
 *                                  if you do not have this integer, please set this parameter to be NULL,
 *                                  it will be generated inside.
 *     h -------------------------- output, partial signature result h
 *     S -------------------------- output, partial signature result S
 * return: SM9_SUCCESS(success), other(error)
 * caution:
 *     1. 
 */
uint32_t sm9_sign(const uint8_t *msg, uint32_t msg_bytes, const uint8_t *fp12g, 
        const uint8_t Ppub_s[128], const uint8_t dsA[64], const uint8_t r[32], 
        uint8_t h[32], uint8_t S[65])
{
    uint32_t ret;
    uint32_t Px[8], Py[8];
    uint32_t tmp_r[8];
    uint32_t el[8];

    if((NULL == msg) || (NULL == dsA) || (NULL == h) || (NULL == S))
    {
        ret = SM9_BUFFER_NULL;
    }
    else if(0u == msg_bytes)
    {
        ret = SM9_INPUT_INVALID;
    }
    else
    {
        //check dsA
        (void)sm9_fp_eccp_point_u8big_2_u32little(dsA, Px, Py);
        ret = eccp_check_point(sm9_curve, Px, Py);
        if(PKE_SUCCESS != ret)
        {
            ret = SM9_NOT_ON_CURVE;
        }
        else
        {}
    }

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_sign_internal(msg, msg_bytes, fp12g, Ppub_s, r, el, h);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = eccp_pointMul(sm9_curve, el, Px, Py, Px, Py);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        S[0] = POINT_UNCOMPRESSED;
        (void)sm9_fp_eccp_point_u32little_2_u8big(Px, Py, &S[1u]);
        ret = SM9_SUCCESS;
    }
    else
    {}

    uint32_clear(Px, (sizeof(Px))>>2);
    uint32_clear(Py, (sizeof(Py))>>2);
    uint32_clear(tmp_r, (sizeof(tmp_r))>>2);
    uint32_clear(el, (sizeof(el))>>2);

    return ret;
}


/* function: SM9 verify the signature step 1(internal API)
 * parameters:
 *     Ppub_s --------------------- input, KGC's master public key, U8 big-endian
 *     h -------------------------- input, partial signature result h, U8 big-endian
 *     S -------------------------- input, partial signature result S, U8 big-endian
 *     fp12g ---------------------- input, the value of e(P1, Ppub_s), U8 big-endian, if set to null, 
 *                                  it will be calculated within the function
 *     Ppub_s_x ------------------- output, x coordinate of Ppub_s, U32 little-endian
 *     Ppub_s_y ------------------- output, y coordinate of Ppub_s, U32 little-endian
 *     h_u32 ---------------------- output, signature h, U32 little-endian
 *     Sx_u32 --------------------- output, x coordinate of signature S, U32 little-endian
 *     Sy_u32 --------------------- output, y coordinate of signature S, U32 little-endian
 *     fp12g_u32 ------------------ output, g = e(P1, Ppub_s), U32 little-endian
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_verify_internal_step_1(const uint8_t Ppub_s[128], const uint8_t h[32], 
        const uint8_t S[65], const uint8_t *fp12g, uint32_t Ppub_s_x[8], uint32_t Ppub_s_y[8], 
        uint32_t *h_u32, uint32_t Sx_u32[8], uint32_t Sy_u32[8], uint32_t *fp12g_u32)
{
    uint32_t ret;

    //check h in [1, n-1]
    u8big_to_u32little_256bits(h, h_u32);
    ret = uint32_integer_check(h_u32, sm9_curve->eccp_n, SM9_BASE_WORD_LEN, SM9_ZERO_ALL, 
            SM9_INTEGER_TOO_BIG, PKE_SUCCESS);
    if(PKE_SUCCESS == ret)
    {
        //check S on curve
        (void)sm9_fp_eccp_point_u8big_2_u32little(&S[1u], Sx_u32, Sy_u32);
        ret = eccp_check_point(sm9_curve, Sx_u32, Sy_u32);
        if(PKE_SUCCESS != ret)
        {
            ret = SM9_NOT_ON_CURVE;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //check Ppub_s
        (void)sm9_fp2_eccp_point_u8big_2_u32little(Ppub_s, Ppub_s_x, Ppub_s_y);
        ret = sm9_fp2_check_point_internal(sm9_curve, Ppub_s_x, Ppub_s_y);
        if(PKE_SUCCESS != ret)
        {
            ret = SM9_NOT_ON_CURVE;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        if(NULL == fp12g)
        {
            ret = sm9_pairing(sm9_curve->eccp_Gx, sm9_curve->eccp_Gy, Ppub_s_x, Ppub_s_y, fp12g_u32);
        }
        else
        {
            (void)sm9_fp12_u8big_2_u32little(fp12g, (uint32_t *)fp12g_u32);
        }
    }
    else
    {}

    return ret;
}


/* function: SM9 verify the signature step 2(internal API)
 * parameters:
 *     IDA ------------------------ input, identify of user A, user A is the signer
 *     IDA_bytes ------------------ input, bytes length of the IDA
 *     hid ------------------------ input, user private key generation function identity, published
 *                                         by KGC, default value is 0x01, one byte.
 *     fp12g_u32 ------------------ input, g = e(P1, Ppub_s), U32 little-endian
 *     h_u32 ---------------------- input, signature h, U32 little-endian
 *     Ppub_s_x ------------------- input, x coordinate of Ppub_s, U32 little-endian
 *     Ppub_s_y ------------------- input, y coordinate of Ppub_s, U32 little-endian
 *     Px_u32 --------------------- output, x coordinate of ([h1]P2 + Ppub_s), U32 little-endian
 *     Py_u32 --------------------- output, y coordinate of ([h1]P2 + Ppub_s), U32 little-endian
 *     fp12t_u32 ------------------ output, t = g^h, U32 little-endian
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1.IDA represents only the signer.
 */
FLAG_STATIC uint32_t sm9_verify_internal_step_2(const uint8_t *IDA, uint32_t IDA_bytes, 
        uint8_t hid, uint32_t *fp12g_u32, const uint32_t *h_u32, const uint32_t Ppub_s_x[8], 
        const uint32_t Ppub_s_y[8], uint32_t Px_u32[8], uint32_t Py_u32[8], uint32_t *fp12t_u32)
{
    uint32_t ret;
    uint32_t *h1_u32 = fp12g_u32;
    uint8_t hid_value = hid;

    //to calc t = g^h
    ret = sm9_fp12_exp_s(fp12g_u32, h_u32, fp12t_u32);
    if(PKE_SUCCESS == ret)
    {
        //to calc h1_u32 = H1(IDA||hid, N)
        ret = sm9_h1_h2((uint8_t)1, IDA, IDA_bytes, &hid_value, 1u, h1_u32);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //P = [h1]P2
        ret = sm9_fp2_pointMul_s(sm9_curve, h1_u32, sm9_G2_Px0_Px1, sm9_G2_Py0_Py1, 
            Px_u32, Py_u32);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //P = [h1]P2 + Ppub_s
        ret = sm9_fp2_pointAdd_internal(Px_u32, Py_u32, Ppub_s_x, Ppub_s_y, Px_u32, Py_u32);
    }
    else
    {}

    return ret;
}


/* function: SM9 verify the signature step 3(internal API)
 * parameters:
 *     msg ------------------------ input, message to be verified
 *     msg_bytes ------------------ input, bytes length of message
 *     h_u32 ---------------------- input, signature h, U32 little-endian
 *     Sx_u32 --------------------- input, x coordinate of signature S, U32 little-endian
 *     Sy_u32 --------------------- input, y coordinate of signature S, U32 little-endian
 *     Px_u32 --------------------- input, x coordinate of ([h1]P2 + Ppub_s), U32 little-endian
 *     Py_u32 --------------------- input, y coordinate of ([h1]P2 + Ppub_s), U32 little-endian
 *     fp12t ---------------------- input, t = g^h, U32 little-endian
 *     fp12t_tmp ------------------ input, temporary buffer, 8*12 words
 * return: SM9_SUCCESS(success), other(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_verify_internal_step_3(const uint8_t *msg, uint32_t msg_bytes, 
        const uint32_t h_u32[8], const uint32_t Sx_u32[8], const uint32_t Sy_u32[8], 
        const uint32_t Px_u32[8], const uint32_t Py_u32[8], const uint32_t *fp12t, 
        uint32_t *fp12_tmp)
{
    uint32_t ret;
    uint32_t *h2_u32 = fp12_tmp;

    //to calc fp12_tmp = u = e(S, P)
    ret = sm9_pairing(Sx_u32, Sy_u32, Px_u32, Py_u32, fp12_tmp);
    if(PKE_SUCCESS == ret)
    {
        //to calc w = u*t
        ret = sm9_fp12_mul(fp12t, fp12_tmp, NULL);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        sm9_fp12_in_pke_ram_u32little_2_u8big((uint8_t *)fp12_tmp);

        //to calc h2 = H2(M||w, N)
        ret = sm9_h1_h2((uint8_t)2, msg, msg_bytes, (uint8_t *)fp12_tmp, 32u*12u, h2_u32);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //compare h and h2
        if(uint32_BigNumCmp(h_u32, SM9_BASE_WORD_LEN, h2_u32, SM9_BASE_WORD_LEN) != 0)
        {
            ret = SM9_VERIFY_FAILED;
        }
        else
        {
            ret = SM9_SUCCESS;
        }
    }
    else
    {}

    return ret;
}


/* function: SM9 verify the signature
 * parameters:
 *     msg ------------------------ input, message to be verified
 *     msg_bytes ------------------ input, bytes length of message
 *     IDA ------------------------ input, identify of user A, user A is the signer
 *     IDA_bytes ------------------ input, bytes length of the IDA
 *     hid ------------------------ input, user private key generation function identity, published
 *                                         by KGC, default value is 0x01, one byte.
 *     fp12g ---------------------- input, the value of e(P1, Ppub_s), U8 big-endian, if set to null, 
 *                                  it will be calculated within the function
 *     Ppub_s --------------------- input, KGC's master public key
 *     h -------------------------- input, partial signature result h
 *     S -------------------------- input, partial signature result S
 * return: SM9_SUCCESS(success), other(error)
 * caution:
 *     1.IDA represents only the signer.
 */
uint32_t sm9_verify(const uint8_t *msg, uint32_t msg_bytes, const uint8_t *IDA, uint32_t IDA_bytes, 
        uint8_t hid, const uint8_t *fp12g, const uint8_t Ppub_s[128], const uint8_t h[32], 
        const uint8_t S[65])
{
    uint32_t ret;
    uint32_t tmp_fp12g[8*12], fp12t[8*12];
#if 0
    uint32_t TP2x[16], TP2y[16];
#else
#ifndef SUPPORT_STATIC_ANALYSIS
#define TP2x       ((uint32_t *)tmp_fp12g)
#define TP2y       (&(tmp_fp12g[2u*SM9_BASE_WORD_LEN]))
#else
    uint32_t *TP2x     = tmp_fp12g;
    uint32_t *TP2y     = &(tmp_fp12g[2u*SM9_BASE_WORD_LEN]);
#endif
#endif
    uint32_t SigH[8];
    uint32_t SigSx[8], SigSy[8];
    uint32_t Ppub_s_x[16], Ppub_s_y[16];

    if((NULL == msg) || (NULL == IDA) || (NULL == Ppub_s) || (NULL == h) || (NULL == S))
    {
        ret = SM9_BUFFER_NULL;
    }
    else if((0u == IDA_bytes) || (POINT_UNCOMPRESSED != S[0]))
    {
        ret = SM9_INPUT_INVALID;
    }
    else
    {
        ret = sm9_verify_internal_step_1(Ppub_s, h, S, fp12g, Ppub_s_x, Ppub_s_y, SigH, 
                SigSx, SigSy, tmp_fp12g);
    }

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_verify_internal_step_2(IDA, IDA_bytes, hid, tmp_fp12g, SigH, Ppub_s_x, Ppub_s_y, 
                TP2x, TP2y, fp12t);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_verify_internal_step_3(msg, msg_bytes, SigH, SigSx, SigSy, TP2x, TP2y, 
                fp12t, tmp_fp12g);
    }
    else
    {}

#ifndef SUPPORT_STATIC_ANALYSIS
#undef TP2x
#undef TP2y
#endif

    uint32_clear(tmp_fp12g, (sizeof(tmp_fp12g))>>2);
    uint32_clear(fp12t, (sizeof(fp12t))>>2);
    uint32_clear(SigH, (sizeof(SigH))>>2);
    uint32_clear(SigSx, (sizeof(SigSx))>>2);
    uint32_clear(SigSy, (sizeof(SigSy))>>2);
    uint32_clear(Ppub_s_x, (sizeof(Ppub_s_x))>>2);
    uint32_clear(Ppub_s_y, (sizeof(Ppub_s_y))>>2);

    return ret;
}


/* function: Generate KGC's master public key from master private key for SM9 encryption system.
 * parameters:
 *     ke ------------------------- input, KGC's master private key, 32 bytes, big-endian
 *     Ppub_e --------------------- output, KGC's master public key(x||y), 64 bytes, big-endian
 * return:
 *     SM9_SUCCESS(success); other(error)
 * caution:
 */
uint32_t sm9_enc_gen_mastPubKey_from_mastPriKey(const uint8_t ke[32], uint8_t Ppub_e[64])
{
    uint32_t tmp_ke[SM9_BASE_WORD_LEN];
    uint32_t t1[SM9_BASE_WORD_LEN],t2[SM9_BASE_WORD_LEN];
    uint32_t ret;

    if((NULL == ke) || (NULL == Ppub_e))
    {
        ret = SM9_BUFFER_NULL;
    }
    else
    {
        u8big_to_u32little_256bits(ke, tmp_ke);

        //make sure priKey in [1, n-1]
        ret = uint32_integer_check(tmp_ke, sm9_curve->eccp_n, SM9_BASE_WORD_LEN, SM9_ZERO_ALL, SM9_INTEGER_TOO_BIG, SM9_SUCCESS);
        if(SM9_SUCCESS == ret)
        {
            //Ppub_e = [ke]P1
            ret = eccp_pointMul(sm9_curve, tmp_ke, sm9_curve->eccp_Gx, sm9_curve->eccp_Gy, t1, t2);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            (void)sm9_fp_eccp_point_u32little_2_u8big(t1, t2, Ppub_e);
            ret = SM9_SUCCESS;
        }
        else
        {}
    }

    uint32_clear_8_words(tmp_ke);
    uint32_clear_8_words(t1);
    uint32_clear_8_words(t2);

    return ret;
}


/* function: Generate random KGC's master key pair for SM9 encryption system.
 * parameters:
 *     ke ------------------------- output, KGC's master private key, 32 bytes, big-endian
 *     Ppub_e --------------------- output, KGC's master public key(x||y), 64 bytes, big-endian
 * return:
 *     SM9_SUCCESS(success); other(error)
 * caution:
 */
uint32_t sm9_enc_gen_mastKeyPair(uint8_t ke[32], uint8_t Ppub_e[64])
{
    uint32_t ret;

    if((NULL == ke) || (NULL == Ppub_e))
    {
        ret = SM9_BUFFER_NULL;
    }
    else
    {
        do {
            ret = get_rand(ke, 32u);
            if(TRNG_SUCCESS != ret)
            {
                break;
            }
            else
            {}

            ret = sm9_enc_gen_mastPubKey_from_mastPriKey(ke, Ppub_e);
        } while ((SM9_ZERO_ALL == ret) || (SM9_INTEGER_TOO_BIG == ret));
    }

    return ret;
}


/* function: Generate user's private key for SM9 encryption system(internal API)
 * parameters:
 *     ke ------------------------- input, KGC's master private key, 8 words, U32 big-endian
 *     t1 ------------------------- input, t1=H1(IDB||hid,N)+ke, 8 words, U32 big-endian
 *     deB ------------------------ output, user B's private key, 128 bytes, U8 big-endian
 * return: SM9_SUCCESS(success), other(error)
 * caution:
 *     1. 
 */
FLAG_STATIC uint32_t sm9_enc_gen_userPriKey_internal(const uint32_t ke[8], 
        const uint32_t t1[8], uint8_t deB[128])
{
    uint32_t ret;
    uint32_t t2[SM9_BASE_WORD_LEN];
    uint32_t Qx[2u*SM9_BASE_WORD_LEN];
    uint32_t Qy[2u*SM9_BASE_WORD_LEN];

    //t2 = t1^(-1) mod n
    ret = pke_modinv_256bits(t1, t2);
    if(PKE_SUCCESS == ret)
    {
        //t2 = ke*(t1^(-1)) mod n
#if 0
        ret = pke_modmul_internal(t2, tmp_ke, t2, SM9_BASE_WORD_LEN);
#else
        ret = pke_mod_add_sub_mul_256bits_internal(t2, ke, t2, MICROCODE_MODMUL);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //deB = [t2]P2
        ret = sm9_fp2_pointMul_s(sm9_curve, t2, sm9_G2_Px0_Px1, sm9_G2_Py0_Py1, Qx, Qy);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)sm9_fp2_eccp_point_u32little_2_u8big(Qx, Qy, deB);

        ret = SM9_SUCCESS;
    }
    else
    {}

    uint32_clear_8_words(t2);
    uint32_clear(Qx, (sizeof(Qx))>>2);
    uint32_clear(Qy, (sizeof(Qy))>>2);

    return ret;
}


/* function: Generate user's private key for SM9 encryption system.
 * parameters:
 *     IDB ------------------------ input, identify of user B
 *     IDB_bytes ------------------ input, bytes length of the IDB
 *     hid ------------------------ input, user private generation function identity, published by KGC,
 *                                         default value is 0x03, one byte.
 *     ke ------------------------- input, KGC's master private key, 32 bytes, big-endian
 *     deB ------------------------ output, user B's private key, 128 bytes, big-endian
 * return: SM9_SUCCESS(success), other(error)
 * caution:
 */
uint32_t sm9_enc_gen_userPriKey(const uint8_t *IDB, uint32_t IDB_bytes, uint8_t hid, 
        const uint8_t ke[32], uint8_t deB[128])
{
    uint32_t ret;
    uint32_t ke_u32[SM9_BASE_WORD_LEN];
    uint32_t t1[SM9_BASE_WORD_LEN];
    uint8_t hid_value = hid;

    if((NULL == IDB) || (NULL == ke) || (NULL == deB))
    {
        ret = SM9_BUFFER_NULL;
    }
    else if(0u == IDB_bytes)
    {
        ret = SM9_INPUT_INVALID;
    }
    else
    {
        //make sure sysPriKey in [1, n-1]
        u8big_to_u32little_256bits(ke, ke_u32);
        ret = uint32_integer_check(ke_u32, sm9_curve->eccp_n, SM9_BASE_WORD_LEN, SM9_ZERO_ALL, 
                SM9_INTEGER_TOO_BIG, PKE_SUCCESS);
        if(PKE_SUCCESS == ret)
        {
            //t1 := H1(IDB||hid, N)
            ret = sm9_h1_h2((uint8_t)1, IDB, IDB_bytes, &hid_value, 1u, t1);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            ret = pke_load_modulus_and_pre_monts_256bits(sm9_curve->eccp_n, sm9_curve->eccp_n_h);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            //t1 = H1(IDB||hid, N) + ke mod n
            ret = pke_mod_add_sub_mul_256bits_internal(t1, ke_u32, t1, MICROCODE_MODADD);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            //t1 can not be 0
            if(0u != uint32_BigNum_Check_Zero(t1, 8u))
            {
                ret = SM9_ZERO_ALL;
            }
            else
            {}
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            ret = sm9_enc_gen_userPriKey_internal(ke_u32, t1, deB);
        }
        else
        {}
    }

    uint32_clear_8_words(t1);
    uint32_clear_8_words(ke_u32);

    return ret;
}


/* function: SM9 SM9 key encapsulation with r, get cipher C and output key
 * parameters:
 *     IDB ------------------------ input, identify of user B, user B is receiver of the cipher C
 *     IDB_bytes ------------------ input, bytes length of the IDB
 *     QBx ------------------------ input, x coordinate of QB = [H1(IDB||hid, N)]P1+Ppub_e
 *     QBy ------------------------ input, y coordinate of QB = [H1(IDB||hid, N)]P1+Ppub_e
 *     fg ------------------------- input, the value of e(P1, Ppub_e)
 *     r -------------------------- input, random integer r 
 *     C -------------------------- output, encapsulated cipher of key, 64 bytes
 *     key_bytes ------------------ input, bytes length of the output key
 *     key ------------------------ output, plaintext output key
 * return: SM9_SUCCESS(success), other(error)
 * caution:
 *     1. fg, QBx, QBy can not be modified
 */
FLAG_STATIC uint32_t sm9_wrap_key_with_r(const uint8_t *IDB, uint32_t IDB_bytes, 
        const uint32_t *QBx, const uint32_t *QBy, const uint32_t *fg, const uint32_t *r, 
        uint8_t C[64], uint32_t key_bytes, uint8_t *key)
{
    uint32_t ret;
    uint32_t counter_buf[1] = {0u};
    const uint8_t *counter = (uint8_t *)counter_buf;
    uint32_t Cx[8], Cy[8];
    uint32_t h2rf_para[8u*12u];

    hash_node_st digest_node[4];

    ((uint8_t *)counter_buf)[3] = (uint8_t)1u;
    digest_node[0].msg_addr  = C;
    digest_node[0].msg_bytes = 64u;
    digest_node[1].msg_addr  = (uint8_t *)h2rf_para;
    digest_node[1].msg_bytes = 32u*12u;
    digest_node[2].msg_addr  = IDB;
    digest_node[2].msg_bytes = IDB_bytes;
    digest_node[3].msg_addr  = counter;
    digest_node[3].msg_bytes = 4u;

    //make sure r in [1, n-1]
    ret = uint32_integer_check(r, sm9_curve->eccp_n, SM9_BASE_WORD_LEN, SM9_ZERO_ALL, SM9_INTEGER_TOO_BIG, 
            PKE_SUCCESS);
    if(PKE_SUCCESS == ret)
    {
        //C = [r]QB
        ret = eccp_pointMul(sm9_curve, r, QBx, QBy, Cx, Cy);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)sm9_fp_eccp_point_u32little_2_u8big(Cx, Cy, C);

        //w = g^r
        ret = sm9_fp12_exp_s(fg, r, NULL);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        sm9_fp12_in_pke_ram_u32little_2_u8big((uint8_t *)h2rf_para);

        //key := KDF(C||w||IDB, key_bytes)
        ret = ansi_x9_63_kdf_node(HASH_SM3, digest_node, 4u, (uint8_t *)counter_buf, key, key_bytes, NULL, 0u);
        if(HASH_SUCCESS == ret)
        {
            if(0u != uint8_BigNum_Check_Zero(key, key_bytes))
            {
                ret = SM9_ZERO_ALL;
            }
            else
            {
                ret = SM9_SUCCESS;
            }
        }
        else
        {}
    }
    else
    {}

    if(SM9_SUCCESS != ret)
    {
        memset_(C, 0, 64u);
        memset_(key, 0, key_bytes);
    }
    else
    {}

    uint32_clear(counter_buf, (sizeof(counter_buf))>>2);
    uint32_clear((uint32_t *)Cx, (sizeof(Cx))>>2);
    uint32_clear((uint32_t *)Cy, (sizeof(Cy))>>2);
    uint32_clear((uint32_t *)h2rf_para, (sizeof(h2rf_para))>>2);

    return ret;
}


/* function: SM9 key encapsulation, generates key and its encapsulated cipher C(internal API)
 * parameters:
 *     IDB ------------------------ input, identify of user B, user B is receiver of the cipher C
 *     IDB_bytes ------------------ input, bytes length of the IDB
 *     fp12g ---------------------- input, the value of e(Ppub_e, P2), if set to null, it will be calculated within the function
 *     fp12 ----------------------- input, temporary buffer, 12*32/4 words, and KGC's system encryption master public key is 
 *                                  stored in this buffer
 *     r -------------------------- input, random big integer r in wrapping, 32 bytes, big-endian,
 *                                  if you do not have this integer, please set this parameter to be NULL,
 *                                  it will be generated inside.
 *     QBx ------------------------ input, x coordiante of QB
 *     QBy ------------------------ input, y coordiante of QB
 *     C -------------------------- output, encapsulated cipher of key, 64 bytes
 *     k_bytes -------------------- input, bytes length of the output key
 *     k -------------------------- output, plaintext output key
 * return: SM9_SUCCESS(success), other(error)
 * caution:
 *     1. 
 */
FLAG_STATIC uint32_t sm9_wrap_key_internal(const uint8_t *IDB, uint32_t IDB_bytes, 
        const uint8_t *fp12g, uint32_t *fp12, const uint8_t r[32], const uint32_t QBx[8], 
        const uint32_t QBy[8], uint8_t C[64], uint32_t k_bytes, uint8_t *k)
{
    uint32_t ret;
    uint32_t r_u32[8];

    //get fp12 := e(Ppub_e, P2)
    if(NULL == fp12g)
    {
        ret = sm9_pairing(fp12, &fp12[SM9_BASE_WORD_LEN], sm9_G2_Px0_Px1, sm9_G2_Py0_Py1, fp12);

    }
    else
    {
        (void)sm9_fp12_u8big_2_u32little(fp12g, fp12);
        ret = PKE_SUCCESS;
    }

    if(PKE_SUCCESS == ret)
    {
        if(NULL == r)
        {
            do {
                ret = get_rand((uint8_t *)r_u32, SM9_BASE_BYTE_LEN);
                if(TRNG_SUCCESS != ret)
                {
                    break;
                }
                else
                {}

                ret = sm9_wrap_key_with_r(IDB, IDB_bytes, QBx, QBy, fp12, r_u32, C, k_bytes, k);
            } while((SM9_ZERO_ALL == ret) || (SM9_INTEGER_TOO_BIG == ret));
        }
        else
        {
            u8big_to_u32little_256bits(r, r_u32);
            ret = sm9_wrap_key_with_r(IDB, IDB_bytes, QBx, QBy, fp12, r_u32, C, k_bytes, k);
        }
    }
    else
    {}

    uint32_clear(r_u32, (sizeof(r_u32))>>2);

    return ret;
}


/* function: SM9 key encapsulation, generates key and its encapsulated cipher C
 * parameters:
 *     IDB ------------------------ input, identify of user B, user B is receiver of the cipher C
 *     IDB_bytes ------------------ input, bytes length of the IDB
 *     hid ------------------------ input, user private generation function identity, published by KGC,
 *                                         default value is 0x03, one byte.
 *     fp12g ---------------------- input, the value of e(Ppub_e, P2), if set to null, it will be calculated within the function
 *     Ppub_e --------------------- input, KGC's system encryption master public key
 *     r -------------------------- input, random big integer r in wrapping, 32 bytes, big-endian,
 *                                  if you do not have this integer, please set this parameter to be NULL,
 *                                  it will be generated inside.
 *     C -------------------------- output, encapsulated cipher of key, 64 bytes
 *     k_bytes -------------------- input, bytes length of the output key
 *     k -------------------------- output, plaintext output key
 * return: SM9_SUCCESS(success), other(error)
 * caution:
 *     1. 
 */
uint32_t sm9_wrap_key(const uint8_t *IDB, uint32_t IDB_bytes, uint8_t hid, const uint8_t *fp12g, 
        const uint8_t Ppub_e[64], const uint8_t r[32], uint8_t C[64], uint32_t k_bytes, uint8_t *k)
{
    uint32_t ret;
    uint32_t QBx[8], QBy[8];
    uint32_t fp12[8*12];
    uint8_t hid_value = hid;

    if((NULL == IDB) || (NULL == k) || (NULL == C) || (NULL == Ppub_e))
    {
        ret = SM9_BUFFER_NULL;
    }
    else if(0u == IDB_bytes)
    {
        ret = SM9_INPUT_INVALID;
    }
    else
    {
        //check Ppub_e
        (void)sm9_fp_eccp_point_u8big_2_u32little(Ppub_e, fp12, &fp12[SM9_BASE_WORD_LEN]);
        ret = eccp_check_point(sm9_curve, fp12, &fp12[SM9_BASE_WORD_LEN]);
        if(PKE_SUCCESS != ret)
        {
            ret = SM9_NOT_ON_CURVE;
        }
        else
        {}
    }

    if(PKE_SUCCESS == ret)
    {
        //tmp := H1(IDB||hid, N)
        ret = sm9_h1_h2((uint8_t)1, IDB, IDB_bytes, &hid_value, 1u, &fp12[SM9_BASE_WORD_LEN<<1]);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //get [H1(IDB||hid, N)]P1
        ret = eccp_pointMul(sm9_curve, &fp12[SM9_BASE_WORD_LEN<<1], sm9_curve->eccp_Gx, sm9_curve->eccp_Gy, QBx, QBy);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //QB = [H1(IDB||hid, N)]P1 + Ppub_e
        ret = eccp_pointAdd_safe_internal(sm9_curve, QBx, QBy, fp12, &fp12[SM9_BASE_WORD_LEN], QBx, QBy);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_wrap_key_internal(IDB, IDB_bytes, fp12g, fp12, r, QBx, QBy, C, k_bytes, k);
    }
    else
    {}

    uint32_clear(QBx, (sizeof(QBx))>>2);
    uint32_clear(QBy, (sizeof(QBy))>>2);
    uint32_clear(fp12, (sizeof(fp12))>>2);

    return ret;
}


/* function: SM9 key decapsulation, generates key from encapsulated cipher C
 * parameters:
 *     IDB ------------------------ input, identity of user B
 *     IDB_bytes ------------------ input, bytes length of the IDB
 *     deB ------------------------ input, private key of user B
 *     C -------------------------- output, encapsulated cipher of key, 64 bytes
 *     k_bytes -------------------- input, bytes length of the output key
 *     k -------------------------- output, plaintext output key
 * return: SM9_SUCCESS(success), other(error)
 * caution:
 */
uint32_t sm9_unwrap_key(const uint8_t *IDB, uint32_t IDB_bytes, const uint8_t deB[128], 
        const uint8_t C[64], uint32_t k_bytes, uint8_t *k)
{
    uint32_t counter_buf[4] = {0u};
    const uint8_t *counter = (uint8_t *)counter_buf;
    uint32_t h2rf_para[8u*12u];
#if 0
    uint32_t Qx[16], Qy[16], Px[8], Py[8];
#else
#ifndef SUPPORT_STATIC_ANALYSIS
#define  Qx  (h2rf_para)
#define  Qy  (&(h2rf_para[2u*SM9_BASE_WORD_LEN]))
#define  Px  (&(h2rf_para[4u*SM9_BASE_WORD_LEN]))
#define  Py  (&(h2rf_para[5u*SM9_BASE_WORD_LEN]))
#else
uint32_t *Qx = h2rf_para;
uint32_t *Qy = &h2rf_para[2u*SM9_BASE_WORD_LEN];
uint32_t *Px = &h2rf_para[4u*SM9_BASE_WORD_LEN];;
uint32_t *Py = &h2rf_para[5u*SM9_BASE_WORD_LEN];
#endif
#endif
    uint32_t ret;
    hash_node_st digest_node[4];

    ((uint8_t *)counter_buf)[3] = (uint8_t)1u;
    digest_node[0].msg_addr  = C;
    digest_node[0].msg_bytes = 64u;
    digest_node[1].msg_addr  = (uint8_t *)h2rf_para;
    digest_node[1].msg_bytes = 32u*12u;
    digest_node[2].msg_addr  = IDB;
    digest_node[2].msg_bytes = IDB_bytes;
    digest_node[3].msg_addr  = counter;
    digest_node[3].msg_bytes = 4u;

    if((NULL == IDB) || (NULL == deB) || (NULL == k) || (NULL == C))
    {
        ret = SM9_BUFFER_NULL;
    }
    else if(0u == IDB_bytes)
    {
        ret = SM9_INPUT_INVALID;
    }
    else
    {
        //check C
        (void)sm9_fp_eccp_point_u8big_2_u32little(C, Px, Py);
        ret = eccp_check_point(sm9_curve, Px, Py);
        if(PKE_SUCCESS != ret)
        {
            ret = SM9_NOT_ON_CURVE;
        }
        else
        {}
    }

    if(PKE_SUCCESS == ret)
    {
        //check deB
        (void)sm9_fp2_eccp_point_u8big_2_u32little(deB, Qx, Qy);
        ret = sm9_fp2_check_point_internal(sm9_curve, Qx, Qy);
        if(PKE_SUCCESS != ret)
        {
            ret = SM9_NOT_ON_CURVE;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //e(C,deB)
        ret = sm9_pairing(Px, Py, Qx, Qy, NULL);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        sm9_fp12_in_pke_ram_u32little_2_u8big((uint8_t *)h2rf_para);

        //key := KDF(C||w||IDB, key_bytes)
        ret = ansi_x9_63_kdf_node(HASH_SM3, digest_node, 4u, (uint8_t *)counter_buf, k, k_bytes, NULL, 0u);
        if(HASH_SUCCESS == ret)
        {
            if(0u != uint8_BigNum_Check_Zero(k, k_bytes))
            {
                ret = SM9_ZERO_ALL;
            }
            else
            {
                ret = SM9_SUCCESS;
            }
        }
        else
        {}
    }
    else
    {}

#ifndef SUPPORT_STATIC_ANALYSIS
#undef  Qx 
#undef  Qy
#undef  Px
#undef  Py
#endif

    uint32_clear(counter_buf, (sizeof(counter_buf))>>2);
    uint32_clear((uint32_t *)h2rf_para, (sizeof(h2rf_para))>>2);

    return ret;
}


/* function: SM9 encrypt with r
 * parameters:
 *     IDB ------------------------ input, identity of user B, user B is the cipher receiver 
 *     IDB_bytes ------------------ input, bytes length of IDB
 *     r -------------------------- input, random big integer r in encrypting
 *     QBx ------------------------ input, x coordinate of QB, QB = [H1(IDB||hid, N)]P1+Ppub_e
 *     QBy ------------------------ input, y coordinate of QB, QB = [H1(IDB||hid, N)]P1+Ppub_e
 *     fp12 ----------------------- input, the value of e(Ppub_e, P2)
 *     C1 ------------------------- output, C1 part of the cipher, 64 bytes
 *     K1 ------------------------- output, key K1 in internal encrypting
 *     K1_bytes ------------------- input, bytes length of the key K1 in internal encrypting
 *     K2 ------------------------- output, key K2 in internal MAC function
 *     K2_bytes ------------------- input, bytes length of the key K2 in MAC function
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. IDB only represents the decrypter.
 *     2. K2_bytes should be less than SM9_MAX_ENC_K2_BYTE_LEN
 */
FLAG_STATIC uint32_t sm9_enc_with_r(const uint8_t *IDB, uint32_t IDB_bytes, const uint32_t *r, 
        const uint32_t *QBx, const uint32_t *QBy, const uint32_t fp12[8*12], uint8_t *C1, 
        uint8_t *K1, uint32_t K1_bytes, uint8_t *K2, uint32_t K2_bytes)
{
    uint32_t ret;
    uint32_t counter_buf[1] = {0u};
    const uint8_t *counter = (uint8_t *)counter_buf;
    uint32_t w[8u*12u];
    hash_node_st digest_node[4];

    //make sure r in [1, n-1]
    ret = uint32_integer_check(r, sm9_curve->eccp_n, SM9_BASE_WORD_LEN, SM9_ZERO_ALL, SM9_INTEGER_TOO_BIG, 
            PKE_SUCCESS);
    if(PKE_SUCCESS == ret)
    {
        //get C1 = [r]QB
        ret = eccp_pointMul(sm9_curve, r, QBx, QBy, w, &w[SM9_BASE_WORD_LEN]);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)sm9_fp_eccp_point_u32little_2_u8big(w, &w[SM9_BASE_WORD_LEN], C1);

        //w = g^r
        ret = sm9_fp12_exp_s(fp12, r, NULL);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        sm9_fp12_in_pke_ram_u32little_2_u8big((uint8_t *)w);

        //get k1||K2 = KDF(C1||w||IDB, K1_bytes+K2_bytes)
        ((uint8_t *)counter_buf)[3] = (uint8_t)1u;
        digest_node[0].msg_addr  = C1;
        digest_node[0].msg_bytes = 64u;
        digest_node[1].msg_addr  = (uint8_t *)w;
        digest_node[1].msg_bytes = 32u*12u;
        digest_node[2].msg_addr  = IDB;
        digest_node[2].msg_bytes = IDB_bytes;
        digest_node[3].msg_addr  = counter;
        digest_node[3].msg_bytes = 4u;
        ret = ansi_x9_63_kdf_node(HASH_SM3, digest_node, 4u, (uint8_t *)counter_buf, K1, K1_bytes, K2, K2_bytes);
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
        //K1 can not be zero
        if(0u != uint8_BigNum_Check_Zero(K1, K1_bytes))
        {
            ret = SM9_ZERO_ALL;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS != ret)
    {
        memset_(C1, 0, 64u);
        memset_(K1, 0, K1_bytes);
        memset_(K2, 0, K2_bytes);
    }
    else
    {}

    uint32_clear(counter_buf, (sizeof(counter_buf))>>2);
    uint32_clear((uint32_t *)w, (sizeof(w))>>2);

    return ret;
}


/* function: SM9 encrypt check input(internal API)
 * parameters:
 *     IDB ------------------------ input, pointer to identity of user B, user B is the cipher receiver 
 *     IDB_bytes ------------------ input, bytes length of IDB
 *     M -------------------------- input, pointer to message to be encrypted
 *     M_bytes -------------------- input, bytes length of message to be encrypted
 *     Ppub_e --------------------- input, pointer to KGC's system encryption master public key
 *     enc_type ------------------- input, type of encryption (SM9_ENC_KDF_STREAM_CIPHER or SM9_ENC_KDF_BLOCK_CIPHER)
 *     padding_type --------------- input, type of padding(SKE_NO_PADDING or SKE_PKCS_5_7_PADDING)
 *     K2_bytes ------------------- input, bytes length of the key K2 in MAC function
 *     C -------------------------- input, pointer to the cipher
 *     C_bytes -------------------- input, pointer to bytes length of the cipher
 * return: SM9_SUCCESS(success), other(error)
 * caution:
 *     1. IDB only represents the decrypter.
 *     2. K2_bytes should be less than SM9_MAX_ENC_K2_BYTE_LEN
 */
FLAG_STATIC uint32_t sm9_enc_check_input(const uint8_t *IDB, uint32_t IDB_bytes, 
        const uint8_t *M, uint32_t M_bytes, const uint8_t Ppub_e[64], 
        sm9_enc_type_e enc_type, sm9_enc_padding_e padding_type, uint32_t K2_bytes, 
        const uint8_t *C, const uint32_t *C_bytes)
{
    uint32_t ret;

    if((NULL == IDB) || (NULL == M) || (NULL == C) || (NULL == Ppub_e) || (NULL == C_bytes))
    {
        ret = SM9_BUFFER_NULL;
    }
    else if(M == C)
    {
        ret = SM9_IN_OUT_SAME_BUFFER;
    }
    else if((0u == M_bytes) || (M_bytes >= SM9_MAX_MSG_BYTE_LEN) || (0u == IDB_bytes) || \
            (K2_bytes > SM9_MAX_ENC_K2_BYTE_LEN) || (enc_type > SM9_ENC_KDF_BLOCK_CIPHER))
    {
        ret = SM9_INPUT_INVALID;
    }
    else
    {
        ret = PKE_SUCCESS;
    }

    if(PKE_SUCCESS == ret)
    {
        if(SM9_ENC_KDF_BLOCK_CIPHER == enc_type)
        {
            if(padding_type > SKE_PKCS_5_7_PADDING)
            {
                ret = SM9_INPUT_INVALID;
            }
            else if((0u != (M_bytes & 0x0Fu)) && (SKE_NO_PADDING == padding_type))
            {
                ret = SM9_INPUT_INVALID;
            }
            else
            {
                //nothing to do, just for static analysis.
            }
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: SM9 encrypt step 1(internal API)
 * parameters:
 *     IDB ------------------------ input, identity of user B, user B is the cipher receiver 
 *     IDB_bytes ------------------ input, bytes length of IDB
 *     hid ------------------------ input, user private generation function identity, published by KGC,
 *                                         default value is 0x03, one byte.
 *     M_bytes -------------------- input, bytes length of message to be encrypted
 *     fp12g ---------------------- input, the value of e(Ppub_e, P2), if set to null, it will be calculated within the function
 *     Ppub_e --------------------- input,  KGC's system encryption master public key
 *     enc_type ------------------- input, type of encryption (SM9_ENC_KDF_STREAM_CIPHER or SM9_ENC_KDF_BLOCK_CIPHER)
 *     fp12g_u32 ------------------ output, g = e(Ppub_e, P2), U32 little-endian
 *     QBx ------------------------ output, x coordinate of QB=[H1(IDB||hid, N)]P1+Ppub_e
 *     QBy ------------------------ output, y coordinate of QB=[H1(IDB||hid, N)]P1+Ppub_e
 *     K1_bytes ------------------- output, bytes length of K1
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. IDB only represents the decrypter.
 */
FLAG_STATIC uint32_t sm9_enc_internal_step_1(const uint8_t *IDB, uint32_t IDB_bytes, uint8_t hid, 
        uint32_t M_bytes, const uint8_t *fp12g, const uint8_t Ppub_e[64], sm9_enc_type_e enc_type, 
        uint32_t *fp12g_u32, uint32_t *QBx, uint32_t *QBy, uint32_t *K1_bytes)
{
    uint32_t ret;

#if 0
    uint32_t Px[8], Py[8];
#else
#ifndef SUPPORT_STATIC_ANALYSIS
#define Px   (fp12g_u32)
#define Py   (&fp12g_u32[SM9_BASE_WORD_LEN])
#else
uint32_t *Px = fp12g_u32;
uint32_t *Py = &fp12g_u32[SM9_BASE_WORD_LEN];
#endif
#endif

    uint8_t hid_value = hid;

    //check Ppub_e
    (void)sm9_fp_eccp_point_u8big_2_u32little(Ppub_e, Px, Py);
    ret = eccp_check_point(sm9_curve, Px, Py);
    if(PKE_SUCCESS != ret)
    {
        ret = SM9_NOT_ON_CURVE;
    }
    else
    {
        //get H1(IDB||hid, N) stroed in QBx
        ret = sm9_h1_h2((uint8_t)1, IDB, IDB_bytes, &hid_value, 1u, QBx);
    }

    if(PKE_SUCCESS == ret)
    {
        //get [H1(IDB||hid, N)]P1
        ret = eccp_pointMul(sm9_curve, QBx, sm9_curve->eccp_Gx, sm9_curve->eccp_Gy, QBx, QBy);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //get QB = [H1(IDB||hid, N)]P1+Ppub_e
        ret = eccp_pointAdd_safe_internal(sm9_curve, QBx, QBy, Px, Py, QBx, QBy);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //get g := e(Ppub_e, P2)
        if(NULL == fp12g)
        {
            ret = sm9_pairing(Px, Py, sm9_G2_Px0_Px1, sm9_G2_Py0_Py1, fp12g_u32);
        }
        else
        {
            (void)sm9_fp12_u8big_2_u32little(fp12g, (uint32_t *)fp12g_u32);
        }
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //get K1 length
        if(SM9_ENC_KDF_STREAM_CIPHER == enc_type)  //KDF
        {
            *K1_bytes = M_bytes;
        }
        else if(SM9_ENC_KDF_BLOCK_CIPHER == enc_type) //SM4
        {
            *K1_bytes = 16u;
        }
        else
        {
            //nothing to do, just for static analysis.
        }
    }
    else
    {}

#ifndef SUPPORT_STATIC_ANALYSIS
#undef  Px
#undef  Py
#endif

    return ret;
}


/* function: SM9 encrypt step 2(internal API)
 * parameters:
 *     IDB ------------------------ input, identity of user B, user B is the cipher receiver 
 *     IDB_bytes ------------------ input, bytes length of IDB
 *     M -------------------------- input, message to be encrypted
 *     M_bytes -------------------- input, bytes length of message to be encrypted
 *     r -------------------------- input, random big integer r in wrapping, 32 bytes, big-endian,
 *                                  if you do not have this integer, please set this parameter to be NULL,
 *                                  it will be generated inside.
 *     enc_type ------------------- input, type of encryption (SM9_ENC_KDF_STREAM_CIPHER or SM9_ENC_KDF_BLOCK_CIPHER)
 *     padding_type --------------- input, type of padding(SKE_NO_PADDING or SKE_PKCS_5_7_PADDING)
 *     K1_bytes ------------------- input, bytes length of K1
 *     K2_bytes ------------------- input, bytes length of the key K2 in MAC function
 *     fp12g_u32 ------------------ input, g = e(Ppub_e, P2), U32 little-endian
 *     QBx ------------------------ input, x coordinate of QB=[H1(IDB||hid, N)]P1+Ppub_e
 *     QBy ------------------------ input, y coordinate of QB=[H1(IDB||hid, N)]P1+Ppub_e
 *     C -------------------------- output, the cipher
 *     C_bytes -------------------- output, bytes length of the cipher
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. IDB only represents the decrypter.
 *     2. K2_bytes should be less than SM9_MAX_ENC_K2_BYTE_LEN
 */
FLAG_STATIC uint32_t sm9_enc_internal_step_2(const uint8_t *IDB, uint32_t IDB_bytes, 
        const uint8_t *M, uint32_t M_bytes, const uint8_t r[32], sm9_enc_type_e enc_type, 
        sm9_enc_padding_e padding_type, uint32_t K1_bytes, uint32_t K2_bytes, 
        const uint32_t *fp12g_u32, const uint32_t *QBx, const uint32_t *QBy, 
        uint8_t *C, uint32_t *C_bytes)
{
    uint32_t ret;
    uint32_t K2[SM9_MAX_ENC_K2_BYTE_LEN>>2];
    uint32_t tmp[8];

    uint8_t *C2 = &C[96u];
    hash_node_st digest_node[2];

    if(NULL == r)
    {
        do {
            ret = get_rand((uint8_t *)tmp, 32u);
            if(TRNG_SUCCESS != ret)
            {
                break;
            }
            else
            {}

            //get C1, K1, K2
            ret = sm9_enc_with_r(IDB, IDB_bytes, tmp, QBx, QBy, fp12g_u32,
                    C, C2, K1_bytes, (uint8_t *)K2, K2_bytes);
        } while((SM9_ZERO_ALL == ret) || (SM9_INTEGER_TOO_BIG == ret));
    }
    else
    {
        u8big_to_u32little_256bits(r, tmp);

        //get C1, K1, K2
        ret = sm9_enc_with_r(IDB, IDB_bytes, tmp, QBx, QBy, fp12g_u32, 
                C, C2, K1_bytes, (uint8_t *)K2, K2_bytes);
    }

    if(PKE_SUCCESS == ret)
    {
        C_bytes[0] = M_bytes;

        if(SM9_ENC_KDF_STREAM_CIPHER == enc_type)     //KDF
        {
            (void)uint8_XOR(C2, M, C2, M_bytes);
        }
        else if(SM9_ENC_KDF_BLOCK_CIPHER == enc_type) //SM4
        {
#if defined(SKE_HP)
            ret = ske_crypto(SKE_ALG_SM4, SKE_MODE_ECB, SKE_CRYPTO_ENCRYPT, C2, 0, NULL,
                    padding_type, M, C2, M_bytes, C_bytes);
#elif defined(SKE_LP)
            ret = ske_crypto(SKE_ALG_SM4, SKE_MODE_ECB, SKE_CRYPTO_ENCRYPT, C2, 0, NULL,
                    padding_type, M, C2, M_bytes, C_bytes);
#elif defined(SKE_SECURE)
            ret = ske_sec_crypto(SKE_ALG_SM4, SKE_MODE_ECB, SKE_CRYPTO_ENCRYPT, C2, 0, NULL,
                    padding_type, M, C2, M_bytes, C_bytes);
#else
            ret = SM9_INPUT_INVALID;
#endif
            if(SKE_SUCCESS == ret)
            {
                ret = PKE_SUCCESS;
            }
            else
            {}
        }
        else
        {
            //nothing to do, just for static analysis.
        }
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //get C3 := MAC(K2, C2) := sm3(C2||K2)
        digest_node[0].msg_addr  = C2;
        digest_node[0].msg_bytes = *C_bytes;
        digest_node[1].msg_addr  = (uint8_t *)K2;
        digest_node[1].msg_bytes = K2_bytes;
        ret = hash_node_steps(HASH_SM3, digest_node, 2u, &C[64u]);
        if(HASH_SUCCESS == ret)
        {
            C_bytes[0] += 96u;

            ret = PKE_SUCCESS;
        }
        else
        {}
    }
    else
    {}

    uint32_clear((uint32_t *)K2, (sizeof(K2))>>2);
    uint32_clear((uint32_t *)tmp, (sizeof(tmp))>>2);

    return ret;
}


/* function: SM9 encrypt
 * parameters:
 *     IDB ------------------------ input, identity of user B, user B is the cipher receiver 
 *     IDB_bytes ------------------ input, bytes length of IDB
 *     hid ------------------------ input, user private generation function identity, published by KGC,
 *                                         default value is 0x03, one byte.
 *     M -------------------------- input, message to be encrypted
 *     M_bytes -------------------- input, bytes length of message to be encrypted
 *     fp12g ---------------------- input, the value of e(Ppub_e, P2), if set to null, it will be calculated within the function
 *     Ppub_e --------------------- input,  KGC's system encryption master public key
 *     r -------------------------- input, random big integer r in wrapping, 32 bytes, big-endian,
 *                                  if you do not have this integer, please set this parameter to be NULL,
 *                                  it will be generated inside.
 *     enc_type ------------------- input, type of encryption (SM9_ENC_KDF_STREAM_CIPHER or SM9_ENC_KDF_BLOCK_CIPHER)
 *     padding_type --------------- input, type of padding(SKE_NO_PADDING or SKE_PKCS_5_7_PADDING)
 *     K2_bytes ------------------- input, bytes length of the key K2 in MAC function
 *     C -------------------------- output, the cipher
 *     C_bytes -------------------- output, bytes length of the cipher
 * return: SM9_SUCCESS(success), other(error)
 * caution:
 *     1. IDB only represents the decrypter.
 *     2. K2_bytes should be less than SM9_MAX_ENC_K2_BYTE_LEN
 */
uint32_t sm9_enc(const uint8_t *IDB, uint32_t IDB_bytes, uint8_t hid, const uint8_t *M, 
        uint32_t M_bytes, const uint8_t *fp12g, const uint8_t Ppub_e[64],  
        const uint8_t r[32], sm9_enc_type_e enc_type, sm9_enc_padding_e padding_type, 
        uint32_t K2_bytes, uint8_t *C, uint32_t *C_bytes)
{
    uint32_t ret;
    uint32_t QBx[8], QBy[8];
    uint32_t fp12g_u32[8u*12u];
    uint32_t K1_bytes = 0u;

    ret = sm9_enc_check_input(IDB, IDB_bytes, M, M_bytes, Ppub_e, enc_type, 
            padding_type, K2_bytes, C, C_bytes);
    if(PKE_SUCCESS == ret)
    {
        ret = sm9_enc_internal_step_1(IDB, IDB_bytes, hid, M_bytes, fp12g, Ppub_e, 
                enc_type, fp12g_u32, QBx, QBy, &K1_bytes);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_enc_internal_step_2(IDB, IDB_bytes, M, M_bytes, r, enc_type, 
                padding_type, K1_bytes, K2_bytes, fp12g_u32, QBx, QBy, C, C_bytes);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = SM9_SUCCESS;
    }
    else
    {
        *C_bytes = 0u;
        memset_(C, 0, 64u+32u+M_bytes);
    }

    uint32_clear((uint32_t *)QBx, (sizeof(QBx))>>2);
    uint32_clear((uint32_t *)QBy, (sizeof(QBy))>>2);
    uint32_clear((uint32_t *)fp12g_u32, (sizeof(fp12g_u32))>>2);

    return ret;
}


/* function: SM9 decrypt check input(internal API)
 * parameters:
 *     IDB ------------------------ input, pointer to identity of user B, user B is the cipher receiver 
 *     IDB_bytes ------------------ input, bytes length of IDB
 *     C -------------------------- input, pointer to the cipher
 *     C_bytes -------------------- input, bytes length of the cipher
 *     deB ------------------------ input, pointer to user B's private key
 *     enc_type ------------------- input, type of encryption (SM9_ENC_KDF_STREAM_CIPHER or SM9_ENC_KDF_BLOCK_CIPHER)
 *     padding_type --------------- input, type of padding(SKE_NO_PADDING or SKE_PKCS_5_7_PADDING)
 *     K2_bytes ------------------- input, bytes length of the key in MAC function
 *     M -------------------------- input, pointer to the plain message
 *     M_bytes -------------------- input, pointer to bytes length of the plaintext message
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_dec_check_input(const uint8_t *IDB, uint32_t IDB_bytes, 
        const uint8_t *C, uint32_t C_bytes, const uint8_t deB[128], sm9_enc_type_e enc_type, 
        sm9_enc_padding_e padding_type, uint32_t K2_bytes, const uint8_t *M, const uint32_t *M_bytes)
{
    uint32_t ret;

    if((NULL == IDB) || (NULL == M) || (NULL == C) || (NULL == deB) || (NULL == M_bytes))
    {
        ret = SM9_BUFFER_NULL;
    }
    else if(M == C)
    {
        ret = SM9_IN_OUT_SAME_BUFFER;
    }
    else if((C_bytes < (96u + 1u)) || (0u == IDB_bytes) || (K2_bytes > SM9_MAX_ENC_K2_BYTE_LEN) || \
            (enc_type > SM9_ENC_KDF_BLOCK_CIPHER))
    {
        ret = SM9_INPUT_INVALID;
    }
    else
    {
        ret = PKE_SUCCESS;
    }

    if(PKE_SUCCESS == ret)
    {
        if(SM9_ENC_KDF_BLOCK_CIPHER == enc_type)
        {
            if(padding_type > SKE_PKCS_5_7_PADDING)
            {
                ret = SM9_INPUT_INVALID;
            }
            else if(0u != ((C_bytes-96u) & 0x0Fu))
            {
                ret = SM9_INPUT_INVALID;
            }
            else
            {
                //nothing to do, just for static analysis.
            }
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: SM9 decrypt step 1(internal API)
 * parameters:
 *     C1 ------------------------- input, C1 part of the ciphertext C
 *     C2_bytes ------------------- input, bytes length of the C2 part of the ciphertext C
 *     deB ------------------------ input, cipher receiver's private key
 *     enc_type ------------------- input, type of encryption (SM9_ENC_KDF_STREAM_CIPHER or SM9_ENC_KDF_BLOCK_CIPHER)
 *     M -------------------------- input, pointer to the plain message
 *     K1 ------------------------- output, pointer to K1
 *     K1_bytes ------------------- output, byte length of K1
 *     w -------------------------- output, w = e(C1, deB)
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_dec_internal_step_1(const uint8_t *C1, uint32_t C2_bytes, 
        const uint8_t deB[128], sm9_enc_type_e enc_type, uint8_t *M, uint8_t **K1, 
        uint32_t *K1_bytes, uint32_t *w)
{
    uint32_t ret;

#if 0
    uint32_t Px[8], Py[8], Qx[16], Qy[16];
#else
#ifndef SUPPORT_STATIC_ANALYSIS
#define Px   ((uint32_t *)w)
#define Py   (((uint32_t *)w)+SM9_BASE_WORD_LEN)
#define Qx   (((uint32_t *)w)+(2u*SM9_BASE_WORD_LEN))
#define Qy   (((uint32_t *)w)+(4u*SM9_BASE_WORD_LEN))
#else
uint32_t *Px = w;
uint32_t *Py = &w[SM9_BASE_WORD_LEN];
uint32_t *Qx = &w[2u*SM9_BASE_WORD_LEN];
uint32_t *Qy = &w[4u*SM9_BASE_WORD_LEN];
#endif
#endif

    //check C1
    (void)sm9_fp_eccp_point_u8big_2_u32little(C1, Px, Py);
    ret = eccp_check_point(sm9_curve, Px, Py);
    if(PKE_SUCCESS != ret)
    {
        ret = SM9_NOT_ON_CURVE;
    }
    else
    {
        //check deB
        (void)sm9_fp2_eccp_point_u8big_2_u32little(deB, Qx, Qy);
        ret = sm9_fp2_check_point_internal(sm9_curve, Qx, Qy);
        if(PKE_SUCCESS != ret)
        {
            ret = SM9_NOT_ON_CURVE;
        }
        else
        {
            //get w := e(C1, deB)
            ret = sm9_pairing(Px, Py, Qx, Qy, NULL);
        }
    }

    if(PKE_SUCCESS == ret)
    {
        if(SM9_ENC_KDF_STREAM_CIPHER == enc_type)  //KDF
        {
            *K1 = M;
            *K1_bytes = C2_bytes;
        }
        else   //SM4(enc_type is SM9_ENC_KDF_BLOCK_CIPHER)
        {}

        sm9_fp12_in_pke_ram_u32little_2_u8big((uint8_t *)w);
    }
    else
    {}

#ifndef SUPPORT_STATIC_ANALYSIS
#undef Px
#undef Py
#undef Qx
#undef Qy
#endif

    return ret;
}


/* function: SM9 decrypt step 2(internal API)
 * parameters:
 *     IDB ------------------------ input, identity of user B, user B is the cipher receiver 
 *     IDB_bytes ------------------ input, bytes length of IDB
 *     C -------------------------- input, ciphertext C
 *     C2_bytes ------------------- input, bytes length of the C2 part of the ciphertext C
 *     enc_type ------------------- input, type of encryption (SM9_ENC_KDF_STREAM_CIPHER or SM9_ENC_KDF_BLOCK_CIPHER)
 *     padding_type --------------- input, type of padding(SKE_NO_PADDING or SKE_PKCS_5_7_PADDING)
 *     w -------------------------- input, w = e(C1, deB), U8 big-endian
 *     K1 ------------------------- input, K1
 *     K1_bytes ------------------- input, byte length of K1
 *     K2_bytes ------------------- input, byte length of K2
 *     M -------------------------- output, the plaintext message
 *     M_bytes -------------------- output, bytes length of the plaintext message
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_dec_internal_step_2(const uint8_t *IDB, uint32_t IDB_bytes, 
        const uint8_t *C, uint32_t C2_bytes, sm9_enc_type_e enc_type, 
        sm9_enc_padding_e padding_type, uint8_t *w, uint8_t *K1, uint32_t K1_bytes, 
        uint32_t K2_bytes, uint8_t *M, uint32_t *M_bytes)
{
    uint32_t ret;
    uint32_t counter_buf[1] = {0u};
    const uint8_t *counter = (uint8_t *)counter_buf;
    uint32_t K2[SM9_MAX_ENC_K2_BYTE_LEN>>2];
    const uint8_t *C2 = &C[96u];

    hash_node_st digest_node[4];

    ((uint8_t *)counter_buf)[3] = (uint8_t)1u;
    digest_node[0].msg_addr  = C;
    digest_node[0].msg_bytes = 64u;
    digest_node[1].msg_addr  = (uint8_t *)w;
    digest_node[1].msg_bytes = 32u*12u;
    digest_node[2].msg_addr  = IDB;
    digest_node[2].msg_bytes = IDB_bytes;
    digest_node[3].msg_addr  = counter;
    digest_node[3].msg_bytes = 4u;

    ret = ansi_x9_63_kdf_node(HASH_SM3, digest_node, 4u, (uint8_t *)counter_buf, K1, K1_bytes, 
            (uint8_t *)K2, K2_bytes);
    if(HASH_SUCCESS == ret)
    {
        if(0u != uint8_BigNum_Check_Zero(K1, K1_bytes))
        {
            ret = SM9_ZERO_ALL;
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
        if(SM9_ENC_KDF_STREAM_CIPHER == enc_type)  //KDF
        {
            (void)uint8_XOR(K1, C2, M, C2_bytes);  //actually K1 is M here

            *M_bytes = C2_bytes;
        }
        else   //SM4(enc_type is SM9_ENC_KDF_BLOCK_CIPHER)
        {
            //C2_bytes is a multiple of 16
#if defined(SKE_HP)
            ret = ske_crypto(SKE_ALG_SM4, SKE_MODE_ECB, SKE_CRYPTO_DECRYPT, (uint8_t *)K1, 0, NULL,
                    padding_type, C2, M, C2_bytes, M_bytes);
#elif defined(SKE_LP)
            ret = ske_crypto(SKE_ALG_SM4, SKE_MODE_ECB, SKE_CRYPTO_DECRYPT, (uint8_t *)K1, 0, NULL,
                    padding_type, C2, M, C2_bytes, M_bytes);
#elif defined(SKE_SECURE)
            ret = ske_sec_crypto(SKE_ALG_SM4, SKE_MODE_ECB, SKE_CRYPTO_DECRYPT, (uint8_t *)K1, 0, NULL,
                    padding_type, C2, M, C2_bytes, M_bytes);
#else
            ret = SM9_INPUT_INVALID;
#endif
            if(SKE_SUCCESS != ret)
            {
                ret = SM9_DECRY_VERIFY_FAILED;
            }
            else
            {
                ret = PKE_SUCCESS;
            }
        }
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //get u := MAC(K2, C2) := sm3(C2||K2)
        digest_node[0].msg_addr  = C2;
        digest_node[0].msg_bytes = C2_bytes;
        digest_node[1].msg_addr  = (uint8_t *)K2;
        digest_node[1].msg_bytes = K2_bytes;
        ret = hash_node_steps(HASH_SM3, digest_node, 2u, (uint8_t *)w);
        if(HASH_SUCCESS == ret)
        {
            //check u = C3 ?
            if((uint8_t)0 != memcmp_(&C[64u], (uint8_t *)w, 32u))
            {
                ret = SM9_DECRY_VERIFY_FAILED;
            }
            else
            {
                ret = SM9_SUCCESS;
            }
        }
        else
        {}
    }
    else
    {}

    uint32_clear(counter_buf, (sizeof(counter_buf))>>2);
    uint32_clear((uint32_t *)K2, (sizeof(K2))>>2);

    return ret;
}


/* function: SM9 decrypt
 * parameters:
 *     IDB ------------------------ input, identity of user B, user B is the cipher receiver 
 *     IDB_bytes ------------------ input, bytes length of IDB
 *     C -------------------------- input, the cipher
 *     C_bytes -------------------- input, bytes length of the cipher
 *     deB ------------------------ input, user B's private key
 *     enc_type ------------------- input, type of encryption (SM9_ENC_KDF_STREAM_CIPHER or SM9_ENC_KDF_BLOCK_CIPHER)
 *     padding_type --------------- input, type of padding(SKE_NO_PADDING or SKE_PKCS_5_7_PADDING)
 *     K2_bytes ------------------- input, bytes length of the key in MAC function
 *     M -------------------------- output, the plaintext message
 *     M_bytes -------------------- output, bytes length of the plaintext message
 * return: SM9_SUCCESS(success), other(error)
 * caution:
 */
uint32_t sm9_dec(const uint8_t *IDB, uint32_t IDB_bytes, const uint8_t *C, uint32_t C_bytes, 
        const uint8_t deB[128], sm9_enc_type_e enc_type, sm9_enc_padding_e padding_type, 
        uint32_t K2_bytes, uint8_t *M, uint32_t *M_bytes)
{
    uint32_t ret;
    uint32_t K1_buf[4];
    uint8_t *K1 = (uint8_t *)K1_buf; //as default, use block cipher(SM4) to encrypt, k1 is 16 bytes
    uint32_t K1_bytes = 16u;
    uint32_t w[8u*12u];
    uint32_t C2_bytes = 0u;

    ret = sm9_dec_check_input(IDB, IDB_bytes, C, C_bytes, deB, enc_type, padding_type, 
            K2_bytes, M, M_bytes);
    if(PKE_SUCCESS == ret)
    {
        C2_bytes = C_bytes - 96u;
        ret = sm9_dec_internal_step_1(C, C2_bytes, deB, enc_type, M, &K1, &K1_bytes, w);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_dec_internal_step_2(IDB, IDB_bytes, C, C2_bytes, enc_type, padding_type, 
                (uint8_t *)w, K1, K1_bytes, K2_bytes, M, M_bytes);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = SM9_SUCCESS;
    }
    else
    {
        memset_(M, 0, C2_bytes);
        *M_bytes = 0u;
    }

    uint32_clear((uint32_t *)K1_buf, (sizeof(K1_buf))>>2);
    uint32_clear((uint32_t *)w, (sizeof(w))>>2);

    return ret;
}


/* function: Generate KGC's master public key from master private key for SM9 key-exchange system.
 * parameters:
 *     ke ------------------------- input, KGC's master private key, 32 bytes, big-endian
 *     Ppub_e --------------------- output, KGC's master public key(x||y), 64 bytes, big-endian
 * return: SM9_SUCCESS(success); other(error)
 * caution:
 */
uint32_t sm9_exckey_gen_mastPubKey_from_mastPriKey(const uint8_t ke[32], uint8_t Ppub_e[64])
{
    return sm9_enc_gen_mastPubKey_from_mastPriKey(ke, Ppub_e);
}


/* function: Generate random KGC's master key pair for SM9 key-exchange system.
 * parameters:
 *     ke ------------------------- output, KGC's master private key, 32 bytes, big-endian
 *     Ppub_e --------------------- output, KGC's master public key(x||y), 64 bytes, big-endian
 * return: SM9_SUCCESS(success); other(error)
 * caution:
 */
uint32_t sm9_exckey_gen_mastKeyPair(uint8_t ke[32], uint8_t Ppub_e[64])
{
    return sm9_enc_gen_mastKeyPair(ke, Ppub_e);
}


/* function: Generate user's private key for SM9 key-exchange system.
 * parameters:
 *     IDA ------------------------ input, identify of user A
 *     IDA_bytes ------------------ input, bytes length of the IDA
 *     hid ------------------------ input, user private key generation function identity, published
 *                                         by KGC, default value is 0x02, one byte.
 *     ke ------------------------- input, KGC's master private key, 32 bytes, big-endian
 *     deA ------------------------ output, user A's private key, 128 bytes, big-endian
 * return: SM9_SUCCESS(success), other(error)
 * caution:
 */
uint32_t sm9_exckey_gen_userPriKey(const uint8_t *IDA, uint32_t IDA_bytes, uint8_t hid, 
        const uint8_t ke[32], uint8_t deA[128])
{
    return sm9_enc_gen_userPriKey(IDA, IDA_bytes, hid, ke, deA);
}


/* function: Generate user's temporary public key from private key for SM9 key-exchange system(internal API)
 * parameters:
 *     IDB ------------------------ input, peer's ID
 *     IDB_bytes ------------------ input, byte length of IDB
 *     hid ------------------------ input, user private key generation function identity, published 
 *                                         by KGC, default value is 0x02, one byte.
 *     Ppub_e --------------------- input, KGC's public key
 *     rA ------------------------- input, local's temporary private key, 32 bytes, big-endian
 *     RA ------------------------- output, local's temporary public key(x||y), 64 bytes, big-endian
 * return: SM9_SUCCESS(success); other(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_exckey_gen_tmpPubKey_internal(const uint8_t *IDB, uint32_t IDB_bytes, 
        uint8_t hid, const uint8_t Ppub_e[64], const uint8_t rA[32], uint8_t RA[64])
{
    uint32_t ret;
    uint32_t t1[SM9_BASE_WORD_LEN];
    uint32_t t2[SM9_BASE_WORD_LEN];
    uint32_t t3[SM9_BASE_WORD_LEN];
    uint32_t t4[SM9_BASE_WORD_LEN];
    uint8_t hid_value = hid;

    //t1 := h1 := H1(IDB||hid, N)
    ret = sm9_h1_h2((uint8_t)1, IDB, IDB_bytes, &hid_value, 1u, t1);
    if(PKE_SUCCESS == ret)
    {
        //(t1, t2) = [h1]P1
        ret = eccp_pointMul(sm9_curve, t1, sm9_curve->eccp_Gx, sm9_curve->eccp_Gy, t1, t2);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //check Ppub_e
        (void)sm9_fp_eccp_point_u8big_2_u32little(Ppub_e, t3, t4);
        ret = eccp_check_point_internal(sm9_curve, t3, t4);
        if(PKE_SUCCESS != ret)
        {
            ret = SM9_NOT_ON_CURVE;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //QB(t1, t2) = [h1]P1 + Ppub_s
        ret = eccp_pointAdd_safe_internal(sm9_curve, t1, t2, t3, t4, t1, t2);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //make sure local's temporary private key in [1, n-1]
        u8big_to_u32little_256bits(rA, t3);
        ret = uint32_integer_check(t3, sm9_curve->eccp_n, SM9_BASE_WORD_LEN, SM9_ZERO_ALL, 
                SM9_INTEGER_TOO_BIG, PKE_SUCCESS);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //RA = [rA]QB
        ret = eccp_pointMul_internal(sm9_curve, t3, t1, t2, t1, t2);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)sm9_fp_eccp_point_u32little_2_u8big(t1, t2, RA);

        ret = SM9_SUCCESS;
    }
    else
    {}

    uint32_clear(t1, (sizeof(t1))>>2);
    uint32_clear(t2, (sizeof(t2))>>2);
    uint32_clear(t3, (sizeof(t3))>>2);
    uint32_clear(t4, (sizeof(t4))>>2);

    return ret;
}


/* function: Generate user's temporary public key from private key for SM9 key-exchange system.
 * parameters:
 *     IDB ------------------------ input, peer's ID
 *     IDB_bytes ------------------ input, byte length of IDB
 *     hid ------------------------ input, user private key generation function identity, published 
 *                                         by KGC, default value is 0x02, one byte.
 *     Ppub_e --------------------- input, KGC's public key
 *     rA ------------------------- input, local's temporary private key, 32 bytes, big-endian
 *     RA ------------------------- output, local's temporary public key(x||y), 64 bytes, big-endian
 * return: SM9_SUCCESS(success); other(error)
 * caution:
 */
uint32_t sm9_exckey_gen_tmpPubKey_from_tmpPriKey(const uint8_t *IDB, uint32_t IDB_bytes, 
        uint8_t hid, const uint8_t Ppub_e[64], const uint8_t rA[32], uint8_t RA[64])
{
    uint32_t ret;

    if((NULL == IDB) || (NULL == Ppub_e) || (NULL == rA) || (NULL == RA))
    {
        ret = SM9_BUFFER_NULL;
    }
    else if(0u == IDB_bytes)
    {
        ret = SM9_INPUT_INVALID;
    }
    else
    {
        ret = sm9_exckey_gen_tmpPubKey_internal(IDB, IDB_bytes,  hid, 
                Ppub_e, rA, RA);
    }

    return ret;
}


/* function: Generate user's temporary random key pair for SM9 key-exchange system.
 * parameters:
 *     IDB ------------------------ input, peer's ID
 *     IDB_bytes ------------------ input, byte length of IDB
 *     hid ------------------------ input, user private generation function identity, published by KGC,
 *                                         default value is 0x02, one byte.
 *     Ppub_e --------------------- input, KGC's public key
 *     rA ------------------------- output, local's temporary private key, 32 bytes, big-endian
 *     RA ------------------------- output, local's temporary public key(x||y), 64 bytes, big-endian
 * return:
 *     SM9_SUCCESS(success); other(error)
 * caution:
 */
uint32_t sm9_exckey_gen_tmpKeyPair(const uint8_t *IDB, uint32_t IDB_bytes, uint8_t hid, 
        const uint8_t Ppub_e[64], uint8_t rA[32], uint8_t RA[64])
{
    uint32_t ret;

    if((NULL == IDB) || (NULL == Ppub_e) || (NULL == rA) || (NULL == RA))
    {
        ret = SM9_BUFFER_NULL;
    }
    else if(0u == IDB_bytes)
    {
        ret = SM9_INPUT_INVALID;
    }
    else
    {
        do {
            ret = get_rand(rA, 32u);
            if(TRNG_SUCCESS != ret)
            {
                break;
            }
            else
            {}

            ret = sm9_exckey_gen_tmpPubKey_from_tmpPriKey(IDB, IDB_bytes, hid, Ppub_e, rA, RA);
        } while((SM9_ZERO_ALL == ret) || (SM9_INTEGER_TOO_BIG == ret));
    }

    return ret;
}


/* function: SM9 Key exchange check input
 * parameters:
 *     role ----------------------- input, local user's role(SM9_Role_Sponsor or SM9_Role_Responsor)
 *     IDA ------------------------ input, pointer to local user's identity
 *     IDA_bytes ------------------ input, bytes length of IDA
 *     IDB ------------------------ input, pointer to peer user's identity
 *     IDB_bytes ------------------ input, bytes length of IDB
 *     deA ------------------------ input, pointer to local user's private key
 *     rA ------------------------- input, pointer to local user's temporary private key
 *     RA ------------------------- input, pointer to local user's temporary public key
 *     RB ------------------------- input, pointer to peer user's temporary public key
 *     k_bytes -------------------- input, bytes length of the output key
 *     k -------------------------- input, pointer to the output key
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. 
 */
FLAG_STATIC uint32_t sm9_exchangekey_s_check_input(sm9_exchange_role_e role, 
                                                 const uint8_t *IDA, uint32_t IDA_bytes,
                                                 const uint8_t *IDB, uint32_t IDB_bytes,
                                                 const uint8_t deA[128], const uint8_t rA[32],
                                                 const uint8_t RA[64], const uint8_t RB[64],
                                                 uint32_t k_bytes,
                                                 const uint8_t *k)
{
    uint32_t ret;

    if((NULL == IDA) || (NULL == IDB) || (NULL == deA) || (NULL == rA) || \
            (NULL == RA) || (NULL == RB) || (NULL == k))
    {
        ret = SM9_BUFFER_NULL;
    }
    else if(role > SM9_Role_Responsor)
    {
        ret = SM9_EXCHANGE_ROLE_INVALID;
    }
    else if((0u == IDA_bytes) || (0u == IDB_bytes) || (0u == k_bytes))
    {
        ret = SM9_INPUT_INVALID;
    }
    else
    {
        ret = PKE_SUCCESS;
    }

    return ret;
}


/* function: SM9 Key exchange step 1(internal API)
 * parameters:
 *     fp12g ---------------------- input, the value of e(Ppub_e, P2), if set to null, it will be calculated within the function
 *     Ppub_e --------------------- input, KGC's system encryption master public key
 *     RA ------------------------- input, local user's temporary public key
 *     RB ------------------------- input, peer user's temporary public key
 *     fp12g_u32 ------------------ output, g = e(Ppub_e, P2), 12*8 words, U32 little-endian
 *     RBx ------------------------ output, x coordinate of RB, 8 words, U32 little-endian
 *     RBy ------------------------ output, y coordinate of RB, 8 words, U32 little-endian
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. fp12g and Ppub_e can not be NULL at the same time
 */
FLAG_STATIC uint32_t sm9_exchangekey_s_internal_step_1(const uint8_t *fp12g, 
        const uint8_t Ppub_e[64], const uint8_t RA[64], const uint8_t RB[64], 
        uint32_t *fp12g_u32, uint32_t *RBx, uint32_t *RBy)
{
    uint32_t ret;
#if 0
    uint32_t Px[8], Py[8];
#else
#ifndef SUPPORT_STATIC_ANALYSIS
#define Px   (fp12g_u32)
#define Py   (fp12g_u32 + SM9_BASE_WORD_LEN)
#else
    uint32_t *Px  = fp12g_u32;
    uint32_t *Py  = &fp12g_u32[SM9_BASE_WORD_LEN];
#endif
#endif

    //just to check RA
    (void)sm9_fp_eccp_point_u8big_2_u32little(RA, RBx, RBy);
    ret = eccp_check_point(sm9_curve, RBx, RBy);
    if(PKE_SUCCESS != ret)
    {
        ret = SM9_NOT_ON_CURVE;
    }
    else
    {
        //check RB
        (void)sm9_fp_eccp_point_u8big_2_u32little(RB, RBx, RBy);
        ret = eccp_check_point_internal(sm9_curve, RBx, RBy);
        if(PKE_SUCCESS != ret)
        {
            ret = SM9_NOT_ON_CURVE;
        }
        else
        {}
    }

    if(PKE_SUCCESS == ret)
    {
        if(NULL != fp12g)
        {
            (void)sm9_fp12_u8big_2_u32little(fp12g, (uint32_t *)fp12g_u32);
        }
        else if(NULL != Ppub_e)
        {
            //check Ppub_e
            (void)sm9_fp_eccp_point_u8big_2_u32little(Ppub_e, Px, Py);
            ret = eccp_check_point_internal(sm9_curve, Px, Py);
            if(PKE_SUCCESS != ret)
            {
                ret = SM9_NOT_ON_CURVE;
            }
            else
            {
                ret = sm9_pairing(Px, Py, sm9_G2_Px0_Px1, sm9_G2_Py0_Py1, fp12g_u32);
            }
        }
        else
        {
            ret = SM9_BUFFER_NULL;
        }
    }
    else
    {}

#ifndef SUPPORT_STATIC_ANALYSIS
#undef Px
#undef Py
#endif

    return ret;
}


/* function: SM9 Key exchange step 2(internal API)
 * parameters:
 *     deA ------------------------ input, local user's private key
 *     rA ------------------------- input, local user's temporary private key
 *     RBx ------------------------ input, x coordinate of RB, 8 words, U32 little-endian
 *     RBy ------------------------ input, y coordinate of RB, 8 words, U32 little-endian
 *     fp12g ---------------------- input, g = e(Ppub_e, P2), 12*8 words, U32 little-endian
 *     fp12_1 --------------------- output, g1 = g^rA, 12*8 words, U32 little-endian
 *     fp12_2 --------------------- output, g2 = e(RB, deA), 12*8 words, U32 little-endian
 *     fp12_3 --------------------- output, g3 = g2^rA, 12*8 words, U32 little-endian
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. 
 */
FLAG_STATIC uint32_t sm9_exchangekey_s_internal_step_2(const uint8_t deA[128], 
        const uint8_t rA[32], const uint32_t *RBx, const uint32_t *RBy, 
        const uint32_t *fp12g, uint32_t *fp12_1, uint32_t *fp12_2, uint32_t *fp12_3)
{
    uint32_t ret;

#if 0
    uint32_t rA_u32[8];
    uint32_t Qx[16], Qy[16];
#else
#ifndef SUPPORT_STATIC_ANALYSIS
#define rA_u32  (fp12_3)
#define Qx      (fp12_3 + (SM9_BASE_WORD_LEN))
#define Qy      (fp12_3 + (3u*SM9_BASE_WORD_LEN))
#else
    uint32_t *rA_u32 = fp12_3;
    uint32_t *Qx     = &fp12_3[SM9_BASE_WORD_LEN];
    uint32_t *Qy     = &fp12_3[3u*SM9_BASE_WORD_LEN];
#endif
#endif

    //check rA
    u8big_to_u32little_256bits(rA, rA_u32);
    ret = uint32_integer_check(rA_u32, sm9_curve->eccp_n, SM9_BASE_WORD_LEN, SM9_ZERO_ALL, 
        SM9_INTEGER_TOO_BIG, PKE_SUCCESS);
    if(PKE_SUCCESS == ret)
    {
        ret = sm9_fp12_exp_s(fp12g, rA_u32, fp12_1);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //check deA
        (void)sm9_fp2_eccp_point_u8big_2_u32little(deA, Qx, Qy);
        ret = sm9_fp2_check_point_internal(sm9_curve, Qx, Qy);
        if(PKE_SUCCESS != ret)
        {
            ret = SM9_NOT_ON_CURVE;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_pairing(RBx, RBy, Qx, Qy, fp12_2);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_fp12_exp_s(fp12_2, rA_u32, fp12_3);
    }
    else
    {}

#ifndef SUPPORT_STATIC_ANALYSIS
#undef rA_u32
#undef Qx
#undef Qy
#endif

    return ret;
}


/* function: SM9 Key exchange step 3(internal API)
 * parameters:
 *     role ----------------------- input, local user's role(SM9_Role_Sponsor or SM9_Role_Responsor)
 *     IDA ------------------------ input, local user's identity
 *     IDA_bytes ------------------ input, bytes length of IDA
 *     IDB ------------------------ input, peer user's identity
 *     IDB_bytes ------------------ input, bytes length of IDB
 *     fp12_1 --------------------- input, g1 = g^rA, 12*8 words, U32 little-endian
 *     fp12_2 --------------------- input, g2 = e(RB, deA), 12*8 words, U32 little-endian
 *     fp12_3 --------------------- input, g3 = g2^rA, 12*8 words, U32 little-endian
 *     RA ------------------------- input, local user's temporary public key
 *     RB ------------------------- input, peer user's temporary public key
 *     k_bytes -------------------- input, bytes length of the output key
 *     k -------------------------- output, output key
 *     S1 ------------------------- output, sponsor's S1, or responsor's S2
 *     SA ------------------------- output, sponsor's SA, or responsor's SB
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. 
 */
FLAG_STATIC uint32_t sm9_exchangekey_s_internal_step_3(sm9_exchange_role_e role,
                        const uint8_t *IDA, uint32_t IDA_bytes,
                        const uint8_t *IDB, uint32_t IDB_bytes,
                        uint32_t *fp12_1, uint32_t *fp12_2, uint32_t *fp12_3,
                        const uint8_t RA[64], const uint8_t RB[64],
                        uint32_t k_bytes,
                        uint8_t *k, uint8_t S1[32], uint8_t SA[32])
{
    uint32_t ret;
    uint32_t counter_buf[1] = {0u};
    const uint8_t *counter = (uint8_t *)counter_buf;
    const uint8_t *sponsor_g1 = (uint8_t *)fp12_1;
    uint32_t *tmp = fp12_3;   //digest

    hash_node_st digest_node[8];
    uint8_t tag;

    (void)sm9_fp12_u32little_2_u8big(fp12_1);
    (void)sm9_fp12_u32little_2_u8big(fp12_2);
    (void)sm9_fp12_u32little_2_u8big(fp12_3);

    if(SM9_Role_Sponsor == role)
    {
        digest_node[0].msg_addr  = IDA;
        digest_node[0].msg_bytes = IDA_bytes;
        digest_node[1].msg_addr  = IDB;
        digest_node[1].msg_bytes = IDB_bytes;
        digest_node[2].msg_addr  = RA;
        digest_node[3].msg_addr  = RB;
        digest_node[4].msg_addr  = (uint8_t *)fp12_1;
        digest_node[5].msg_addr  = (uint8_t *)fp12_2;
        digest_node[6].msg_addr  = (uint8_t *)fp12_3;
    }
    else
    {
        digest_node[0].msg_addr  = IDB;
        digest_node[0].msg_bytes = IDB_bytes;
        digest_node[1].msg_addr  = IDA;
        digest_node[1].msg_bytes = IDA_bytes;
        digest_node[2].msg_addr  = RB;
        digest_node[3].msg_addr  = RA;
        digest_node[4].msg_addr  = (uint8_t *)fp12_2;
        digest_node[5].msg_addr  = (uint8_t *)fp12_1;
        digest_node[6].msg_addr  = (uint8_t *)fp12_3;
    }

    ((uint8_t *)counter_buf)[3] = (uint8_t)1u;
    digest_node[2].msg_bytes = 64u;
    digest_node[3].msg_bytes = 64u;
    digest_node[4].msg_bytes = 32u*12u;
    digest_node[5].msg_bytes = 32u*12u;
    digest_node[6].msg_bytes = 32u*12u;
    digest_node[7].msg_addr  = counter;
    digest_node[7].msg_bytes = 4u;

    //key := kdf(IDA||IDB||RA||RB||g1||g2||g3, key_bytes)
    ret = ansi_x9_63_kdf_node(HASH_SM3, digest_node, 8u, (uint8_t *)counter_buf, k, k_bytes, NULL, 0u);

    //check values are optional
    if((HASH_SUCCESS == ret) && (NULL != S1) && (NULL != SA))
    {
        if(SM9_Role_Sponsor == role)
        {
            sponsor_g1 = (uint8_t *)fp12_1;

            //tmp := Hash(g2||g3||IDA||IDB||RA||RB))
            digest_node[0].msg_addr  = (uint8_t *)fp12_2;
            digest_node[1].msg_addr  = (uint8_t *)fp12_3;
            digest_node[2].msg_addr  = IDA;
            digest_node[2].msg_bytes = IDA_bytes;
            digest_node[3].msg_addr  = IDB;
            digest_node[3].msg_bytes = IDB_bytes;
            digest_node[4].msg_addr  = RA;
            digest_node[5].msg_addr  = RB;
        }
        else
        {
            sponsor_g1 = (uint8_t *)fp12_2;

            //tmp = Hash(g1||g3||IDB||IDA||RB||RA)), here g1 is responsor's g1, it is expected to be sponsor's g2
            digest_node[0].msg_addr  = (uint8_t *)fp12_1;
            digest_node[1].msg_addr  = (uint8_t *)fp12_3;
            digest_node[2].msg_addr  = IDB;
            digest_node[2].msg_bytes = IDB_bytes;
            digest_node[3].msg_addr  = IDA;
            digest_node[3].msg_bytes = IDA_bytes;
            digest_node[4].msg_addr  = RB;
            digest_node[5].msg_addr  = RA;
        }

        digest_node[0].msg_bytes = 32u*12u;
        digest_node[1].msg_bytes = 32u*12u;
        digest_node[4].msg_bytes = 64u;
        digest_node[5].msg_bytes = 64u;

        ret = hash_node_steps(HASH_SM3, digest_node, 6u, (uint8_t *)tmp);
        if(HASH_SUCCESS == ret)
        {
            digest_node[0].msg_addr  = &tag;
            digest_node[0].msg_bytes = 1u;
            digest_node[1].msg_addr  = sponsor_g1;
            digest_node[1].msg_bytes = 32u*12u;
            digest_node[2].msg_addr  = (uint8_t *)tmp;
            digest_node[2].msg_bytes = 32u;

            tag = (uint8_t)0x82;
            if(SM9_Role_Sponsor == role)
            {
                ret = hash_node_steps(HASH_SM3, digest_node, 3u, S1);
            }
            else
            {
                ret = hash_node_steps(HASH_SM3, digest_node, 3u, SA);
            }
        }
        else
        {}

        if(HASH_SUCCESS == ret)
        {
            tag = (uint8_t)0x83;
            if(SM9_Role_Sponsor == role)
            {
                ret = hash_node_steps(HASH_SM3, digest_node, 3u, SA);
            }
            else
            {
                ret = hash_node_steps(HASH_SM3, digest_node, 3u, S1);
            }
        }
        else
        {}
    }
    else
    {}

    uint32_clear(counter_buf, (sizeof(counter_buf))>>2);

    return ret;
}


/* function: SM9 Key exchange
 * parameters:
 *     role ----------------------- input, local user's role(SM9_Role_Sponsor or SM9_Role_Responsor)
 *     IDA ------------------------ input, local user's identity
 *     IDA_bytes ------------------ input, bytes length of IDA
 *     IDB ------------------------ input, peer user's identity
 *     IDB_bytes ------------------ input, bytes length of IDB
 *     fp12g ---------------------- input, the value of e(Ppub_e, P2), if set to null, it will be calculated within the function
 *     Ppub_e --------------------- input, KGC's system encryption master public key
 *     deA ------------------------ input, local user's private key
 *     rA ------------------------- input, local user's temporary private key
 *     RA ------------------------- input, local user's temporary public key
 *     RB ------------------------- input, peer user's temporary public key
 *     k_bytes -------------------- input, bytes length of the output key
 *     k -------------------------- output, output key
 *     S1 ------------------------- output, sponsor's S1, or responsor's S2
 *     SA ------------------------- output, sponsor's SA, or responsor's SB
 * return: SM9_SUCCESS(success), other(error)
 * caution:
 *     1. fp12g and Ppub_e can not be NULL at the same time
 */
uint32_t sm9_exchangekey(sm9_exchange_role_e role,
                        const uint8_t *IDA, uint32_t IDA_bytes,
                        const uint8_t *IDB, uint32_t IDB_bytes,
                        const uint8_t *fp12g,
                        const uint8_t Ppub_e[64],
                        const uint8_t deA[128], const uint8_t rA[32],
                        const uint8_t RA[64], const uint8_t RB[64],
                        uint32_t k_bytes,
                        uint8_t *k, uint8_t S1[32], uint8_t SA[32])
{
    uint32_t ret;
    uint32_t fp12_1[8u*12u];
    uint32_t fp12_2[8u*12u];
    uint32_t fp12_3[8u*12u];

#if 0
    uint32_t RBx[8], RBy[8];
#else
#ifndef SUPPORT_STATIC_ANALYSIS
#define RBx  (fp12_3 + (5u*SM9_BASE_WORD_LEN))
#define RBy  (fp12_3 + (6u*SM9_BASE_WORD_LEN))
#else
    uint32_t *RBx = &fp12_3[5u*SM9_BASE_WORD_LEN];
    uint32_t *RBy = &fp12_3[6u*SM9_BASE_WORD_LEN];
#endif
#endif

    ret = sm9_exchangekey_s_check_input(role, IDA, IDA_bytes, IDB, IDB_bytes,
                                      deA, rA, RA, RB, k_bytes, k);
    if(PKE_SUCCESS == ret)
    {
        ret = sm9_exchangekey_s_internal_step_1(fp12g, Ppub_e, RA, RB, fp12_2, RBx, RBy);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_exchangekey_s_internal_step_2(deA, rA, RBx, RBy, fp12_2, fp12_1, fp12_2, fp12_3);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_exchangekey_s_internal_step_3(role, IDA, IDA_bytes, IDB, IDB_bytes,
                        fp12_1, fp12_2, fp12_3, RA, RB, k_bytes, k, S1, SA);
        if(HASH_SUCCESS == ret)
        {
            ret = SM9_SUCCESS;
        }
        else
        {}
    }
    else
    {}

#ifndef SUPPORT_STATIC_ANALYSIS
#undef RBx
#undef RBy
#endif

    uint32_clear(fp12_1, (sizeof(fp12_1))>>2);
    uint32_clear(fp12_2, (sizeof(fp12_2))>>2);
    uint32_clear(fp12_3, (sizeof(fp12_3))>>2);

    return ret;
}

#endif

