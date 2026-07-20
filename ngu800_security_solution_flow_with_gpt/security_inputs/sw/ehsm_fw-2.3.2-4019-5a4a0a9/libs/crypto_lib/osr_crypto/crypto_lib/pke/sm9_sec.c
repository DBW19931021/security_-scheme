
#include "../../crypto_hal/pke.h"
#include "../../crypto_include/pke/sm9.h"


#if (defined(SUPPORT_SM9) && defined(SM9_SEC))

#include "./sm9_internal.h"
#include "../../crypto_include/hash_hmac/hash_kdf.h"
#include "../../crypto_include/trng/trng.h"
#include "../../crypto_include/crypto_common/utility_sec.h"
#include "eccp_sec_common.h"



#define SM9_SEC_SIGN_COUNTER1       (0x5776CECDU)
#define SM9_SEC_SIGN_COUNTER2       (0xC333DA7FU)
#define SM9_SEC_WRAP_COUNTER        (0x4BE34FF6U)
#define SM9_SEC_DEC_COUNTER         (0xBC5827A9U)
#define SM9_SEC_EXC_COUNTER         (0x27F8C9EDU)


/* function: SM9 signature internal step 1
 * parameters:
 *     r --------------------------- input, random integer r in signing
 *     msg ------------------------- input, message to be signed
 *     msg_bytes ------------------- input, bytes length of message
 *     el -------------------------- output, el = (r-h) mod n
 *     h2rf_para ------------------- output, parameters for subsequent calculations
 *     el_2 ------------------------ output, repeated calculation of el value
 *     t --------------------------- output, random number for sleeping
 *     count ------------------------ input&output, counter, used to record the number of operations
 *     curve_p ---------------------- input, SM9 curve parameter p
 *     curve_p_h -------------------- input, SM9 curve parameter p's hash value
 *     curve_n ---------------------- input, SM9 curve parameter n
 *     curve ------------------------ input, pointer to ECCP curve structure
 *     h --------------------------- output, H2(msg||w, N)
 *     h_2 -------------------------- output, repeated calculation of h value
 * return: SM9_SUCCESS_S(success), other(error)
 * caution:
 */
static uint32_t sm9_sign_with_r_s_internal_step_1(
    uint32_t *r, 
    const uint8_t *msg, 
    uint32_t msg_bytes, 
    uint32_t el[8],
    uint8_t h2rf_para[32u*12u],
    uint32_t el_2[8],
    uint32_t t[1],
    volatile uint32_t *count,
    uint32_t *curve_p, 
    uint32_t *curve_p_h, 
    uint32_t *curve_n,
    const eccp_curve_st *curve,
    uint8_t h[32],
    uint32_t h_2[8]
)
{
    uint32_t ret;

    //copy back sm9 curve paras in PKE RAM that not erased by hardware
    uint32_copy_8_words(curve_p, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));
    uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM9_STEPS)));

    //copy back r for the following calculation, and to compare r outside
    uint32_copy_8_words(r, (uint32_t *)rPKE_A(1u,SM9_STEPS));

    (void)counter_add_one(count);

    (void)sm9_fp12_in_pke_ram_u32little_2_u8big(h2rf_para);

    (void)counter_add_one(count);

    /********* calc el and h *********/
    //get h := H2(msg||w, N)
    ret = sm9_h1_h2((uint8_t)2, msg, msg_bytes, h2rf_para, 32u*12u, el);

    if(PKE_SUCCESS == ret)
    {
        u32little_to_u8big_256bits(el, h);

        (void)counter_add_one(count);

        //el = (r-h) mod n
#if 0
        ret = pke_modsub(curve->eccp_n, r, el, el, SM9_BASE_WORD_LEN);
#else
        ret = pke_modadd_modsub_256bits(curve->eccp_n, r, el, el, MICROCODE_MODSUB);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //copy back sm9 curve paras in PKE RAM that not erased by hardware
        uint32_copy_8_words(curve_n, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));

        (void)counter_add_one(count);

        if(0u != uint32_BigNum_Check_Zero_sec(el, SM9_BASE_WORD_LEN))
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
        (void)counter_add_one(count);

        //get rand for sleeping
        ret = get_rand((uint8_t *)t, 1u);
        if(TRNG_SUCCESS != ret)
        {
            ret = SM9_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    if(SM9_SUCCESS == ret)
    {
        //sleep between the two repeated calculations
        uint32_sleep(t[0] & 0x0Fu, (uint8_t)(t[0] >> 4u));

        (void)counter_add_one(count);

        /********* calc el and h again *********/
        //get h_2 := H2(msg||w, N)
        ret = sm9_h1_h2((uint8_t)2, msg, msg_bytes, h2rf_para, 32u*12u, el_2);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        u32little_to_u8big_256bits(el_2, (uint8_t *)h_2);

        (void)counter_add_one(count);

        //el_2 = (r-h_2) mod n
#if 0
        ret = pke_modsub(curve->eccp_n, r, el_2, el_2, SM9_BASE_WORD_LEN);
#else
        ret = pke_modadd_modsub_256bits(curve->eccp_n, r, el_2, el_2, MICROCODE_MODSUB);
#endif
        if(PKE_SUCCESS == ret)
        {
            //copy back sm9 curve paras in PKE RAM that not erased by hardware
            uint32_copy_8_words(curve_n, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));

            if(0u != uint32_BigNum_Check_Zero_sec(el_2, SM9_BASE_WORD_LEN))
            {
                ret = SM9_ZERO_ALL;
            }
            else
            {}
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: SM9 signature internal step 2
 * parameters:
 *     el --------------------------- input, el = (r-h) mod n
 *     h --------------------------- input, partial signature result h
 *     el_2 ------------------------ input, repeated calculation of el value
 *     h_2 -------------------------- input, repeated calculation of h value
 *     buf ------------------------- input&output, buffer for comparison of h value
 *     t --------------------------- input, random number for sleeping
 *     count ------------------------ input&output, counter, used to record the number of operations
 * return: SM9_SUCCESS_S(success), other(error)
 * caution:
 */
static uint32_t sm9_sign_with_r_s_internal_step_2(
    const uint32_t el[8], 
    const uint8_t h[32],
    const uint32_t el_2[8],
    const uint32_t h_2[8],
    uint32_t buf[8],
    uint32_t t[1],
    volatile uint32_t *count
)
{
    uint32_t ret = PKE_SUCCESS;

    (void)counter_add_one(count);
    memcpy_(buf, h, 32U);

    // compare 3 times
    t[0] >>= 8;
    uint32_sleep(t[0] & 0x0Fu, (uint8_t)(t[0] >> 4u));
    if(0u != uint32_cmp_sec(el, el_2, 8u, (uint8_t)(t[0] >> 5)))
    {
        ret = SM9_ERROR_S;
    }
    else if(0u != uint32_cmp_sec(buf, h_2, 8u, (uint8_t)(t[0] >> 6)))
    {
        ret = SM9_ERROR_S;
    }
    else
    {
        //nothing to do, just for static analysis.
    }

    if(PKE_SUCCESS ==  ret)
    {
        (void)counter_add_one(count);

        t[0] >>= 8;
        uint32_sleep(t[0] & 0x0Fu, (uint8_t)(t[0] >> 4u));
        if(0u != uint32_cmp_sec(el, el_2, 8u, (uint8_t)(t[0] >> 5)))
        {
            ret = SM9_ERROR_S;
        }
        else if(0u != uint32_cmp_sec(buf, h_2, 8u, (uint8_t)(t[0] >> 6)))
        {
            ret = SM9_ERROR_S;
        }
        else
        {
            //nothing to do, just for static analysis.
        }
    }
    else
    {}

    if(PKE_SUCCESS ==  ret)
    {
        (void)counter_add_one(count);

        t[0] >>= 8;
        uint32_sleep(t[0] & 0x0Fu, (uint8_t)(t[0] >> 4u));
        if(0u != uint32_cmp_sec(el, el_2, 8u, (uint8_t)(t[0] >> 5)))
        {
            ret = SM9_ERROR_S;
        }
        else if(0u != uint32_cmp_sec(buf, h_2, 8u, (uint8_t)(t[0] >> 6)))
        {
            ret = SM9_ERROR_S;
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

/* function: SM9 sign with r, get el and h(secure version)
 * parameters:
 *     ctx ------------------------ input, eccp_sec_ctx_t struct pointer
 *     fg ------------------------- input, the value of e(P1, Ppub_s)
 *     r -------------------------- input, random integer r in signing
 *     msg ------------------------ input, message to be signed
 *     msg_bytes ------------------ input, bytes length of message
 *     el ------------------------- output, el = (r-h) mod n
 *     h -------------------------- output, partial signature result h
 * return: SM9_SUCCESS_S(success), other(error)
 * caution:
 *     1. ctx must be initialized by eccp_curve_init() before calling this function.
 *     2. fg, r, can not be modified
 */
static uint32_t sm9_sign_with_r_s(eccp_sec_ctx_t *ctx, const uint32_t *fg, uint32_t *r, 
        const uint8_t *msg, uint32_t msg_bytes, uint32_t el[8], uint8_t h[32])
{
    uint32_t ret;
    uint8_t h2rf_para[32u*12u];
    uint32_t el_2[8];
    uint32_t h_2[8];
    uint32_t buf[8];
    uint32_t t[1];
    volatile uint32_t count = SM9_SEC_SIGN_COUNTER1;
    uint32_t *curve_p, *curve_p_h, *curve_n;
    const eccp_curve_st *curve;

    curve = ctx->curve;

    //actually the following pointers are the same as the corresponding fields of curve
    curve_p   = &(ctx->eccp_curve_mem[0]);
    curve_p_h = &(ctx->eccp_curve_mem[SM9_BASE_WORD_LEN]);
    curve_n   = &(ctx->eccp_curve_mem[SM9_BASE_WORD_LEN*6u]);

    (void)counter_add_one(&count);

    //make sure r in [1, n-1]
    ret = uint32_integer_check_sec(r, curve->eccp_n, SM9_BASE_WORD_LEN, SM9_ZERO_ALL, SM9_INTEGER_TOO_BIG, SM9_SUCCESS_S);
    if(SM9_SUCCESS_S == ret)
    {
        (void)counter_add_one(&count);

        //get w = g^r
        ret = sm9_fp12_exp_s(fg, r, NULL);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_sign_with_r_s_internal_step_1(r, msg, msg_bytes, el, h2rf_para, el_2, t, &count, curve_p, 
            curve_p_h, curve_n, curve, h, h_2);
    }
    else
    {}

    if(PKE_SUCCESS ==  ret)
    {
        ret = sm9_sign_with_r_s_internal_step_2(el, h, el_2, h_2, buf, t, &count);
    }
    else
    {}

    if(PKE_SUCCESS ==  ret)
    {
        if(count == (SM9_SEC_SIGN_COUNTER1 + 0x0CU))
        {
            ret = SM9_SUCCESS_S;
        }
        else
        {}
    }
    else
    {
        (void)get_rand_fast((uint8_t *)h, SM9_BASE_BYTE_LEN);
        (void)get_rand_fast((uint8_t *)el, SM9_BASE_BYTE_LEN);
    }

    (void)get_rand_fast((uint8_t *)h2rf_para, sizeof(h2rf_para));
    (void)get_rand_fast((uint8_t *)el_2, sizeof(el_2));
    (void)get_rand_fast((uint8_t *)h_2, sizeof(h_2));
    (void)get_rand_fast((uint8_t *)t, sizeof(t));

    return ret;
}


/* function: SM9 sign s check input
 * parameters:
 *     msg ------------------------ input, message to be signed
 *     msg_bytes ------------------ input, bytes length of message
 *     fp12g ---------------------- input, the value of e(P1, Ppub_s), if set to null, it will be calculated within the function
 *     Ppub_s --------------------- input, KGC's master public key
 *     dsA ------------------------ input, signer's private key
 *     h -------------------------- input, partial signature result h
 *     S -------------------------- input, partial signature result S
 * return: SM9_SUCCESS_S(success), SM9_ERROR_S(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_sign_s_check_input(const uint8_t *msg, uint32_t msg_bytes, const uint8_t *fp12g, 
        const uint8_t Ppub_s[128], const uint8_t dsA[64], const uint8_t h[32], 
        const uint8_t S[65])
{
    uint32_t ret = PKE_SUCCESS;

    if((NULL == msg) || (NULL == dsA) || (NULL == h) || (NULL == S))
    {
        ret = SM9_ERROR_S;
    }
    else if((NULL == fp12g) && (NULL == Ppub_s))
    {
        ret = SM9_ERROR_S;
    }
    else if(0u == msg_bytes)
    {
        ret = SM9_ERROR_S;
    }
    else
    {
        //nothing to do, just for static analysis.
    }

    return ret;
}


/* function: SM9 sign s internal step 2
 * parameters:
 *     curve ---------------------- input, pointer to pointer of eccp_curve_st struct
 *     ctx ------------------------ input, pointer to eccp_sec_ctx_t struct
 *     count ---------------------- input&output, pointer to volatile uint32_t counter
 *     curve_a -------------------- input&output, pointer to pointer of uint32_t for curve parameter a
 *     curve_b -------------------- input&output, pointer to pointer of uint32_t for curve parameter b
 *     curve_p -------------------- input&output, pointer to pointer of uint32_t for curve parameter p
 *     curve_p_h ------------------ input&output, pointer to pointer of uint32_t for curve parameter p's hash
 *     eccp_curve_crc16 ------------ input, CRC16 checksum of the curve parameters
 * return: SM9_SUCCESS_S(success), SM9_ERROR_S(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_sign_s_internal_step_2(const eccp_curve_st **curve, eccp_sec_ctx_t *ctx, 
    volatile uint32_t *count, uint32_t **curve_a, uint32_t **curve_b, uint32_t **curve_p, uint32_t **curve_p_h, uint16_t eccp_curve_crc16)
{
    uint32_t ret = PKE_SUCCESS;

    (void)counter_add_one(count);

    //init sm9 curve
    *curve = eccp_curve_init(ctx, sm9_curve);
    if(NULL == curve)
    {
        ret = SM9_ERROR_S;
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //check crc16 of sm9 paras
        if(0u != ecc_crc16_check(*curve, eccp_curve_crc16))
        {
            ret = SM9_ERROR_S;
        }
        else
        {
            //actually the following pointers are the same as the corresponding fields of curve
            *curve_p   = &(ctx->eccp_curve_mem[0]);
            *curve_p_h = &(ctx->eccp_curve_mem[SM9_BASE_WORD_LEN]);
            *curve_a   = &(ctx->eccp_curve_mem[SM9_BASE_WORD_LEN<<1]);
            *curve_b   = &(ctx->eccp_curve_mem[SM9_BASE_WORD_LEN*3u]);

            (void)counter_add_one(count);
        }
    }
    else
    {}

    return ret;
}


/* function: SM9 sign s internal step 3
 * parameters:
 *     count ---------------------- input&output, pointer to volatile uint32_t counter
 *     fp12g ---------------------- input, the value of e(P1, Ppub_s), if set to null, it will be calculated within the function
 *     dsA ------------------------ input, signer's private key
 *     tmp_fp12g ------------------ output, temporary buffer for fp12g
 *     Px ------------------------- output, x coordinate of the signer's public key
 *     Py ------------------------- output, y coordinate of the signer's public key
 *     curve_a -------------------- input&output, pointer to pointer of uint32_t for curve parameter a
 *     curve_b -------------------- input&output, pointer to pointer of uint32_t for curve parameter b
 *     curve ---------------------- input, pointer to eccp_curve_st struct
 *     crc_g ---------------------- output, CRC16 checksum of fp12g
 *     Qx ------------------------- output, x coordinate of the KGC's master public key
 *     Qy ------------------------- output, y coordinate of the KGC's master public key
 *     Ppub_s --------------------- input, KGC's master public key
 * return: SM9_SUCCESS_S(success), SM9_ERROR_S(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_sign_s_internal_step_3(volatile uint32_t *count, const uint8_t *fp12g, 
        const uint8_t dsA[64], uint32_t *tmp_fp12g, uint32_t Px[8], uint32_t Py[8], 
        uint32_t *curve_a, uint32_t *curve_b, const eccp_curve_st *curve, uint16_t *crc_g,
        uint32_t *Qx, uint32_t *Qy, const uint8_t Ppub_s[128])
{
    uint32_t ret = PKE_SUCCESS;

    //check dsA
    (void)sm9_fp_eccp_point_u8big_2_u32little(dsA, Px, Py);
    if(PKE_SUCCESS != eccp_check_point(curve, Px, Py))
    {
        ret = SM9_ERROR_S;
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //copy back sm9 curve paras in PKE RAM that not erased by hardware
        uint32_copy_8_words(curve_a, (uint32_t *)(rPKE_B(4u,SM9_STEPS)));
        uint32_copy_8_words(curve_b, (uint32_t *)(rPKE_A(4u,SM9_STEPS)));
#if 0   //since p and p_h will be used by the following hardware calculation.
        uint32_copy_8_words(curve_p, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));
        uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM9_STEPS)));
#endif

        (void)counter_add_one(count);

        if(NULL == fp12g)
        {
            //check Ppub_s
            (void)sm9_fp2_eccp_point_u8big_2_u32little(Ppub_s, Qx, Qy);
            if(PKE_SUCCESS != sm9_fp2_check_point_internal(curve, Qx, Qy))
            {
                ret = SM9_ERROR_S;
            }
            else
            {}

            if(PKE_SUCCESS== ret)
            {
                (void)counter_add_one(count);

                ret = sm9_pairing(curve->eccp_Gx, curve->eccp_Gy, Qx, Qy, tmp_fp12g);
                if(PKE_SUCCESS == ret)
                {
                    *crc_g = crc16_calc((uint8_t *)tmp_fp12g, 32u*12u, (uint16_t)0xFFFF);
                }
                else
                {
                    ret = SM9_ERROR_S;
                }
            }
            else
            {}
        }
        else
        {
            *crc_g = crc16_calc(fp12g, 32u*12u, (uint16_t)0xFFFF);

            (void)counter_add_one(count);

            (void)sm9_fp12_u8big_2_u32little(fp12g, (uint32_t *)tmp_fp12g);
        }

    }
    else
    {}

    return ret;
}


/* function: SM9 sign s internal step 4
 * parameters:
 *     msg ------------------------ input, message to be signed
 *     msg_bytes ------------------ input, bytes length of message
 *     h -------------------------- output, hash value of the message
 *     count ---------------------- input&output, pointer to volatile uint32_t counter
 *     r -------------------------- input, random integer r in signing
 *     curve_p -------------------- input&output, pointer to pointer of uint32_t for curve parameter p
 *     curve_p_h ------------------ input&output, pointer to pointer of uint32_t for curve parameter p's hash
 *     tmp_fp12g ------------------ input, temporary buffer for fp12g
 *     Qx ------------------------- output, x coordinate of the signer's public key
 *     Qy ------------------------- output, y coordinate of the signer's public key
 *     tmp_r ---------------------- output, temporary buffer for r
 *     el ------------------------- output, el = (r-h) mod n
 *     ctx ------------------------ input, pointer to eccp_sec_ctx_t struct
 * return: SM9_SUCCESS_S(success), SM9_ERROR_S(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_sign_s_internal_step_4(const uint8_t *msg, uint32_t msg_bytes, uint8_t h[32], 
        volatile uint32_t *count, const uint8_t r[32], uint32_t *curve_p, uint32_t *curve_p_h,
        const uint32_t tmp_fp12g[8u*12u], uint32_t Qx[16], const uint32_t Qy[16], uint32_t tmp_r[8], uint32_t el[8], eccp_sec_ctx_t ctx[1])
{
    uint32_t ret;

    //copy back sm9 curve paras
    uint32_copy_8_words(curve_p, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));
    uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM9_STEPS)));

    if(NULL == r)
    {
        (void)counter_add_one(count);

        do {
            ret = get_rand((uint8_t *)tmp_r, 32u);
            if(TRNG_SUCCESS != ret)
            {
                ret = SM9_ERROR_S;
                break;
            }
            else
            {}

            //backup r
            uint32_copy_8_words(Qx, tmp_r);

            ret = sm9_sign_with_r_s(ctx, tmp_fp12g, tmp_r, msg, msg_bytes, el, h);
        } while((SM9_ZERO_ALL == ret) || (SM9_INTEGER_TOO_BIG == ret));

        if(SM9_SUCCESS_S == ret)
        {
            //success while sm9_sign_with_r_s() returns success and r not corrupted
            if(0u == uint32_cmp_sec(Qx, tmp_r, SM9_BASE_WORD_LEN, (uint8_t)(tmp_r[0]*Qy[0])))
            {
                (void)counter_add_one(count);
            }
            else
            {
                ret = SM9_ERROR_S;
            }
        }
        else
        {}
    }
    else
    {
        u8big_to_u32little_256bits(r, tmp_r);

        (void)counter_add_one(count);

        ret = sm9_sign_with_r_s(ctx, tmp_fp12g, tmp_r, msg, msg_bytes, el, h);
        if(SM9_SUCCESS_S == ret)
        {
            (void)counter_add_one(count);

            //check tmp_r
            u8big_to_u32little_256bits(r, Qx);
            if(0u != uint32_cmp_sec(Qx, tmp_r, SM9_BASE_WORD_LEN, (uint8_t)(tmp_r[0]*Qy[0])))
            {
                ret = SM9_ERROR_S;
            }
            else
            {}
        }
        else
        {
            ret = SM9_ERROR_S;
        }
    }

    if(SM9_SUCCESS_S == ret)
    {
        (void)counter_add_one(count);
        ret = PKE_SUCCESS;
    }
    else
    {}

    return ret;
}


/* function: SM9 sign s internal step 5
 * parameters:
 *     Px ------------------------- input&output, x coordinate of the signer's public key
 *     Py ------------------------- input&output, y coordinate of the signer's public key
 *     Qx ------------------------- input&output, x coordinate of the KGC's master public key
 *     Qy ------------------------- input&output, y coordinate of the KGC's master public key
 *     tmp_r ---------------------- input&output, temporary buffer for r
 *     el ------------------------- input, el = (r-h) mod n
 *     count ---------------------- input&output, pointer to volatile uint32_t counter
 *     curve_a -------------------- input&output, pointer to pointer of uint32_t for curve parameter a
 *     curve_b -------------------- input&output, pointer to pointer of uint32_t for curve parameter b
 *     curve ---------------------- input, pointer to eccp_curve_st struct
 * return: SM9_SUCCESS_S(success), SM9_ERROR_S(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_sign_s_internal_step_5(uint32_t Px[8], 
    uint32_t Py[8],
    uint32_t Qx[16], uint32_t Qy[16],
    uint32_t tmp_r[8],
    const uint32_t el[8],
    volatile uint32_t *count,
    uint32_t *curve_a, uint32_t *curve_b,
    const eccp_curve_st *curve)
{
    uint32_t ret;

    /********** calc S = [el]dsA twice and cmpare 3 times **********/
    //check input point dsA again
    ret = eccp_check_point(curve, Px, Py);
    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //copy back sm9 curve paras in PKE RAM that not erased by hardware
        uint32_copy_8_words(curve_a, (uint32_t *)(rPKE_B(4u,SM9_STEPS)));
        uint32_copy_8_words(curve_b, (uint32_t *)(rPKE_A(4u,SM9_STEPS)));
#if 0   //since p and p_h will be used by the following hardware calculation.
        uint32_copy_8_words(curve_p, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));
        uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM9_STEPS)));
#endif
        (void)counter_add_one(count);

        //calc S = [el]dsA
        ret = eccp_pointMul_sec_safe_internal(curve, el, Px, Py, Qx, Qy);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //copy back sm9 curve paras in PKE RAM that not erased by hardware
        uint32_copy_8_words(curve_a, (uint32_t *)(rPKE_B(4u,SM9_STEPS)));
#if 0   //since p and p_h will be used by the following hardware calculation.
        uint32_copy_8_words(curve_p, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));
        uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM9_STEPS)));
#endif

        (void)counter_add_one(count);

        //get rand for sleeping
        ret = get_rand((uint8_t *)tmp_r, 1u<<2);
        if(TRNG_SUCCESS != ret)
        {
            ret = SM9_ERROR_S;
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
        //sleep between the two repeated calculations
        uint32_sleep(tmp_r[0] & 0x0Fu, (uint8_t)(tmp_r[0] >> 4u));

        (void)counter_add_one(count);

        //calc S = [el]dsA again
        ret = eccp_pointMul_sec_safe_internal(curve, el, Px, Py, Px, Py);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //copy back sm9 curve paras in PKE RAM that not erased by hardware
        uint32_copy_8_words(curve_a, (uint32_t *)(rPKE_B(4u,SM9_STEPS)));
#if 0   //since p and p_h will be used by the following hardware calculation.
        uint32_copy_8_words(curve_p, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));
        uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM9_STEPS)));
#endif

        (void)counter_add_one(count);

        //check output point
        ret = eccp_check_point_internal(curve, Px, Py);
    }
    else
    {}

    return ret;
}


/* function: SM9 sign s internal step 6
 * parameters:
 *     right_hand_operand_ret1 ---- output, result of comparison operation
 *     Px ------------------------- input, x coordinate of the signer's public key
 *     Py ------------------------- input, y coordinate of the signer's public key
 *     Qx ------------------------- input, x coordinate of the KGC's master public key
 *     Qy ------------------------- input, y coordinate of the KGC's master public key
 *     tmp_r ---------------------- input, temporary buffer for random number
 *     count ---------------------- input&output, pointer to volatile uint32_t counter
 *     curve_a -------------------- input&output, pointer to pointer of uint32_t for curve parameter a
 *     curve_b -------------------- input&output, pointer to pointer of uint32_t for curve parameter b
 *     curve_p -------------------- input&output, pointer to pointer of uint32_t for curve parameter p
 *     curve_p_h ------------------ input&output, pointer to pointer of uint32_t for curve parameter p's hash
 * return: SM9_SUCCESS_S(success), SM9_ERROR_S(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_sign_s_internal_step_6(uint32_t *right_hand_operand_ret1,
    const uint32_t Px[8], const uint32_t Py[8],
    const uint32_t Qx[16], const uint32_t Qy[16],
    uint32_t tmp_r[8],
    volatile uint32_t *count,
    uint32_t *curve_a, uint32_t *curve_b, uint32_t *curve_p, uint32_t*curve_p_h)
{
    uint32_t ret = PKE_SUCCESS;

    (void)counter_add_one(count);

    //copy back sm9 curve paras in PKE RAM that not erased by hardware
    uint32_copy_8_words(curve_a, (uint32_t *)(rPKE_B(4u,SM9_STEPS)));
    uint32_copy_8_words(curve_b, (uint32_t *)(rPKE_A(4u,SM9_STEPS)));
    uint32_copy_8_words(curve_p, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));
    uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM9_STEPS)));

    (void)counter_add_one(count);

    // compare 1
    tmp_r[0] >>= 8;
    uint32_sleep(tmp_r[0] & 0x0Fu, (uint8_t)(tmp_r[0] >> 4));
    *right_hand_operand_ret1 = uint32_cmp_sec(Py, Qy, 8u, (uint8_t)(tmp_r[0] >> 6));
    if((0u != uint32_cmp_sec(Px, Qx, 8u, (uint8_t)(tmp_r[0] >> 5))) || 
        (0u != (*right_hand_operand_ret1)))
    {
        ret = SM9_ERROR_S;
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        // compare 2
        tmp_r[0] >>= 8;
        *right_hand_operand_ret1 = uint32_cmp_sec(Py, Qy, 8u, (uint8_t)(tmp_r[0] >> 6));
        uint32_sleep(tmp_r[0] & 0x0Fu, (uint8_t)(tmp_r[0] >> 4));
        if((0u != uint32_cmp_sec(Px, Qx, 8u, (uint8_t)(tmp_r[0] >> 5))) || 
            (0u != (*right_hand_operand_ret1)))
        {
            ret = SM9_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        // compare 3
        tmp_r[0] >>= 8;
        uint32_sleep(tmp_r[0] & 0x0Fu, (uint8_t)(tmp_r[0] >> 4));
        *right_hand_operand_ret1 = uint32_cmp_sec(Py, Qy, 8u, (uint8_t)(tmp_r[0] >> 6));
        if((0u != uint32_cmp_sec(Px, Qx, 8u, (uint8_t)(tmp_r[0] >> 5))) || 
            (0u != (*right_hand_operand_ret1)))
        {
            ret = SM9_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: SM9 sign s internal step 7
 * parameters:
 *     fp12g ---------------------- input, the value of e(P1, Ppub_s), if set to null, it will be calculated within the function
 *     S -------------------------- output, partial signature result S
 *     right_hand_operand_ret1 ---- input&output, pointer to uint32_t for comparison result
 *     right_hand_operand_ret2 ---- input&output, pointer to uint32_t for comparison result
 *     tmp_fp12g ------------------ input, temporary buffer for fp12g
 *     Px ------------------------- input, x coordinate of the signer's public key
 *     Py ------------------------- input, y coordinate of the signer's public key
 *     count ---------------------- input&output, pointer to volatile uint32_t counter
 *     curve ---------------------- input, pointer to eccp_curve_st struct
 *     eccp_curve_crc16 ------------ input, CRC16 checksum of the curve parameters
 *     crc_g ---------------------- input, CRC16 checksum of fp12g
 *     crc_g2 --------------------- output, CRC16 checksum of fp12g (calculated within the function)
 * return: SM9_SUCCESS_S(success), SM9_ERROR_S(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_sign_s_internal_step_7(const uint8_t *fp12g, 
    uint8_t S[65],
    uint32_t *right_hand_operand_ret1,
    uint32_t *right_hand_operand_ret2,
    const uint32_t tmp_fp12g[8u*12u],
    const uint32_t Px[8], const uint32_t Py[8],
    volatile uint32_t *count,
    const eccp_curve_st *curve,
    const uint16_t *eccp_curve_crc16,
    const uint16_t *crc_g,
    uint16_t *crc_g2)
{
    uint32_t ret = PKE_SUCCESS;

    (void)counter_add_one(count);

    //check crc16 of sm9 paras
    if(0U != ecc_crc16_check(curve, *eccp_curve_crc16))
    {
        ret = SM9_ERROR_S;
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        // calc crc of g
        if(NULL == fp12g)
        {
            *crc_g2 = crc16_calc((const uint8_t *)tmp_fp12g, 32u*12u, (uint16_t)0xFFFF);
        }
        else
        {
            *crc_g2 = crc16_calc(fp12g, 32u*12u, (uint16_t)0xFFFF);
        }

        *right_hand_operand_ret1 = (uint32_t)(*crc_g) ^ (uint32_t)(*crc_g2);
        *right_hand_operand_ret2 = (uint32_t)(*crc_g) - (uint32_t)(*crc_g2);
        if((*crc_g != (*crc_g2)) || (((uint16_t)0) != (*right_hand_operand_ret1)) || (((uint16_t)0) != (*right_hand_operand_ret2)))
        {
            ret = SM9_ERROR_S;
        }
        else
        {}
    }
    else
    {}
    
    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        if(*count != (SM9_SEC_SIGN_COUNTER2 + 0x17U))
        {
            ret = SM9_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //output S part
        S[0] = POINT_UNCOMPRESSED;
        (void)sm9_fp_eccp_point_u32little_2_u8big(Px, Py, &S[1]);

        ret = SM9_SUCCESS_S;
    }
    else
    {}

    return ret;
}


/* function: SM9 sign(secure version)
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
 * return: SM9_SUCCESS_S(success), SM9_ERROR_S(error)
 * caution:
 */
uint32_t sm9_sign_s(const uint8_t *msg, uint32_t msg_bytes, const uint8_t *fp12g, 
        const uint8_t Ppub_s[128], const uint8_t dsA[64], const uint8_t r[32], uint8_t h[32], 
        uint8_t S[65])
{
    uint32_t ret = PKE_SUCCESS;
    uint32_t right_hand_operand_ret1;
    uint32_t right_hand_operand_ret2;
    uint32_t tmp_fp12g[8u*12u];
    uint32_t Px[8], Py[8];
    uint32_t Qx[16], Qy[16];
    uint32_t tmp_r[8];
    uint32_t el[8];
    volatile uint32_t count = SM9_SEC_SIGN_COUNTER2;

    uint32_t *curve_a, *curve_b, *curve_p, *curve_p_h;
    const eccp_curve_st *curve;
    eccp_sec_ctx_t ctx[1];
    uint16_t eccp_curve_crc16 = (uint16_t)0;
    uint16_t crc_g;
    uint16_t crc_g2;

    ret = sm9_sign_s_check_input(msg, msg_bytes, fp12g, Ppub_s, dsA, h, S);
    if(PKE_SUCCESS == ret)
    {
        ret = sm9_sign_s_internal_step_2(&curve, ctx, &count, &curve_a, &curve_b, &curve_p, &curve_p_h, eccp_curve_crc16);
    }

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_sign_s_internal_step_3(&count, fp12g, dsA, tmp_fp12g, Px, Py, curve_a, curve_b, curve, 
            &crc_g, Qx, Qy, Ppub_s);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_sign_s_internal_step_4(msg, msg_bytes, h, &count, r, curve_p, curve_p_h, tmp_fp12g, Qx, 
            Qy, tmp_r, el, ctx);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_sign_s_internal_step_5(Px, Py, Qx, Qy, tmp_r, el, &count, curve_a, curve_b, curve);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_sign_s_internal_step_6(&right_hand_operand_ret1, Px, Py, Qx, Qy, tmp_r, &count, curve_a, 
            curve_b, curve_p, curve_p_h);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_sign_s_internal_step_7(fp12g, S, &right_hand_operand_ret1, &right_hand_operand_ret2, 
            tmp_fp12g, Px, Py, &count, curve, &eccp_curve_crc16, &crc_g, &crc_g2);
    }
    else
    {}

    if(SM9_SUCCESS_S != ret)
    {
        (void)get_rand_fast((uint8_t *)h, SM9_BASE_BYTE_LEN);
        (void)get_rand_fast((uint8_t *)S, (SM9_BASE_BYTE_LEN<<1)+1u);
    }
    else
    {}

    (void)get_rand_fast((uint8_t *)tmp_fp12g, sizeof(tmp_fp12g));
    (void)get_rand_fast((uint8_t *)Px, sizeof(Px));
    (void)get_rand_fast((uint8_t *)Py, sizeof(Py));
    (void)get_rand_fast((uint8_t *)Qx, sizeof(Qx));
    (void)get_rand_fast((uint8_t *)Qy, sizeof(Qy));
    (void)get_rand_fast((uint8_t *)tmp_r, sizeof(tmp_r));
    (void)get_rand_fast((uint8_t *)el, sizeof(el));
    (void)get_rand_fast((uint8_t *)ctx, sizeof(eccp_sec_ctx_t));
    curve = NULL;
    crc_g =(uint16_t)0; 
    crc_g2 = (uint16_t)0;

    return ret;
}


/* function: SM9 verify s internal step 1
 * parameters:
 *     msg ------------------------ input, message to be verified
 *     IDA ------------------------ input, identity of the signer
 *     IDA_bytes ------------------ input, bytes length of identity
 *     Ppub_s --------------------- input, KGC's master public key
 *     h -------------------------- input, partial signature result h
 *     S -------------------------- input, partial signature result S
 *     Qx ------------------------- output, x coordinate of the signer's public key
 *     Qy ------------------------- output, y coordinate of the signer's public key
 *     SigH ----------------------- output, hash value of the message
 *     SigSx ---------------------- output, x coordinate of the signature point
 *     SigSy ---------------------- output, y coordinate of the signature point
 *     curve_a -------------------- output, pointer to pointer of uint32_t for curve parameter a
 *     curve_b -------------------- output, pointer to pointer of uint32_t for curve parameter b
 *     curve_p -------------------- output, pointer to pointer of uint32_t for curve parameter p
 *     curve_p_h ------------------ output, pointer to pointer of uint32_t for curve parameter p's hash
 *     curve ---------------------- output, pointer to eccp_curve_st struct
 *     ctx ------------------------ output, pointer to eccp_sec_ctx_t struct
 *     eccp_curve_crc16 ------------ input, CRC16 checksum of the curve parameters
 * return: SM9_SUCCESS_S(success), SM9_ERROR_S(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_verify_s_internal_step_1(
    const uint8_t *msg, 
    const uint8_t *IDA, 
    uint32_t IDA_bytes, 
    const uint8_t Ppub_s[128], 
    const uint8_t h[32], 
    const uint8_t S[65],
    uint32_t *Qx,
    uint32_t *Qy,
    uint32_t SigH[8],
    uint32_t SigSx[8], uint32_t SigSy[8],
    uint32_t **curve_a, 
    uint32_t **curve_b,
    uint32_t **curve_p,
    uint32_t **curve_p_h,
    const eccp_curve_st **curve,
    eccp_sec_ctx_t ctx[1],
    const uint16_t *eccp_curve_crc16)
{
    uint32_t right_hand_operand_ret;
    uint32_t ret = PKE_SUCCESS;

    if((NULL == msg) || (NULL == IDA) || (NULL == Ppub_s) || (NULL == h) || (NULL == S))
    {
        ret = SM9_ERROR_S;
    }
    else if((0u == IDA_bytes) || (POINT_UNCOMPRESSED != S[0]))
    {
        ret = SM9_ERROR_S;
    }
    else
    {
        //nothing to do, just for static analysis.
    }

    if(PKE_SUCCESS == ret)
    {
        //init sm9 curve
        *curve = eccp_curve_init(ctx, sm9_curve);
        if(NULL == curve)
        {
            ret = SM9_ERROR_S;
        }
        else
        {
            right_hand_operand_ret = ecc_crc16_check(*curve, *eccp_curve_crc16);
        }

        if((PKE_SUCCESS == ret) && (0u != right_hand_operand_ret))
        {
            //check crc16 of sm9 paras
            ret = SM9_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //actually the following pointers are the same as the corresponding fields of curve
        *curve_p   = &(ctx->eccp_curve_mem[0]);
        *curve_p_h = &(ctx->eccp_curve_mem[SM9_BASE_WORD_LEN]);
        *curve_a   = &(ctx->eccp_curve_mem[SM9_BASE_WORD_LEN<<1]);
        *curve_b   = &(ctx->eccp_curve_mem[SM9_BASE_WORD_LEN*3u]);

        //check h in [1, n-1]
        u8big_to_u32little_256bits(h, SigH);
        ret = uint32_integer_check_sec(SigH, (*curve)->eccp_n, SM9_BASE_WORD_LEN, SM9_ZERO_ALL, SM9_INTEGER_TOO_BIG, PKE_SUCCESS);
        if(PKE_SUCCESS != ret)
        {
            ret = SM9_ERROR_S;
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            //check S on curve
            (void)sm9_fp_eccp_point_u8big_2_u32little(&S[1], SigSx, SigSy);
            ret = eccp_check_point(*curve, SigSx, SigSy);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            //copy back sm9 curve paras in PKE RAM that not erased by hardware
            uint32_copy_8_words(*curve_a, (uint32_t *)(rPKE_B(4u,SM9_STEPS)));
            uint32_copy_8_words(*curve_b, (uint32_t *)(rPKE_A(4u,SM9_STEPS)));
#if 0   //since p and p_h will be used by the following hardware calculation.
            uint32_copy_8_words(curve_p, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));
            uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM9_STEPS)));
#endif

            //check Ppub_s
            (void)sm9_fp2_eccp_point_u8big_2_u32little(Ppub_s, Qx, Qy);
			ret = sm9_fp2_check_point_internal(*curve, Qx, Qy);
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: SM9 verify s internal step 2
 * parameters:
 *     fp12g ---------------------- input, the value of e(P1, Ppub_s), if set to null, it will be calculated within the function
 *     tmp_fp12g ------------------ output, temporary buffer for fp12g
 *     fp12g_1 -------------------- output, result of fp12g exponentiation
 *     Qx ------------------------- input, x coordinate of the signer's public key
 *     Qy ------------------------- input, y coordinate of the signer's public key
 *     SigH ----------------------- input, hash value of the message
 *     curve ---------------------- input, pointer to eccp_curve_st struct
 *     crc_g ---------------------- output, CRC16 checksum of fp12g
 * return: SM9_SUCCESS_S(success), SM9_ERROR_S(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_verify_s_internal_step_2(
    const uint8_t *fp12g,
    uint32_t tmp_fp12g[8u*12u], 
    uint32_t fp12g_1[8u*12u],
    const uint32_t *Qx,
    const uint32_t *Qy,
    const uint32_t SigH[8],
    const eccp_curve_st *curve,
    uint16_t *crc_g)
{
    uint32_t ret = PKE_SUCCESS;

    if(NULL == fp12g)
    {
        ret = sm9_pairing(curve->eccp_Gx, curve->eccp_Gy, Qx, Qy, tmp_fp12g);

        *crc_g = crc16_calc((uint8_t *)tmp_fp12g, 32u*12u, (uint16_t)0xFFFF);
    }
    else
    {
        *crc_g = crc16_calc(fp12g, 32u*12u, (uint16_t)0xFFFF);

        (void)sm9_fp12_u8big_2_u32little(fp12g, (uint32_t *)tmp_fp12g);
    }

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_fp12_exp_s(tmp_fp12g, SigH, fp12g_1);
        if(PKE_SUCCESS != ret)
        {
            ret = SM9_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: SM9 verify s internal step 3
 * parameters:
 *     msg ------------------------ input, message to be verified
 *     msg_bytes ------------------ input, bytes length of message
 *     IDA ------------------------ input, identity of the signer
 *     IDA_bytes ------------------ input, bytes length of identity
 *     tmp_fp12g_1 ---------------- output, temporary buffer for fp12g
 *     fp12g_1 -------------------- input, result of fp12g exponentiation
 *     tmp ------------------------ output, temporary buffer for h2rf_para
 *     h2rf_para ------------------ output, parameter for h2rf function
 *     Qx ------------------------- output, x coordinate of the signer's public key
 *     Qy ------------------------- input, y coordinate of the signer's public key
 *     TP2x ----------------------- output, x coordinate of the temporary point TP2
 *     TP2y ----------------------- output, y coordinate of the temporary point TP2
 *     SigH ----------------------- input, hash value of the message
 *     SigSx ---------------------- input, x coordinate of the signature point
 *     SigSy ---------------------- input, y coordinate of the signature point
 *     curve_b -------------------- output, pointer to pointer of uint32_t for curve parameter b
 *     curve_p -------------------- output, pointer to pointer of uint32_t for curve parameter p
 *     curve_p_h ------------------ output, pointer to pointer of uint32_t for curve parameter p's hash
 *     curve ---------------------- input, pointer to eccp_curve_st struct
 *     hid_value ------------------ input, hidden value for h1 function
 * return: SM9_SUCCESS_S(success), SM9_ERROR_S(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_verify_s_internal_step_3(
    const uint8_t *msg, 
    uint32_t msg_bytes, 
    const uint8_t *IDA, 
    uint32_t IDA_bytes, 
    uint32_t tmp_fp12g_1[8u*12u], 
    const uint32_t fp12g_1[8u*12u],
    uint32_t tmp[8],
    uint8_t *h2rf_para,
    uint32_t *Qx,
    const uint32_t *Qy,
    uint32_t *TP2x,
    uint32_t *TP2y,
    const uint32_t SigH[8],
    const uint32_t SigSx[8], 
    const uint32_t SigSy[8],
    uint32_t *curve_b, 
    uint32_t *curve_p, 
    uint32_t *curve_p_h,
    const eccp_curve_st *curve,
    uint8_t hid_value
)
{
    uint32_t ret = PKE_SUCCESS;
    uint8_t hid_value_bak = hid_value;

    //copy back sm9 curve paras in PKE RAM that not erased by hardware
    uint32_copy_8_words(curve_p, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));
    uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM9_STEPS)));

    //check h
    if((int32_t)0 != uint32_BigNumCmp_sec(SigH, SM9_BASE_WORD_LEN, (uint32_t *)rPKE_A(1u,SM9_STEPS), SM9_BASE_WORD_LEN))
    {
        ret = SM9_ERROR_S;
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //tmp := h1 := H1(IDA||hid, N)
        ret = sm9_h1_h2((uint8_t)1, IDA, IDA_bytes, &hid_value_bak, 1u, tmp);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //TP2 = [h1]P2
        ret = sm9_fp2_pointMul_s(curve, tmp, sm9_G2_Px0_Px1, sm9_G2_Py0_Py1, TP2x, TP2y);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //copy back sm9 curve paras in PKE RAM that not erased by hardware
        uint32_copy_8_words(curve_b, (uint32_t *)(rPKE_B(3u,SM9_STEPS)));
#if 0   //since p and p_h will be used by the following hardware calculation.
        uint32_copy_8_words(curve_p, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));
        uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM9_STEPS)));
#endif

        //TP2 = [h1]P2 + Ppub_s
        ret = sm9_fp2_pointAdd_internal(TP2x, TP2y, Qx, Qy, TP2x, TP2y);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //tmp_fp12g := u := e(S, TP2)
        ret = sm9_pairing(SigSx, SigSy, TP2x, TP2y, tmp_fp12g_1);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //h2rf_para = w = u*t
        ret = sm9_fp12_mul(fp12g_1, tmp_fp12g_1, NULL);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //copy back sm9 curve paras in PKE RAM that not erased by hardware
        uint32_copy_8_words(curve_p, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));
        uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM9_STEPS)));

        (void)sm9_fp12_in_pke_ram_u32little_2_u8big(h2rf_para);

        //tmp := h2 := H2(M||w, N)
        ret = sm9_h1_h2((uint8_t)2, msg, msg_bytes, h2rf_para, 32u*12u, tmp);
    }
    else
    {}


    if(PKE_SUCCESS == ret)
    {
        //get rand for sleeping
        ret = get_rand((uint8_t *)Qx, 1u<<2);
        if(TRNG_SUCCESS == ret)
        {
            ret = PKE_SUCCESS;
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: SM9 verify s internal step 4
 * parameters:
 *     fp12g ---------------------- input, the value of e(P1, Ppub_s), if set to null, it will be calculated within the function
 *     right_hand_operand_ret1 ---- output, result of comparison operation
 *     right_hand_operand_ret2 ---- output, result of comparison operation
 *     tmp_fp12g ------------------ input, temporary buffer for fp12g
 *     tmp ------------------------ input, temporary buffer for h2rf_para
 *     Qx ------------------------- output, x coordinate of the signer's public key
 *     SigH ----------------------- input, hash value of the message
 *     curve ---------------------- input, pointer to eccp_curve_st struct
 *     eccp_curve_crc16 ------------ input, CRC16 checksum of the curve parameters
 *     crc_g ---------------------- input, CRC16 checksum of fp12g
 *     crc_g2 --------------------- output, CRC16 checksum of fp12g (calculated within the function)
 * return: SM9_SUCCESS_S(success), SM9_ERROR_S(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_verify_s_internal_step_4(
    const uint8_t *fp12g,
    uint32_t *right_hand_operand_ret1,
    uint32_t *right_hand_operand_ret2,
    const uint32_t tmp_fp12g[8u*12u],
    const uint32_t tmp[8],
    uint32_t *Qx,
    const uint32_t SigH[8],
    const eccp_curve_st *curve,
    uint16_t eccp_curve_crc16,
    const uint16_t *crc_g,
    uint16_t *crc_g2
)
{
    uint32_t right_hand_operand_ret;
    uint32_t ret = PKE_SUCCESS;

    //compare h and h2 1
    uint32_sleep(Qx[0] & 0x0Fu, (uint8_t)(Qx[0] >> 4));
    right_hand_operand_ret = uint32_cmp_sec(SigH, tmp, SM9_BASE_WORD_LEN, (uint8_t)(Qx[0] >> 5));

    if((PKE_SUCCESS == ret) && (0u == right_hand_operand_ret))
    {
        //compare h and h2 2
        Qx[0] >>= 8;
        uint32_sleep(Qx[0] & 0x0Fu, (uint8_t)(Qx[0] >> 4));
        if(0u != uint32_cmp_sec(SigH, tmp, SM9_BASE_WORD_LEN, (uint8_t)(Qx[0] >> 5)))
        {
            ret = SM9_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //compare h and h2 3
        Qx[0] >>= 8;
        uint32_sleep(Qx[0] & 0x0Fu, (uint8_t)(Qx[0] >> 4));
        if(0u != uint32_cmp_sec(SigH, tmp, SM9_BASE_WORD_LEN, (uint8_t)(Qx[0] >> 5)))
        {
            ret = SM9_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //check crc16 of sm9 paras
        if(0u != ecc_crc16_check(curve, eccp_curve_crc16))
        {
            ret = SM9_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        // calc crc of g
        if(NULL == fp12g)
        {
            *crc_g2 = crc16_calc((const uint8_t *)tmp_fp12g, 32u*12u, (uint16_t)0xFFFF);
        }
        else
        {
            *crc_g2 = crc16_calc(fp12g, 32u*12u, (uint16_t)0xFFFF);
        }

        *right_hand_operand_ret1 = (uint32_t)(*crc_g) ^ (uint32_t)(*crc_g2);
        *right_hand_operand_ret2 = (uint32_t)(*crc_g) - (uint32_t)(*crc_g2);
        if((*crc_g != *crc_g2) || (((uint16_t)0) != (*right_hand_operand_ret1)) || (((uint16_t)0) != (*right_hand_operand_ret2)))
        {
            ret = SM9_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: SM9 verify the signature(secure version)
 * parameters:
 *     msg ------------------------ input, message to be verified
 *     msg_bytes ------------------ input, bytes length of message
 *     IDA ------------------------ input, identity of user A, user A is the signer
 *     IDA_bytes ------------------ input, bytes length of the IDA
 *     hid ------------------------ input, user private key generation function identity, published
 *                                         by KGC, default value is 0x01, one byte.
 *     fp12g ---------------------- input, the value of e(P1, Ppub_s), if set to null, it will be calculated within the function
 *     Ppub_s --------------------- input, KGC's master public key
 *     h -------------------------- input, partial signature result h
 *     S -------------------------- input, partial signature result S
 * return: SM9_SUCCESS_S(success), other(error)
 * caution:
 *     1.IDA represents only the signer.
 *
 */
uint32_t sm9_verify_s(const uint8_t *msg, uint32_t msg_bytes, const uint8_t *IDA, uint32_t IDA_bytes, 
        uint8_t hid, const uint8_t *fp12g, const uint8_t Ppub_s[128], const uint8_t h[32], 
        const uint8_t S[65])
{
    uint32_t ret = PKE_SUCCESS;
    uint32_t right_hand_operand_ret1;
    uint32_t right_hand_operand_ret2;
    uint32_t tmp_fp12g[8u*12u], tmp_fp12g_1[8u*12u], fp12g_1[8u*12u];
    uint32_t tmp[8];

#if 0
    uint8_t h2rf_para[32*12];
    uint32_t Qx[16], Qy[16];
    uint32_t TP2x[16], TP2y[16];
#else
#ifndef SUPPORT_STATIC_ANALYSIS
#define h2rf_para  ((uint8_t *)fp12g_1)
#define Qx         ((uint32_t *)tmp_fp12g_1)
#define Qy         (&(tmp_fp12g_1[2u*SM9_BASE_WORD_LEN]))
#define TP2x       (&(tmp_fp12g_1[4u*SM9_BASE_WORD_LEN]))
#define TP2y       (&(tmp_fp12g_1[6u*SM9_BASE_WORD_LEN]))
#else
uint8_t *h2rf_para = (uint8_t *)fp12g_1;
uint32_t *Qx       = (uint32_t *)tmp_fp12g_1;
uint32_t *Qy       = &(tmp_fp12g_1[2u*SM9_BASE_WORD_LEN]);
uint32_t *TP2x     = &(tmp_fp12g_1[4u*SM9_BASE_WORD_LEN]);
uint32_t *TP2y     = &(tmp_fp12g_1[6u*SM9_BASE_WORD_LEN]);
#endif
#endif

    uint32_t SigH[8];
    uint32_t SigSx[8], SigSy[8];

    uint32_t *curve_a, *curve_b, *curve_p, *curve_p_h;
    const eccp_curve_st *curve;
    eccp_sec_ctx_t ctx[1];
    uint16_t eccp_curve_crc16 = (uint16_t)0;
    uint16_t crc_g;
    uint16_t crc_g2;
    uint8_t hid_value = hid;

    ret = sm9_verify_s_internal_step_1(msg, IDA, IDA_bytes, Ppub_s, h, S, Qx, Qy,
        SigH,SigSx, SigSy, &curve_a, &curve_b, &curve_p, &curve_p_h, &curve, ctx, &eccp_curve_crc16);
    if(PKE_SUCCESS == ret)
    {
        ret = sm9_verify_s_internal_step_2(fp12g, tmp_fp12g, fp12g_1, Qx, Qy, SigH, curve, &crc_g);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_verify_s_internal_step_3(msg, msg_bytes, IDA, IDA_bytes, tmp_fp12g_1, fp12g_1,tmp,h2rf_para,Qx,Qy,
            TP2x,TP2y,SigH,SigSx, SigSy,curve_b, curve_p, curve_p_h,curve,hid_value);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_verify_s_internal_step_4(fp12g, &right_hand_operand_ret1, &right_hand_operand_ret2, 
            tmp_fp12g, tmp,Qx,SigH,curve,eccp_curve_crc16, &crc_g, &crc_g2);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = SM9_SUCCESS_S;
    }
    else
    {
        ret = SM9_ERROR_S;
    }

#ifndef SUPPORT_STATIC_ANALYSIS
#undef h2rf_para
#undef Qx
#undef Qy
#undef TP2x
#undef TP2y
#endif

    (void)get_rand_fast((uint8_t *)tmp_fp12g, sizeof(tmp_fp12g));
    (void)get_rand_fast((uint8_t *)tmp_fp12g_1, sizeof(tmp_fp12g_1));
    (void)get_rand_fast((uint8_t *)fp12g_1, sizeof(fp12g_1));
    (void)get_rand_fast((uint8_t *)tmp, sizeof(tmp));
    (void)get_rand_fast((uint8_t *)SigH, sizeof(SigH));
    (void)get_rand_fast((uint8_t *)SigSx, sizeof(SigSx));
    (void)get_rand_fast((uint8_t *)SigSy, sizeof(SigSy));
    (void)get_rand_fast((uint8_t *)ctx, sizeof(eccp_sec_ctx_t));
    crc_g =(uint16_t)0; 
    crc_g2 = (uint16_t)0;

    return ret;
}


/* function: SM9 key encapsulation with r check CRC (secure version)
 * parameters:
 *     key_bytes ------------------ input, bytes length of the output key
 *     key ------------------------ input, plaintext output key
 *     Cx ------------------------- input, x coordinate of the encapsulated cipher
 *     crc_k ---------------------- input, CRC16 checksum of the key
 * return: SM9_SUCCESS_S(success), SM9_ERROR_S(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_wrap_key_with_r_s_check_crc(
    uint32_t key_bytes, 
    const uint8_t *key, 
    uint32_t *Cx,
    uint16_t crc_k)
{
    uint16_t crc_k2;
    uint32_t ret = PKE_SUCCESS;

    crc_k2 = crc16_calc(key, key_bytes, (uint16_t)0xFFFF);

    if((crc_k != crc_k2) || ((uint16_t)0 != (crc_k ^ crc_k2)) || ((uint16_t)0 != (crc_k - crc_k2)))
    {
        ret = PKE_ERROR;
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //check key 3 times
        Cx[0] >>= 8;
        uint32_sleep(Cx[0] & 0x0Fu, (uint8_t)(Cx[0] >> 4));
        if(0u != uint8_BigNum_Check_Zero_sec(key, key_bytes))
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
        Cx[0] >>= 8;
        uint32_sleep(Cx[0] & 0x0Fu, (uint8_t)(Cx[0] >> 4));
        if(0u != uint8_BigNum_Check_Zero_sec(key, key_bytes))
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
        Cx[0] >>= 8;
        uint32_sleep(Cx[0] & 0x0Fu, (uint8_t)(Cx[0] >> 4));
        if(0u != uint8_BigNum_Check_Zero_sec(key, key_bytes))
        {
            ret = SM9_ZERO_ALL;
        }
        else
        {}
    }
    else
    {}

    crc_k2 = (uint16_t)0;

    return ret;
}


/* function: SM9 SM9 key encapsulation with r(secure version), get cipher C and output key
 * parameters:
 *     ctx ------------------------ input, eccp_sec_ctx_t struct pointer
 *     IDB ------------------------ input, identify of user B, user B is receiver of the cipher C
 *     IDB_bytes ------------------ input, bytes length of the IDB
 *     QBx ------------------------ input, x coordinate of QB = [H1(IDB||hid, N)]P1+Ppub_e
 *     QBy ------------------------ input, y coordinate of QB = [H1(IDB||hid, N)]P1+Ppub_e
 *     fg ------------------------- input, the value of e(P1, Ppub_e)
 *     r -------------------------- input, random integer r 
 *     C -------------------------- output, encapsulated cipher of key, 64 bytes
 *     key_bytes ------------------ input, bytes length of the output key
 *     key ------------------------ output, plaintext output key
 * return: SM9_SUCCESS_S(success), other(error)
 * caution:
 *     1. ctx must be initialized by eccp_curve_init() before calling this function.
 *     2. fg, QBx, QBy can not be modified
 */
FLAG_STATIC uint32_t sm9_wrap_key_with_r_s(eccp_sec_ctx_t *ctx, const uint8_t *IDB, 
        uint32_t IDB_bytes, const uint32_t *QBx, const uint32_t *QBy, const uint32_t *fg, 
        const uint32_t *r, uint8_t C[64], uint32_t key_bytes, uint8_t *key)
{
    uint32_t ret;
    uint32_t counter_buf[1] = {0u};
    const uint8_t *counter = (uint8_t *)counter_buf;
    uint32_t Cx[8], Cy[8];
    uint8_t h2rf_para[32u*12u];
    uint32_t *curve_a, *curve_p, *curve_p_h;
    const eccp_curve_st *curve;
    uint16_t crc_k;

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

    curve = ctx->curve;

    //actually the following pointers are the same as the corresponding fields of curve
    curve_p   = &(ctx->eccp_curve_mem[0]);
    curve_p_h = &(ctx->eccp_curve_mem[SM9_BASE_WORD_LEN]);
    curve_a   = &(ctx->eccp_curve_mem[SM9_BASE_WORD_LEN<<1]);

    //make sure r in [1, n-1]
    ret = uint32_integer_check_sec(r, sm9_curve->eccp_n, SM9_BASE_WORD_LEN, SM9_ZERO_ALL, SM9_INTEGER_TOO_BIG, PKE_SUCCESS);
    if(PKE_SUCCESS == ret)
    {
        //C = [r]QB
        ret = eccp_pointMul_sec(curve, r, QBx, QBy, Cx, Cy);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //copy back sm9 curve paras in PKE RAM that not erased by hardware
        uint32_copy_8_words(curve_a, (uint32_t *)(rPKE_B(4u,SM9_STEPS)));
#if 0   //since p and p_h will be used by the following hardware calculation.
        uint32_copy_8_words(curve_p, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));
        uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM9_STEPS)));
#endif

        (void)sm9_fp_eccp_point_u32little_2_u8big(Cx, Cy, C);

        //w = g^r
        ret = sm9_fp12_exp_s(fg, r, NULL);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //check r
        if((int32_t)0 != uint32_BigNumCmp_sec(r, SM9_BASE_WORD_LEN, (uint32_t *)rPKE_A(1u,SM9_STEPS), SM9_BASE_WORD_LEN))
        {
            ret = PKE_ERROR;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //copy back sm9 curve paras in PKE RAM that not erased by hardware
        uint32_copy_8_words(curve_p, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));
        uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM9_STEPS)));

        (void)sm9_fp12_in_pke_ram_u32little_2_u8big(h2rf_para);

        //key := KDF(C||w||IDB, key_bytes)
        ret = ansi_x9_63_kdf_node(HASH_SM3, digest_node, 4u, (uint8_t *)counter_buf, key, key_bytes, NULL, 0u);
        if(HASH_SUCCESS == ret)
        {
            crc_k = crc16_calc(key, key_bytes, (uint16_t)0xFFFF);

            //get rand for sleeping
            ret = get_rand((uint8_t *)Cx, 1u<<2);
            if(TRNG_SUCCESS == ret)
            {
                //sleep between the two repeated calculations
                uint32_sleep(Cx[0] & 0x0Fu, (uint8_t)(Cx[0] >> 4));

                //calc key = KDF(C||w||IDB, key_bytes) again
                counter_buf[0] = 0u;
                ((uint8_t *)counter_buf)[3] = (uint8_t)1u;
                ret = ansi_x9_63_kdf_node(HASH_SM3, digest_node, 4u, (uint8_t *)counter_buf, key, key_bytes, NULL, 0u);
            }
            else
            {}
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_wrap_key_with_r_s_check_crc(key_bytes, key, Cx, crc_k);
    }
    else
    {}

    if(PKE_ERROR == ret)
    {
        (void)get_rand_fast(C, 64u);
        (void)get_rand_fast(key, key_bytes);
    }
    else
    {}

    (void)get_rand_fast((uint8_t *)counter_buf, sizeof(counter_buf));
    (void)get_rand_fast((uint8_t *)Cx, sizeof(Cx));
    (void)get_rand_fast((uint8_t *)Cy, sizeof(Cy));
    (void)get_rand_fast((uint8_t *)h2rf_para, sizeof(h2rf_para));
    crc_k =  (uint16_t)0;

    return ret;
}


/* function: SM9 key encapsulation s internal step 1
 * parameters:
 *     fp12 ---------------------- output, buffer for storing the result of pairing operation
 *     curve_a -------------------- output, pointer to pointer of uint32_t for curve parameter a
 *     curve_b -------------------- output, pointer to pointer of uint32_t for curve parameter b
 *     curve_p -------------------- output, pointer to pointer of uint32_t for curve parameter p
 *     curve_p_h ------------------ output, pointer to pointer of uint32_t for curve parameter p's hash
 *     curve ---------------------- output, pointer to eccp_curve_st struct
 *     ctx ------------------------ output, pointer to eccp_sec_ctx_t struct
 *     eccp_curve_crc16 ------------ input, CRC16 checksum of the curve parameters
 *     Ppub_e --------------------- input, KGC's master public key
 * return: SM9_SUCCESS_S(success), SM9_ERROR_S(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_wrap_key_s_internal_step_1(
    uint32_t fp12[8u*12u],
    uint32_t **curve_a, 
    uint32_t **curve_b, 
    uint32_t **curve_p, 
    uint32_t **curve_p_h,
    const eccp_curve_st **curve,
    eccp_sec_ctx_t ctx[1],
    uint16_t eccp_curve_crc16 ,
    const uint8_t Ppub_e[64]
)
{
    uint32_t ret = PKE_SUCCESS;

    //init sm9 curve
    *curve = eccp_curve_init(ctx, sm9_curve);
    if(NULL == curve)
    {
        ret = SM9_ERROR_S;
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //check crc16 of sm9 paras
        if(0u != ecc_crc16_check(*curve, eccp_curve_crc16))
        {
            ret = SM9_ERROR_S;
        }
        else
        {}
    }
    else
    {}
    
    if(PKE_SUCCESS == ret)
    {
        *curve_p = &(ctx->eccp_curve_mem[0]);
        *curve_p_h = &(ctx->eccp_curve_mem[SM9_BASE_WORD_LEN]);
        *curve_a   = &(ctx->eccp_curve_mem[SM9_BASE_WORD_LEN<<1]);
        *curve_b   = &(ctx->eccp_curve_mem[SM9_BASE_WORD_LEN*3u]);

        //check Ppub_e
        (void)sm9_fp_eccp_point_u8big_2_u32little(Ppub_e, fp12, &fp12[SM9_BASE_WORD_LEN]);
        ret = eccp_check_point(sm9_curve, fp12, &fp12[SM9_BASE_WORD_LEN]);
    }   
    else
    {}     

    return ret;
}


/* function: SM9 key encapsulation s internal step 2
 * parameters:
 *     IDB ------------------------ input, identity of user B
 *     IDB_bytes ------------------ input, bytes length of the IDB
 *     fp12g ---------------------- input, the value of e(Ppub_e, P2), if set to null, it will be calculated within the function
 *     tmp ------------------------ output, temporary buffer for calculations
 *     QBx ------------------------- output, x coordinate of the point QB
 *     QBy ------------------------- output, y coordinate of the point QB
 *     fp12 ---------------------- output, buffer for storing the result of pairing operation
 *     curve_a -------------------- output, pointer to pointer of uint32_t for curve parameter a
 *     curve_b -------------------- output, pointer to pointer of uint32_t for curve parameter b
 *     curve_p -------------------- output, pointer to pointer of uint32_t for curve parameter p
 *     curve_p_h ------------------ output, pointer to pointer of uint32_t for curve parameter p's hash
 *     curve ---------------------- output, pointer to eccp_curve_st struct
 *     crc_g ---------------------- output, CRC16 checksum of fp12g
 *     hid_value ------------------ input, user private generation function identity
 *     r -------------------------- input, random big integer r in wrapping, 32 bytes, big-endian
 *     C -------------------------- output, encapsulated cipher of key, 64 bytes
 *     k_bytes -------------------- input, bytes length of the output key
 *     k -------------------------- output, plaintext output key
 *     ctx ------------------------ output, pointer to eccp_sec_ctx_t struct
 * return: SM9_SUCCESS_S(success), SM9_ERROR_S(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_wrap_key_s_internal_step_2(
    const uint8_t *IDB, 
    uint32_t IDB_bytes, 
    const uint8_t *fp12g, 
    uint32_t tmp[8],
    uint32_t QBx[8], 
    uint32_t QBy[8],
    uint32_t fp12[8u*12u],
    uint32_t *curve_a, 
    uint32_t *curve_b, 
    uint32_t *curve_p, 
    uint32_t *curve_p_h,
    const eccp_curve_st *curve,
    volatile uint16_t *crc_g,
    uint8_t hid_value,
    const uint8_t r[32], uint8_t C[64], uint32_t k_bytes, uint8_t *k, 
    eccp_sec_ctx_t ctx[1]
)
{
    uint8_t hid_value_bak = hid_value;
    uint32_t ret;

    //actually the following pointers are the same as the corresponding fields of curve
    //copy back sm9 curve paras in PKE RAM that not erased by hardware
    uint32_copy_8_words(curve_a, (uint32_t *)(rPKE_B(4u,SM9_STEPS)));
    uint32_copy_8_words(curve_b, (uint32_t *)(rPKE_A(4u,SM9_STEPS)));
    uint32_copy_8_words(curve_p, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));
    uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM9_STEPS)));

    //tmp := H1(IDB||hid, N)
    ret = sm9_h1_h2((uint8_t)1, IDB, IDB_bytes, &hid_value_bak, 1u, tmp);
    if(PKE_SUCCESS == ret)
    {
        //get [H1(IDB||hid, N)]P1
        ret = eccp_pointMul_sec(curve, tmp, curve->eccp_Gx, curve->eccp_Gy, QBx, QBy);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //copy back sm9 curve paras in PKE RAM that not erased by hardware
        uint32_copy_8_words(curve_a, (uint32_t *)(rPKE_B(4u,SM9_STEPS)));
#if 0   //since p and p_h will be used by the following hardware calculation.
        uint32_copy_8_words(curve_p, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));
        uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM9_STEPS)));
#endif

        //QB = [H1(IDB||hid, N)]P1 + Ppub_e
        ret = eccp_pointAdd_safe_internal(curve, QBx, QBy, fp12, &fp12[SM9_BASE_WORD_LEN], QBx, QBy);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //get fp12 := e(Ppub_e, P2)
        if(NULL == fp12g)
        {
            ret = sm9_pairing(fp12, &fp12[SM9_BASE_WORD_LEN], sm9_G2_Px0_Px1, sm9_G2_Py0_Py1, fp12);

            *crc_g = crc16_calc((uint8_t *)fp12, 32u*12u, (uint16_t)0xFFFF);
        }
        else
        {
            *crc_g = crc16_calc(fp12g, 32u*12u, (uint16_t)0xFFFF);

            (void)sm9_fp12_u8big_2_u32little(fp12g, (uint32_t *)fp12);
        }
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //copy back sm9 curve paras in PKE RAM that not erased by hardware
        uint32_copy_8_words(curve_p, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));
        uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM9_STEPS)));

        if(NULL == r)
        {
            do {
                ret = get_rand((uint8_t *)tmp, SM9_BASE_BYTE_LEN);
                if(TRNG_SUCCESS != ret)
                {
                    ret = SM9_ERROR_S;
                    break;
                }
                else
                {}

                ret = sm9_wrap_key_with_r_s(ctx, IDB, IDB_bytes, QBx, QBy, fp12, tmp, C, k_bytes, k);
            } while((SM9_ZERO_ALL == ret) || (SM9_INTEGER_TOO_BIG == ret));
        }
        else
        {
            u8big_to_u32little_256bits(r, tmp);

            ret = sm9_wrap_key_with_r_s(ctx, IDB, IDB_bytes, QBx, QBy, fp12, tmp, C, k_bytes, k);
        }
    }
    else
    {}

    return ret;
}


/* function: SM9 key encapsulation(secure version), generates key and its encapsulated cipher C
 * parameters:
 *     IDB ------------------------ input, identify of user B, user B is receiver of the cipher C
 *     IDB_bytes ------------------ input, bytes length of the IDB
 *     hid ------------------------ input, user private generation function identity, published by KGC,
 *                                         default value is 0x03, one byte.
 *     fp12g ---------------------- input, the value of e(Ppub_e, P2), if set to null, it will be calculated within the function
 *     Ppub_e --------------------- input,  KGC's system encryption master public key
 *     r -------------------------- input, random big integer r in wrapping, 32 bytes, big-endian,
 *                                  if you do not have this integer, please set this parameter to be NULL,
 *                                  it will be generated inside.
 *     C -------------------------- output, encapsulated cipher of key, 64 bytes
 *     k_bytes -------------------- input, bytes length of the output key
 *     k -------------------------- output, plaintext output key
 * return: SM9_SUCCESS_S(success), other(error)
 * caution:
 */
uint32_t sm9_wrap_key_s(const uint8_t *IDB, uint32_t IDB_bytes, uint8_t hid, const uint8_t *fp12g, 
        const uint8_t Ppub_e[64], const uint8_t r[32], uint8_t C[64], uint32_t k_bytes, uint8_t *k)
{
    uint32_t ret = PKE_SUCCESS;
    uint32_t right_hand_operand_ret1;
    uint32_t right_hand_operand_ret2;
    uint32_t tmp[8];
    uint32_t QBx[8], QBy[8];
    uint32_t fp12[8u*12u];

    uint32_t *curve_a, *curve_b, *curve_p, *curve_p_h;
    const eccp_curve_st *curve;
    eccp_sec_ctx_t ctx[1];
    uint16_t eccp_curve_crc16 = (uint16_t)0;
    volatile uint16_t crc_g;
    uint16_t crc_g2;
    uint8_t hid_value = hid;

    if((NULL == IDB) || (NULL == k) || (NULL == C) || (NULL == Ppub_e) || (0u == IDB_bytes))
    {
        ret = SM9_ERROR_S;
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_wrap_key_s_internal_step_1(fp12, &curve_a, &curve_b, &curve_p, &curve_p_h, &curve, ctx, eccp_curve_crc16, Ppub_e);
    }

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_wrap_key_s_internal_step_2(IDB, IDB_bytes, fp12g, tmp, QBx, QBy, fp12, curve_a, curve_b, curve_p, 
            curve_p_h, curve, &crc_g, hid_value, r, C, k_bytes, k, ctx);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //check crc16 of sm9 paras
        if(0u != ecc_crc16_check(curve, eccp_curve_crc16))
        {
            ret = SM9_ERROR_S;
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            // calc crc of g
            if(NULL == fp12g)
            {
                crc_g2 = crc16_calc((uint8_t *)fp12, 32u*12u, (uint16_t)0xFFFF);
            }
            else
            {
                crc_g2 = crc16_calc(fp12g, 32u*12u, (uint16_t)0xFFFF);
            }
            
            right_hand_operand_ret1 = (uint32_t)crc_g ^ (uint32_t)crc_g2;
            right_hand_operand_ret2 = (uint32_t)crc_g - (uint32_t)crc_g2;
            if((crc_g != crc_g2) || (0u != right_hand_operand_ret1) || ((uint16_t)0 != right_hand_operand_ret2))
            {
                ret = SM9_ERROR_S;
            }
            else
            {}
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = SM9_SUCCESS_S;
    }
    else
    {
        ret = SM9_ERROR_S;
        (void)get_rand_fast(C, 64u);
        (void)get_rand_fast(k, k_bytes);
    }

    (void)get_rand_fast((uint8_t *)tmp, sizeof(tmp));
    (void)get_rand_fast((uint8_t *)QBx, sizeof(QBx));
    (void)get_rand_fast((uint8_t *)QBy, sizeof(QBy));
    (void)get_rand_fast((uint8_t *)fp12, sizeof(fp12));
    (void)get_rand_fast((uint8_t *)ctx, sizeof(eccp_sec_ctx_t));
    crc_g =(uint16_t)0; 
    crc_g2 = (uint16_t)0;

    return ret;
}


/* function: SM9 key decapsulation s internal step 1
 * parameters:
 *     deB ------------------------ input, the decapsulated key
 *     C -------------------------- input, the encapsulated ciphertext
 *     curve_a -------------------- output, pointer to pointer of uint32_t for curve parameter a
 *     curve_b -------------------- output, pointer to pointer of uint32_t for curve parameter b
 *     curve_p -------------------- output, pointer to pointer of uint32_t for curve parameter p
 *     curve_p_h ------------------ output, pointer to pointer of uint32_t for curve parameter p's hash
 *     curve ---------------------- output, pointer to eccp_curve_st struct
 *     ctx ------------------------ output, pointer to eccp_sec_ctx_t struct
 *     count ---------------------- input&output, pointer to volatile uint32_t counter
 *     Px ------------------------- output, x coordinate of the point P
 *     Py ------------------------- output, y coordinate of the point P
 *     Qx ------------------------- output, x coordinate of the point Q
 *     Qy ------------------------- output, y coordinate of the point Q
 * return: SM9_SUCCESS_S(success), SM9_ERROR_S(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_unwrap_key_s_internal_step_1(
    const uint8_t deB[128], 
    const uint8_t C[64],
    uint32_t **curve_a, 
    uint32_t **curve_b, 
    uint32_t **curve_p, 
    uint32_t **curve_p_h,
    const eccp_curve_st **curve,
    eccp_sec_ctx_t ctx[1],
    volatile uint32_t *count,
    uint32_t *Px,
    uint32_t *Py,
    uint32_t *Qx,
    uint32_t *Qy
)
{
    uint16_t eccp_curve_crc16 = 0u;
    uint32_t ret = PKE_SUCCESS;
    
    (void)counter_add_one(count);

    //init sm9 curve
    *curve = eccp_curve_init(ctx, sm9_curve);
    if(NULL == curve)
    {
        ret = SM9_ERROR_S;
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //check crc16 of sm9 paras
        if(0u != ecc_crc16_check(*curve, eccp_curve_crc16))
        {
            ret = SM9_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //actually the following pointers are the same as the corresponding fields of curve
        *curve_p   = &(ctx->eccp_curve_mem[0]);
        *curve_p_h = &(ctx->eccp_curve_mem[SM9_BASE_WORD_LEN]);
        *curve_a   = &(ctx->eccp_curve_mem[SM9_BASE_WORD_LEN<<1]);
        *curve_b   = &(ctx->eccp_curve_mem[SM9_BASE_WORD_LEN*3u]);

        (void)counter_add_one(count);

        //check C
        (void)sm9_fp_eccp_point_u8big_2_u32little(C, Px, Py);
        ret = eccp_check_point(*curve, Px, Py);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //copy back sm9 curve paras in PKE RAM that not erased by hardware
        uint32_copy_8_words(*curve_a, (uint32_t *)(rPKE_B(4u,SM9_STEPS)));
        uint32_copy_8_words(*curve_b, (uint32_t *)(rPKE_A(4u,SM9_STEPS)));
#if 0   //since p and p_h will be used by the following hardware calculation.
        uint32_copy_8_words(curve_p, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));
        uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM9_STEPS)));
#endif

        (void)counter_add_one(count);

        //check deB
        (void)sm9_fp2_eccp_point_u8big_2_u32little(deB, Qx, Qy);
        ret = sm9_fp2_check_point_internal(*curve, Qx, Qy);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);
    }
    else
    {}

    return ret;
}


/* function: SM9 key decapsulation s internal step 2
 * parameters:
 *     tmp_r ---------------------- output, temporary buffer for random number
 *     Qx ------------------------- input, x coordinate of the point Q
 *     Qy ------------------------- input, y coordinate of the point Q
 *     Px ------------------------- input, x coordinate of the point P
 *     Py ------------------------- input, y coordinate of the point P
 *     R1x ------------------------ output, x coordinate of the point R1
 *     R1y ------------------------ output, y coordinate of the point R1
 *     R2x ------------------------ output, x coordinate of the point R2
 *     R2y ------------------------ output, y coordinate of the point R2
 *     tmp_fp12g1 ----------------- output, temporary buffer for fp12g1
 *     tmp_fp12g2 ----------------- output, temporary buffer for fp12g2
 *     curve_b -------------------- output, pointer to pointer of uint32_t for curve parameter b
 *     curve ---------------------- input, pointer to eccp_curve_st struct
 *     count ---------------------- input&output, pointer to volatile uint32_t counter
 * return: SM9_SUCCESS_S(success), SM9_ERROR_S(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_unwrap_key_s_internal_step_2(
    uint32_t *tmp_r,
    const uint32_t *Qx,
    const uint32_t *Qy,
    const uint32_t *Px,
    const uint32_t *Py,
    uint32_t *R1x,
    uint32_t *R1y,
    uint32_t *R2x,
    uint32_t *R2y,
    uint32_t tmp_fp12g1[8u*12u], 
    uint32_t tmp_fp12g2[8u*12u],
    uint32_t *curve_b,
    const eccp_curve_st *curve,
    volatile uint32_t *count
)
{
    uint32_t ret;

    //get random tmp_r in [1,n-1]
    do {
        ret = get_rand((uint8_t *)tmp_r, 32u);
        if(TRNG_SUCCESS != ret)
        {
            ret = SM9_ERROR_S;
            break;
        }
        else
        {}

        ret = uint32_integer_check_sec(tmp_r, curve->eccp_n, SM9_BASE_WORD_LEN, SM9_ZERO_ALL, SM9_INTEGER_TOO_BIG, PKE_SUCCESS);
    } while(PKE_SUCCESS != ret);

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        // get deB1 (R1x, R1y) = [tmp_r]G2
        ret = sm9_fp2_pointMul_s_internal(curve, tmp_r, sm9_G2_Px0_Px1, sm9_G2_Py0_Py1, R1x, R1y);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //copy back sm9 curve paras in PKE RAM that not erased by hardware
        uint32_copy_8_words(curve_b, (uint32_t *)(rPKE_B(3u,SM9_STEPS)));
#if 0   //since p and p_h will be used by the following hardware calculation.
        uint32_copy_8_words(curve_p, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));
        uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM9_STEPS)));
#endif

        (void)counter_add_one(count);

        //(R1x, -R1y)
        uint32_copy(R2x, R1x, 16u);
#if 0
        ret = pke_sub(curve->eccp_p, R1y, R2y, SM9_BASE_WORD_LEN);
#else
        ret = pke_mod_add_sub_mul_256bits_internal(curve->eccp_p, R1y, R2y, MICROCODE_INTSUB);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
    (void)counter_add_one(count);
#if 0
    ret = pke_sub(curve->eccp_p, &R1y[SM9_BASE_WORD_LEN], &R2y[SM9_BASE_WORD_LEN], SM9_BASE_WORD_LEN);
#else
    ret = pke_mod_add_sub_mul_256bits_internal(curve->eccp_p, &R1y[SM9_BASE_WORD_LEN], 
            &R2y[SM9_BASE_WORD_LEN], MICROCODE_INTSUB);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //get deB = R1 + R2
        ret = sm9_fp2_pointAdd_internal(Qx, Qy, R2x, R2y, R2x, R2y);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        ret = sm9_pairing(Px, Py, R1x, R1y, tmp_fp12g1);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        ret = sm9_pairing(Px, Py, R2x, R2y, tmp_fp12g2);
    }
    else
    {}


    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        ret = sm9_fp12_mul(tmp_fp12g1, tmp_fp12g2, NULL);
    }
    else
    {}

    return ret;
}


/* function: SM9 key decapsulation s internal step 3
 * parameters:
 *     h2rf_para ------------------ output, parameter for h2rf function
 *     Qx ------------------------- output, x coordinate of the point Q
 *     curve_p -------------------- output, pointer to pointer of uint32_t for curve parameter p
 *     curve_p_h ------------------ output, pointer to pointer of uint32_t for curve parameter p's hash
 *     count ---------------------- input&output, pointer to volatile uint32_t counter
 *     digest_node ---------------- input, array of hash nodes
 *     counter_buf ---------------- input, buffer for counter
 *     k_bytes -------------------- input, bytes length of the output key
 *     k -------------------------- output, plaintext output key
 *     crc_k ---------------------- output, CRC16 checksum of the key
 * return: SM9_SUCCESS_S(success), SM9_ERROR_S(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_unwrap_key_s_internal_step_3(
    uint8_t *h2rf_para,  
    uint32_t *Qx,
    uint32_t *curve_p, 
    uint32_t *curve_p_h,
    volatile uint32_t *count,  
    const hash_node_st digest_node[4],  
    uint32_t counter_buf[1],   
    uint32_t k_bytes,  
    uint8_t *k,
    volatile uint16_t *crc_k
)
{
    uint32_t ret;

    //copy back sm9 curve paras in PKE RAM that not erased by hardware
    uint32_copy_8_words(curve_p, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));
    uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM9_STEPS)));

    (void)counter_add_one(count);

    (void)sm9_fp12_in_pke_ram_u32little_2_u8big(h2rf_para);

    (void)counter_add_one(count);

    //key := KDF(C||w||IDB, key_bytes)
    ret = ansi_x9_63_kdf_node(HASH_SM3, digest_node, 4u, (uint8_t *)counter_buf, k, k_bytes, NULL, 0u);
    if(HASH_SUCCESS == ret)
    {
        *crc_k = crc16_calc(k, k_bytes, (uint16_t)0xFFFF);

        (void)counter_add_one(count);

        //get rand for sleeping
        ret = get_rand((uint8_t *)Qx, 1u<<2);
        if(TRNG_SUCCESS == ret)
        {
            //sleep between the two repeated calculations
            uint32_sleep(Qx[0] & 0x0Fu, (uint8_t)(Qx[0] >> 4));

            (void)counter_add_one(count);

            //calc key = KDF(C||w||IDB, key_bytes) again
            counter_buf[0] = 0u;
            ((uint8_t *)counter_buf)[3] = (uint8_t)1u;
            ret = ansi_x9_63_kdf_node(HASH_SM3, digest_node, 4u, (uint8_t *)counter_buf, k, k_bytes, NULL, 0u);
            if(HASH_SUCCESS == ret)
            {
                ret = PKE_SUCCESS;
            }
            else
            {}
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: SM9 key decapsulation s internal step 4
 * parameters:
 *     curve ---------------------- input, pointer to eccp_curve_st struct
 *     k_bytes -------------------- input, bytes length of the output key
 *     k -------------------------- input, plaintext output key
 *     Qx ------------------------- output, x coordinate of the point Q
 *     count ---------------------- input&output, pointer to volatile uint32_t counter
 *     crc_k ---------------------- input, CRC16 checksum of the key
 *     crc_k2 --------------------- output, CRC16 checksum of the key (calculated within the function)
 * return: SM9_SUCCESS_S(success), SM9_ERROR_S(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_unwrap_key_s_internal_step_4(
    const eccp_curve_st *curve,
    uint32_t k_bytes,
    const uint8_t *k,
    uint32_t *Qx,
    volatile uint32_t *count,
    const uint16_t *crc_k,
    uint16_t *crc_k2
)
{
    uint32_t right_hand_operand_ret1;
    uint32_t right_hand_operand_ret2;
    uint32_t ret = PKE_SUCCESS;
    uint16_t eccp_curve_crc16 = 0u;

    *crc_k2 = crc16_calc(k, k_bytes, (uint16_t)0xFFFF);

    (void)counter_add_one(count);

    right_hand_operand_ret1 = (uint32_t)(*crc_k) ^ (uint32_t)(*crc_k2);
    right_hand_operand_ret2 = (uint32_t)(*crc_k) - (uint32_t)(*crc_k2);
    if(((*crc_k) != (*crc_k2)) || ((uint16_t)0 != right_hand_operand_ret1) || ((uint16_t)0 != right_hand_operand_ret2))
    {
        ret = SM9_ERROR_S;
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //check key for 3 times
        Qx[0] >>= 8;
        uint32_sleep(Qx[0] & 0x0Fu, (uint8_t)(Qx[0] >> 4));
        right_hand_operand_ret1 = uint8_BigNum_Check_Zero(k, k_bytes);
    }
    else
    {}

    if((PKE_SUCCESS == ret) && (0u == right_hand_operand_ret1))
    {
        (void)counter_add_one(count);

        Qx[0] >>= 8;
        uint32_sleep(Qx[0] & 0x0Fu, (uint8_t)(Qx[0] >> 4));
        right_hand_operand_ret1 = uint8_BigNum_Check_Zero(k, k_bytes);
    }
    else
    {}

    if((PKE_SUCCESS == ret) && (0u == right_hand_operand_ret1))
    {
        (void)counter_add_one(count);

        Qx[0] >>= 8;
        uint32_sleep(Qx[0] & 0x0Fu, (uint8_t)(Qx[0] >> 4));
        right_hand_operand_ret1 = uint8_BigNum_Check_Zero(k, k_bytes);
    }
    else
    {}

    if((PKE_SUCCESS == ret) && (0u == right_hand_operand_ret1))
    {
        (void)counter_add_one(count);

        //check crc16 of sm9 paras
        if(0u != ecc_crc16_check(curve, eccp_curve_crc16))
        {
            ret = SM9_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        if((*count) != (SM9_SEC_WRAP_COUNTER + 0x15U))
        {
            ret = SM9_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: SM9 key decapsulation(secure version), generates key from encapsulated cipher C
 * parameters:
 *     IDB ------------------------ input, identity of user B
 *     IDB_bytes ------------------ input, bytes length of the IDB
 *     deB ------------------------ input, private key of user B
 *     C -------------------------- output, encapsulated cipher of key, 64 bytes
 *     k_bytes -------------------- input, bytes length of the output key
 *     k -------------------------- output, plaintext output key
 * return: SM9_SUCCESS_S(success), other(error)
 * caution:
 */
uint32_t sm9_unwrap_key_s(const uint8_t *IDB, uint32_t IDB_bytes, const uint8_t deB[128], 
        const uint8_t C[64], uint32_t k_bytes, uint8_t *k)
{
    uint32_t ret = PKE_SUCCESS;
    uint32_t counter_buf[1] = {0u};
    const uint8_t *counter = (uint8_t *)counter_buf;
    uint32_t tmp_fp12g1[8u*12u], tmp_fp12g2[8u*12u];

#if 0
    uint32_t tmp_r[8];
    uint32_t Px[8], Py[8], Qx[16], Qy[16];
    uint32_t R1x[16], R1y[16], R2x[16], R2y[16];
    uint8_t h2rf_para[32*12];
#else
#ifndef SUPPORT_STATIC_ANALYSIS
#define  tmp_r      ((uint32_t *)tmp_fp12g1)
#define  Qx         (&(tmp_fp12g1[SM9_BASE_WORD_LEN]))
#define  Qy         (&(tmp_fp12g1[3u*SM9_BASE_WORD_LEN]))
#define  Px         ((uint32_t *)tmp_fp12g2)
#define  Py         (&(tmp_fp12g2[SM9_BASE_WORD_LEN]))
#define  R1x        (&(tmp_fp12g2[2u*SM9_BASE_WORD_LEN]))
#define  R1y        (&(tmp_fp12g2[4u*SM9_BASE_WORD_LEN]))
#define  R2x        (&(tmp_fp12g2[6u*SM9_BASE_WORD_LEN]))
#define  R2y        (&(tmp_fp12g2[8u*SM9_BASE_WORD_LEN]))
#define  h2rf_para  ((uint8_t *)tmp_fp12g2)
#else
    uint32_t *tmp_r    = (uint32_t *)tmp_fp12g1;
    uint32_t *Qx       = &(tmp_fp12g1[SM9_BASE_WORD_LEN]);
    uint32_t *Qy       = &(tmp_fp12g1[3u*SM9_BASE_WORD_LEN]);
    uint32_t *Px       = (uint32_t *)tmp_fp12g2;
    uint32_t *Py       = &(tmp_fp12g2[SM9_BASE_WORD_LEN]);
    uint32_t *R1x      = &(tmp_fp12g2[2u*SM9_BASE_WORD_LEN]);
    uint32_t *R1y      = &(tmp_fp12g2[4u*SM9_BASE_WORD_LEN]);
    uint32_t *R2x      = &(tmp_fp12g2[6u*SM9_BASE_WORD_LEN]);
    uint32_t *R2y      = &(tmp_fp12g2[8u*SM9_BASE_WORD_LEN]);
    uint8_t *h2rf_para = (uint8_t *)tmp_fp12g2;
#endif
#endif

    volatile uint32_t count = SM9_SEC_WRAP_COUNTER;

    hash_node_st digest_node[4];

    uint32_t *curve_a, *curve_b, *curve_p, *curve_p_h;
    const eccp_curve_st *curve;
    eccp_sec_ctx_t ctx[1];
    uint16_t crc_k;
    uint16_t crc_k2;

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
        ret = SM9_ERROR_S;
    }
    else if(0u == IDB_bytes)
    {
        ret = SM9_ERROR_S;
    }
    else
    {
        //nothing to do, just for static analysis.
    }

    if(PKE_SUCCESS == ret)
    {   
        ret = sm9_unwrap_key_s_internal_step_1(deB, C, &curve_a, &curve_b, &curve_p, &curve_p_h, &curve, ctx, &count, Px, Py, Qx, Qy);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_unwrap_key_s_internal_step_2(tmp_r, Qx, Qy, Px, Py, R1x, R1y, R2x, R2y, tmp_fp12g1, tmp_fp12g2, curve_b, curve, &count);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_unwrap_key_s_internal_step_3(h2rf_para, Qx, curve_p, curve_p_h, &count, digest_node, counter_buf, k_bytes, k, &crc_k);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_unwrap_key_s_internal_step_4(curve, k_bytes, k, Qx, &count, &crc_k, &crc_k2);
    }
    else
    {}

#ifndef SUPPORT_STATIC_ANALYSIS
#undef  tmp_r
#undef  Qx
#undef  Qy
#undef  Px
#undef  Py
#undef  R1x
#undef  R1y
#undef  R2x
#undef  R2y
#undef  h2rf_para
#endif

    if(PKE_SUCCESS == ret)
    {
        ret = SM9_SUCCESS_S;
    }
    else
    {
        ret = SM9_ERROR_S;
        (void)get_rand_fast((uint8_t *)k, k_bytes);
    }

    (void)get_rand_fast((uint8_t *)counter_buf, sizeof(counter_buf));
    (void)get_rand_fast((uint8_t *)tmp_fp12g1, sizeof(tmp_fp12g1));
    (void)get_rand_fast((uint8_t *)tmp_fp12g2, sizeof(tmp_fp12g2));
    (void)get_rand_fast((uint8_t *)ctx, sizeof(eccp_sec_ctx_t));
    curve = NULL;
    crc_k =  (uint16_t)0;
    crc_k2 = (uint16_t)0;

    return ret;
}


/* function: SM9 encrypt with r check CRC (secure version)
 * parameters:
 *     K1 ------------------------- input, key K1 in internal encrypting
 *     K1_bytes ------------------- input, bytes length of the key K1 in internal encrypting
 *     K2 ------------------------- input, key K2 in internal MAC function
 *     K2_bytes ------------------- input, bytes length of the key K2 in MAC function
 *     t -------------------------- input, temporary buffer for calculations
 *     crc_k ---------------------- input, CRC16 checksum of the key
 * return: SM9_SUCCESS_S(success), SM9_ERROR_S(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_enc_with_r_s_check_crc(
    const uint8_t *K1, 
    uint32_t K1_bytes, 
    const uint8_t *K2, 
    uint32_t K2_bytes,
    uint32_t t[1],
    uint16_t crc_k
)
{
    uint16_t crc_k2;
    uint32_t ret = PKE_SUCCESS;    

    crc_k2 = crc16_calc(K1, K1_bytes, (uint16_t)0xFFFF);
    crc_k2 = crc16_calc(K2, K2_bytes, crc_k2);

    if((crc_k != crc_k2) || ((uint16_t)0 != (crc_k ^ crc_k2)) || ((uint16_t)0 != (crc_k - crc_k2)))
    {
        ret = SM9_ERROR_S;
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //check K1 3 times, K1 can not be zero
        t[0] >>= 8;
        uint32_sleep((t[0]) & 0x0Fu, (uint8_t)(t[0] >> 4));
        if(0u != uint8_BigNum_Check_Zero_sec(K1, K1_bytes))
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
        t[0] >>= 8;
        uint32_sleep((t[0]) & 0x0Fu, (uint8_t)(t[0] >> 4));
        if(0u != uint8_BigNum_Check_Zero_sec(K1, K1_bytes))
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
        t[0] >>= 8;
        uint32_sleep((t[0]) & 0x0Fu, (uint8_t)(t[0] >> 4));
        if(0u != uint8_BigNum_Check_Zero_sec(K1, K1_bytes))
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


/* function: SM9 encrypt with r(secure version)
 * parameters:
 *     ctx ------------------------ input, eccp_sec_ctx_t struct pointer
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
 * return: SM9_SUCCESS_S(success), other(error)
 * caution:
 *     1. ctx must be initialized by eccp_curve_init() before calling this function.
 *     2. IDB only represents the decrypter.
 *     3. K2_bytes should be less than SM9_MAX_ENC_K2_BYTE_LEN
 */
FLAG_STATIC uint32_t sm9_enc_with_r_s(eccp_sec_ctx_t *ctx, const uint8_t *IDB, 
        uint32_t IDB_bytes, const uint32_t *r, const uint32_t *QBx, 
        const uint32_t *QBy, const uint32_t fp12[8*12], uint8_t *C1, uint8_t *K1, 
        uint32_t K1_bytes, uint8_t *K2, uint32_t K2_bytes)
{
    uint32_t ret;
    uint32_t counter_buf[1] = {0u};
    const uint8_t *counter = (uint8_t *)counter_buf;
    uint32_t w[8u*12u];
    uint32_t t[1];
    uint16_t crc_k;
    uint32_t *curve_a, *curve_p, *curve_p_h;
    const eccp_curve_st *curve;

    hash_node_st digest_node[4];

    curve = ctx->curve;

    //actually the following pointers are the same as the corresponding fields of curve
    curve_p   = &(ctx->eccp_curve_mem[0]);
    curve_p_h = &(ctx->eccp_curve_mem[SM9_BASE_WORD_LEN]);
    curve_a   = &(ctx->eccp_curve_mem[SM9_BASE_WORD_LEN<<1]);

    //make sure r in [1, n-1]
    ret = uint32_integer_check_sec(r, sm9_curve->eccp_n, SM9_BASE_WORD_LEN, SM9_ZERO_ALL, SM9_INTEGER_TOO_BIG, PKE_SUCCESS);
    if(PKE_SUCCESS == ret)
    {
        //get C1 = [r]QB
        ret = eccp_pointMul_sec(curve, r, QBx, QBy, w, &w[SM9_BASE_WORD_LEN]);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //copy back sm9 curve paras in PKE RAM that not erased by hardware
        uint32_copy_8_words(curve_a, (uint32_t *)(rPKE_B(4u,SM9_STEPS)));
#if 0   //since p and p_h will be used by the following hardware calculation.
        uint32_copy_8_words(curve_p, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));
        uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM9_STEPS)));
#endif

        (void)sm9_fp_eccp_point_u32little_2_u8big(w, &w[SM9_BASE_WORD_LEN], C1);

        //w = g^r
        ret = sm9_fp12_exp_s(fp12, r, NULL);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //copy back sm9 curve paras in PKE RAM that not erased by hardware
        uint32_copy_8_words(curve_p, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));
        uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM9_STEPS)));

        //check r
        if((int32_t)0 != uint32_BigNumCmp_sec(r, SM9_BASE_WORD_LEN, (uint32_t *)rPKE_A(1u,SM9_STEPS), SM9_BASE_WORD_LEN))
        {
            ret = SM9_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)sm9_fp12_in_pke_ram_u32little_2_u8big((uint8_t *)w);

        //get k1||K2 = KDF(C1||w||IDB, K1_bytes+K2_bytes)
        ((uint8_t *)counter_buf)[3] = (uint8_t)1u;
        digest_node[0].msg_addr  = C1;
        digest_node[0].msg_bytes = 64u;
        digest_node[1].msg_addr  = (uint8_t *)w;
        digest_node[1].msg_bytes = 32u*12u;
        digest_node[2].msg_addr  = IDB;
        digest_node[2].msg_bytes = IDB_bytes;
        digest_node[3].msg_addr  = counter ;
        digest_node[3].msg_bytes = 4u;
        ret = ansi_x9_63_kdf_node(HASH_SM3, digest_node, 4u, (uint8_t *)counter_buf, K1, K1_bytes, K2, K2_bytes);

        if(HASH_SUCCESS == ret)
        {
            crc_k = crc16_calc(K1, K1_bytes, (uint16_t)0xFFFF);
            crc_k = crc16_calc(K2, K2_bytes, crc_k);

            //get rand for sleeping
            ret = get_rand((uint8_t *)t, 1u<<2u);
            if(TRNG_SUCCESS == ret)
            {
                //sleep between the two repeated calculations
                uint32_sleep((t[0]) & 0x0Fu, (uint8_t)(t[0] >> 4));

                //calc again
                counter_buf[0] = 0u;
                ((uint8_t *)counter_buf)[3] = (uint8_t)1u;
                ret = ansi_x9_63_kdf_node(HASH_SM3, digest_node, 4u, (uint8_t *)counter_buf, K1, K1_bytes, K2, K2_bytes);
#if 0
                if(HASH_SUCCESS == ret)
                {
                    ret = PKE_SUCCESS;
                }
                else
                {}
#endif
            }
            else
            {}
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_enc_with_r_s_check_crc(K1, K1_bytes, K2, K2_bytes, t, crc_k);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = SM9_SUCCESS_S;
    }
    else
    {
        (void)get_rand_fast(C1, 64u);
        (void)get_rand_fast(K1, K1_bytes);
        (void)get_rand_fast(K2, K2_bytes);
    }

    (void)get_rand_fast((uint8_t *)counter_buf, sizeof(counter_buf));
    (void)get_rand_fast((uint8_t *)w, sizeof(w));

    return ret;
}


/* function: SM9 encrypt s internal step 1
 * parameters:
 *     IDB ------------------------ input, identity of user B, user B is the cipher receiver
 *     IDB_bytes ------------------ input, bytes length of IDB
 *     M -------------------------- input, plaintext message to be encrypted
 *     M_bytes -------------------- input, bytes length of the plaintext message
 *     Ppub_e --------------------- input, KGC's master public key
 *     enc_type ------------------- input, encryption type
 *     padding_type --------------- input, padding type
 *     K2_bytes ------------------- input, bytes length of the key K2 in MAC function
 *     C -------------------------- input, ciphertext buffer
 *     C_bytes -------------------- input, pointer to the length of the ciphertext buffer
 * return: SM9_SUCCESS_S(success), SM9_ERROR_S(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_enc_s_internal_step_1(
    const uint8_t *IDB, 
    uint32_t IDB_bytes, 
    const uint8_t *M, 
    uint32_t M_bytes, 
    const uint8_t Ppub_e[64], 
    sm9_enc_type_e enc_type, 
    sm9_enc_padding_e padding_type, 
    uint32_t K2_bytes, 
    const uint8_t *C, 
    const uint32_t *C_bytes
)
{
    uint32_t ret = PKE_SUCCESS;

    if(enc_type > SM9_ENC_KDF_BLOCK_CIPHER)
    {
        ret = SM9_ERROR_S;
    }
    else if(SM9_ENC_KDF_BLOCK_CIPHER == enc_type)
    {
        if(padding_type > SKE_PKCS_5_7_PADDING)
        {
            ret = SM9_ERROR_S;
        }
        else if((0u != (M_bytes & 0x0Fu)) && (SKE_NO_PADDING == padding_type))
        {
            ret = SM9_ERROR_S;
        }
        else
        {
            //nothing to do, just for static analysis.
        }
    }
    else
    {
        //nothing to do, just for static analysis.
    }

    if(PKE_SUCCESS == ret)
    {
        if((NULL == IDB) || (NULL == M) || (NULL == C) || (NULL == C_bytes) || (NULL == Ppub_e) || (M == C) || (0u == IDB_bytes))
        {
            ret = SM9_ERROR_S;
        }
        else if((0u == M_bytes) || (M_bytes >= SM9_MAX_MSG_BYTE_LEN))
        {
            ret = SM9_ERROR_S;
        }
        else if(K2_bytes > SM9_MAX_ENC_K2_BYTE_LEN)
        {
            ret = SM9_ERROR_S;
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


/* function: SM9 encrypt s internal step 2
 * parameters:
 *     tmp ------------------------ output, temporary buffer for calculations
 *     QBx ------------------------- output, x coordinate of the point QB
 *     QBy ------------------------- output, y coordinate of the point QB
 *     fp12 ---------------------- output, buffer for storing the result of pairing operation
 *     Px ------------------------- output, x coordinate of the point P
 *     Py ------------------------- output, y coordinate of the point P
 *     curve_a -------------------- output, pointer to pointer of uint32_t for curve parameter a
 *     curve_b -------------------- output, pointer to pointer of uint32_t for curve parameter b
 *     curve_p -------------------- output, pointer to pointer of uint32_t for curve parameter p
 *     curve_p_h ------------------ output, pointer to pointer of uint32_t for curve parameter p's hash
 *     curve ---------------------- output, pointer to eccp_curve_st struct
 *     ctx ------------------------ output, pointer to eccp_sec_ctx_t struct
 *     crc_g ---------------------- output, CRC16 checksum of fp12g
 *     hid_value ------------------ input, user private generation function identity
 *     IDB ------------------------ input, identity of user B
 *     IDB_bytes ------------------ input, bytes length of the IDB
 *     fp12g ---------------------- input, the value of e(Ppub_e, P2), if set to null, it will be calculated within the function
 *     Ppub_e --------------------- input, KGC's master public key
 * return: SM9_SUCCESS_S(success), SM9_ERROR_S(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_enc_s_internal_step_2(
    uint32_t tmp[8u+8u],
    uint32_t QBx[8], 
    uint32_t QBy[8],
    uint32_t fp12[8u*12u],
    uint32_t *Px,
    uint32_t *Py,
    uint32_t **curve_a, 
    uint32_t **curve_b, 
    uint32_t **curve_p, 
    uint32_t **curve_p_h,
    const eccp_curve_st **curve,
    eccp_sec_ctx_t ctx[1],
    volatile uint16_t *crc_g,
    uint8_t hid_value,
    const uint8_t *IDB, 
    uint32_t IDB_bytes,
    const uint8_t *fp12g, 
    const uint8_t Ppub_e[64]
)
{
    uint16_t eccp_curve_crc16 = 0u;
    uint32_t ret = PKE_SUCCESS;
    uint8_t hid_value_bak = hid_value;

    //init sm9 curve
    *curve = eccp_curve_init(ctx, sm9_curve);
    if(NULL == *curve)
    {
        ret = SM9_ERROR_S;
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //check crc16 of sm9 paras
        if(0u != ecc_crc16_check(*curve, eccp_curve_crc16))
        {
            ret = SM9_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //actually the following pointers are the same as the corresponding fields of curve
        *curve_p   = &(ctx->eccp_curve_mem[0]);
        *curve_p_h = &(ctx->eccp_curve_mem[SM9_BASE_WORD_LEN]);
        *curve_a   = &(ctx->eccp_curve_mem[SM9_BASE_WORD_LEN<<1]);
        *curve_b   = &(ctx->eccp_curve_mem[SM9_BASE_WORD_LEN*3u]);

        //check Ppub_e
        (void)sm9_fp_eccp_point_u8big_2_u32little(Ppub_e, Px, Py);
        ret = eccp_check_point(*curve, Px, Py);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //copy back sm9 curve paras in PKE RAM that not erased by hardware
        uint32_copy_8_words(*curve_a, (uint32_t *)(rPKE_B(4u,SM9_STEPS)));
        uint32_copy_8_words(*curve_b, (uint32_t *)(rPKE_A(4u,SM9_STEPS)));
        uint32_copy_8_words(*curve_p, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));
        uint32_copy_8_words(*curve_p_h, (uint32_t *)(rPKE_B(0u,SM9_STEPS)));

        //tmp := H1(IDB||hid, N)
        ret = sm9_h1_h2((uint8_t)1, IDB, IDB_bytes, &hid_value_bak, 1u, tmp);
        if(PKE_SUCCESS == ret)
        {
            //get [H1(IDB||hid, N)]P1
            ret = eccp_pointMul_sec(*curve, tmp, sm9_curve->eccp_Gx, sm9_curve->eccp_Gy, QBx, QBy);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            //copy back sm9 curve paras in PKE RAM that not erased by hardware
            uint32_copy_8_words(*curve_a, (uint32_t *)(rPKE_B(4u,SM9_STEPS)));
#if 0   //since p and p_h will be used by the following hardware calculation.
            uint32_copy_8_words(curve_p, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));
            uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM9_STEPS)));
#endif

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
                ret = sm9_pairing(Px, Py, sm9_G2_Px0_Px1, sm9_G2_Py0_Py1, fp12);

                *crc_g = crc16_calc((uint8_t *)fp12, 32u*12u, (uint16_t)0xFFFF);
            }
            else
            {
                *crc_g = crc16_calc(fp12g, 32u*12u, (uint16_t)0xFFFF);

                (void)sm9_fp12_u8big_2_u32little(fp12g, fp12);
            }
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: SM9 encrypt s internal step 3
 * parameters:
 *     IDB ------------------------ input, identity of user B
 *     IDB_bytes ------------------ input, bytes length of the IDB
 *     M_bytes -------------------- input, bytes length of the plaintext message
 *     r -------------------------- input, random big integer r in wrapping, 32 bytes, big-endian
 *     enc_type ------------------- input, encryption type
 *     K2_bytes ------------------- input, bytes length of the key K2 in MAC function
 *     C -------------------------- output, encapsulated cipher of key, 64 bytes
 *     K2 ------------------------- output, key K2 in internal MAC function
 *     tmp ------------------------ output, temporary buffer for calculations
 *     QBx ------------------------- input, x coordinate of the point QB
 *     QBy ------------------------- input, y coordinate of the point QB
 *     fp12 ---------------------- input, buffer for storing the result of pairing operation
 *     curve_p -------------------- output, pointer to pointer of uint32_t for curve parameter p
 *     curve_p_h ------------------ output, pointer to pointer of uint32_t for curve parameter p's hash
 *     C2 ------------------------- output, ciphertext buffer
 *     ctx ------------------------ output, pointer to eccp_sec_ctx_t struct
 * return: SM9_SUCCESS_S(success), SM9_ERROR_S(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_enc_s_internal_step_3(
    const uint8_t *IDB, 
    uint32_t IDB_bytes,
    uint32_t M_bytes,
    const uint8_t r[32],
    sm9_enc_type_e enc_type,
    uint32_t K2_bytes,
    uint8_t *C,
    uint8_t K2[SM9_MAX_ENC_K2_BYTE_LEN],
    uint32_t tmp[8u+8u],
    const uint32_t QBx[8], 
    const uint32_t QBy[8],
    const uint32_t fp12[8u*12u],
    uint32_t *curve_p, 
    uint32_t *curve_p_h,
    uint8_t *C2,
    eccp_sec_ctx_t ctx[1]
)
{
    uint32_t K1_bytes = 0U;
    uint32_t ret;

    //copy back sm9 curve paras in PKE RAM that not erased by hardware
    uint32_copy_8_words(curve_p, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));
    uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM9_STEPS)));

    //get K1 length
    if(SM9_ENC_KDF_STREAM_CIPHER == enc_type)  //KDF
    {
        K1_bytes = M_bytes;
    }
    else if(SM9_ENC_KDF_BLOCK_CIPHER == enc_type) //SM4
    {
        K1_bytes = 16u;
    }
    else
    {
        //nothing to do, just for static analysis.
    }

    if(NULL == r)
    {
        do {
            ret = get_rand((uint8_t *)tmp, 32u);
            if(TRNG_SUCCESS != ret)
            {
                ret = SM9_ERROR_S;
                break;
            }
            else
            {}

            //get C1, K1, K2
            ret = sm9_enc_with_r_s(ctx, IDB, IDB_bytes, tmp, QBx, QBy, fp12,
                    C, C2, K1_bytes, K2, K2_bytes);
        } while((SM9_ZERO_ALL == ret) || (SM9_INTEGER_TOO_BIG == ret));
    }
    else
    {
        u8big_to_u32little_256bits(r, tmp);

        //get C1, K1, K2
        ret = sm9_enc_with_r_s(ctx, IDB, IDB_bytes, tmp, QBx, QBy, fp12,  
                C, C2, K1_bytes, K2, K2_bytes);
    }

    return ret;
}


/* function: SM9 encrypt s internal step 4
 * parameters:
 *     C2 ------------------------- output, ciphertext buffer
 *     digest_node ---------------- input, array of hash nodes
 *     K2 ------------------------- input, key K2 in internal MAC function
 *     tmp ------------------------ output, temporary buffer for calculations
 *     QBx ------------------------- input, x coordinate of the point QB
 *     QBy ------------------------- input, y coordinate of the point QB
 *     M_bytes -------------------- input, bytes length of the plaintext message
 *     enc_type ------------------- input, encryption type
 *     padding_type --------------- input, padding type
 *     K2_bytes ------------------- input, bytes length of the key K2 in MAC function
 *     C -------------------------- output, encapsulated cipher of key, 64 bytes
 *     C_bytes -------------------- input&output, pointer to the length of the ciphertext buffer
 *     M -------------------------- input, plaintext message to be encrypted
 *     t -------------------------- input, temporary buffer for calculations
 * return: SM9_SUCCESS_S(success), SM9_ERROR_S(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_enc_s_internal_step_4(
    uint8_t *C2,
    hash_node_st digest_node[2],
    const uint8_t K2[SM9_MAX_ENC_K2_BYTE_LEN],
    uint32_t tmp[8u+8u],
    uint32_t QBx[8], uint32_t QBy[8],
    uint32_t M_bytes, 
    sm9_enc_type_e enc_type, 
    sm9_enc_padding_e padding_type, 
    uint32_t K2_bytes, 
    uint8_t *C, 
    uint32_t *C_bytes,
    const uint8_t *M,
    uint32_t t[1]
)
{
    uint32_t ret = SKE_SUCCESS;
    *C_bytes = M_bytes;

    if(SM9_ENC_KDF_STREAM_CIPHER == enc_type)     //KDF
    {
        (void)uint8_XOR(C2, M, C2, M_bytes);
    }
    else if(SM9_ENC_KDF_BLOCK_CIPHER == enc_type) //SM4
    {
#if defined(SKE_HP) || defined(SKE_LP)
        ret = ske_crypto(SKE_ALG_SM4, SKE_MODE_ECB, SKE_CRYPTO_ENCRYPT, C2, 0u, NULL,
                padding_type, M, C2, M_bytes, C_bytes);
#elif defined(SKE_SECURE)
        ret = ske_sec_crypto(SKE_ALG_SM4, SKE_MODE_ECB, SKE_CRYPTO_ENCRYPT, C2, 0u, NULL,
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
        ret = PKE_ERROR;
    }

    if(PKE_SUCCESS == ret)
    {
        //get C3 := MAC(K2, C2) := sm3(C2||K2)
        digest_node[0].msg_addr  = C2;
        digest_node[0].msg_bytes = *C_bytes;
        digest_node[1].msg_addr  = K2;
        digest_node[1].msg_bytes = K2_bytes;
        ret = hash_node_steps(HASH_SM3, digest_node, 2u, &C[64u]);

        if(HASH_SUCCESS == ret)
        {
            memcpy_((uint8_t *)QBx, &C[64u], 32u);
            memcpy_((uint8_t *)(&tmp[8]), &C[64u], 32u);
            memcpy_((uint8_t *)QBy, &C[64u], 32u);

            //get rand for sleeping
            ret = get_rand((uint8_t *)t, 1u<<2);
            if(TRNG_SUCCESS == ret)
            {
                //sleep between the two repeated calculations
                uint32_sleep((t[0]) & 0x0Fu, (uint8_t)(t[0] >> 4));

                //calc C3 again
                ret = hash_node_steps(HASH_SM3, digest_node, 2u, (uint8_t *)tmp);
            }
            else
            {}

            if(HASH_SUCCESS == ret)
            {
                ret = PKE_SUCCESS;
            }
            else
            {}
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: SM9 encrypt s internal step 5
 * parameters:
 *     curve ---------------------- input, pointer to eccp_curve_st struct
 *     tmp ------------------------ input, temporary buffer for calculations
 *     QBx ------------------------- input, x coordinate of the point QB
 *     QBy ------------------------- input, y coordinate of the point QB
 *     fp12 ---------------------- input, buffer for storing the result of pairing operation
 *     fp12g ---------------------- input, the value of e(Ppub_e, P2), if set to null, it will be calculated within the function
 *     C_bytes -------------------- output, pointer to the length of the ciphertext buffer
 *     crc_g ---------------------- input, CRC16 checksum of fp12g
 *     crc_g2 --------------------- output, CRC16 checksum of fp12g (calculated within the function)
 *     t -------------------------- input, temporary buffer for calculations
 * return: SM9_SUCCESS_S(success), SM9_ERROR_S(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_enc_s_internal_step_5(
    const eccp_curve_st *curve,
    const uint32_t tmp[8u+8u],
    const uint32_t QBx[8], 
    const uint32_t QBy[8],
    const uint32_t fp12[8u*12u],
    const uint8_t *fp12g,
    uint32_t *C_bytes,
    const uint16_t *crc_g,
    uint16_t *crc_g2,
    uint32_t t[1]
)
{
    uint16_t eccp_curve_crc16 = 0u;
    uint32_t right_hand_operand_ret1;
    uint32_t right_hand_operand_ret2;
    uint32_t ret = PKE_SUCCESS;

    //check C3 3 times
    t[0] >>= 8;
    uint32_sleep((t[0]) & 0x0Fu, (uint8_t)(t[0] >> 4));
    right_hand_operand_ret1 = uint32_cmp_sec(tmp, QBx, 8, (uint8_t)(t[0] >> 5));

    if((PKE_SUCCESS == ret) && (0u == right_hand_operand_ret1))
    {
        t[0] >>= 8;
        uint32_sleep((t[0]) & 0x0Fu, (uint8_t)(t[0] >> 4));
        if(0u != uint32_cmp_sec(tmp, &tmp[8], 8u, (uint8_t)(t[0] >> 5)))
        {
            ret = SM9_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        t[0] >>= 8;
        uint32_sleep((t[0]) & 0x0Fu, (uint8_t)(t[0] >> 4));
        if(0u != uint32_cmp_sec(tmp, QBy, 8u, (uint8_t)(t[0] >> 5)))
        {
            ret = SM9_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //check crc16 of sm9 paras
        if(0u != ecc_crc16_check(curve, eccp_curve_crc16))
        {
            ret = SM9_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        // calc crc of g
        if(NULL == fp12g)
        {
            *crc_g2 = crc16_calc((const uint8_t *)fp12, 32u*12u, (uint16_t)0xFFFF);
        }
        else
        {
            *crc_g2 = crc16_calc(fp12g, 32u*12u, (uint16_t)0xFFFF);
        }

        right_hand_operand_ret1 = (uint32_t)(*crc_g) ^ (uint32_t)(*crc_g2);
        right_hand_operand_ret2 = (uint32_t)(*crc_g) - (uint32_t)(*crc_g2);
        if(((*crc_g) != (*crc_g2)) || ((uint16_t)0 != right_hand_operand_ret1) || ((uint16_t)0 != right_hand_operand_ret2))
        {
            ret = SM9_ERROR_S;
        }
        else
        {
            C_bytes[0] += 96u;
        }
    }
    else
    {}

    return ret;
}


/* function: SM9 encrypt(secure version)
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
 * return: SM9_SUCCESS_S(success), other(error)
 * caution:
 *     1. IDB only represents the decrypter.
 *     2. K2_bytes should be less than SM9_MAX_ENC_K2_BYTE_LEN
 */
uint32_t sm9_enc_s(const uint8_t *IDB, uint32_t IDB_bytes, uint8_t hid, const uint8_t *M, 
        uint32_t M_bytes, const uint8_t *fp12g, const uint8_t Ppub_e[64], const uint8_t r[32], 
        sm9_enc_type_e enc_type, sm9_enc_padding_e padding_type, uint32_t K2_bytes, 
        uint8_t *C, uint32_t *C_bytes)
{
    uint32_t ret = PKE_SUCCESS;
    uint8_t K2[SM9_MAX_ENC_K2_BYTE_LEN];
    uint32_t tmp[8u+8u];
    uint32_t QBx[8], QBy[8];
    uint32_t fp12[8u*12u];

#if 0
    uint32_t Px[8], Py[8];
#else
#ifndef SUPPORT_STATIC_ANALYSIS
#define Px   (fp12)
#define Py   (&fp12[SM9_BASE_WORD_LEN])
#else
uint32_t *Px = fp12;
uint32_t *Py = &fp12[SM9_BASE_WORD_LEN];
#endif
#endif
    uint8_t *C2;
    uint32_t t[1];
    hash_node_st digest_node[2];

    uint32_t *curve_a, *curve_b, *curve_p, *curve_p_h;
    const eccp_curve_st *curve;
    eccp_sec_ctx_t ctx[1];
    uint16_t crc_g;
    uint16_t crc_g2;
    uint8_t hid_value = hid;

    ret = sm9_enc_s_internal_step_1(IDB, IDB_bytes, M, M_bytes, Ppub_e, enc_type, padding_type, K2_bytes, C, C_bytes);

    if(PKE_SUCCESS == ret)
    {
        C2 = &C[96u];
        ret = sm9_enc_s_internal_step_2(tmp, QBx, QBy, fp12, Px, Py, &curve_a, &curve_b, &curve_p, &curve_p_h, &curve, ctx, &crc_g, 
            hid_value, IDB, IDB_bytes, fp12g, Ppub_e);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_enc_s_internal_step_3(IDB, IDB_bytes, M_bytes, r, enc_type, K2_bytes, C, K2, tmp, 
            QBx, QBy, fp12, curve_p, curve_p_h, C2, ctx);

        if(SM9_SUCCESS_S == ret)
        {
            ret = sm9_enc_s_internal_step_4(C2, digest_node, K2, tmp, QBx, QBy, M_bytes, enc_type, 
                padding_type, K2_bytes, C, C_bytes, M, t);
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_enc_s_internal_step_5(curve, tmp, QBx, QBy, fp12, fp12g, C_bytes, &crc_g, &crc_g2, t);
    }

    if(PKE_SUCCESS == ret)
    {
        ret = SM9_SUCCESS_S;
    }
    else
    {
        ret = SM9_ERROR_S;
        *C_bytes = 0u;
        (void)get_rand_fast(C, 64u+32u+M_bytes);
    }

#ifndef SUPPORT_STATIC_ANALYSIS
#undef  Px
#undef  Py
#endif

    (void)get_rand_fast((uint8_t *)K2, sizeof(K2));
    (void)get_rand_fast((uint8_t *)tmp, sizeof(tmp));
    (void)get_rand_fast((uint8_t *)QBx, sizeof(QBx));
    (void)get_rand_fast((uint8_t *)QBy, sizeof(QBy));
    (void)get_rand_fast((uint8_t *)fp12, sizeof(fp12));
    (void)get_rand_fast((uint8_t *)ctx, sizeof(eccp_sec_ctx_t));
    crc_g =(uint16_t)0; 
    crc_g2 = (uint16_t)0;

    return ret;
}


/* function: SM9 decrypt s internal step 1
 * parameters:
 *     IDB ------------------------ input, identity of user B, user B is the cipher receiver
 *     IDB_bytes ------------------ input, bytes length of IDB
 *     C -------------------------- input, ciphertext to be decrypted
 *     C_bytes -------------------- input, bytes length of the ciphertext
 *     deB ------------------------ input, decapsulated key
 *     enc_type ------------------- input, type of encryption (SM9_ENC_KDF_STREAM_CIPHER or SM9_ENC_KDF_BLOCK_CIPHER)
 *     padding_type --------------- input, type of padding(SKE_NO_PADDING or SKE_PKCS_5_7_PADDING)
 *     K2_bytes ------------------- input, bytes length of the key K2 in MAC function
 *     M -------------------------- input, message buffer to store the decrypted message
 *     M_bytes -------------------- input, pointer to the length of the message buffer
 * return: SM9_SUCCESS_S(success), SM9_ERROR_S(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_dec_s_internal_step_1(const uint8_t *IDB, uint32_t IDB_bytes, const uint8_t *C, uint32_t C_bytes, 
        const uint8_t deB[128], sm9_enc_type_e enc_type, sm9_enc_padding_e padding_type, 
        uint32_t K2_bytes, const uint8_t *M, const uint32_t *M_bytes)
{
    uint32_t ret = PKE_SUCCESS;

    if(enc_type > SM9_ENC_KDF_BLOCK_CIPHER)
    {
        ret = SM9_ERROR_S;
    }
    else if(SM9_ENC_KDF_BLOCK_CIPHER == enc_type)
    {
        if(padding_type > SKE_PKCS_5_7_PADDING)
        {
            ret = SM9_ERROR_S;
        }
        else if(0u != ((C_bytes-96u) & 0x0Fu))
        {
            ret = SM9_ERROR_S;
        }
        else
        {
            //nothing to do, just for static analysis.
        }
    }
    else
    {
        //nothing to do, just for static analysis.
    }

    if(PKE_SUCCESS == ret)
    {
        if((NULL == IDB) || (NULL == M) || (NULL == M_bytes) || (NULL == C) || (NULL == deB) || (0u == IDB_bytes))
        {
            ret = SM9_ERROR_S;
        }
        else if((M == C) || (C_bytes < (96u + 1u)))
        {
            ret = SM9_ERROR_S;
        }
        else if(K2_bytes > SM9_MAX_ENC_K2_BYTE_LEN)
        {
            ret = SM9_ERROR_S;
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


/* function: SM9 decrypt s internal step 2
 * parameters:
 *     C -------------------------- input, ciphertext to be decrypted
 *     C_bytes -------------------- input, bytes length of the ciphertext
 *     deB ------------------------ input, decapsulated key
 *     tmp_r ---------------------- output, temporary buffer for random number
 *     Qx ------------------------- output, x coordinate of the point Q
 *     Qy ------------------------- output, y coordinate of the point Q
 *     Px ------------------------- output, x coordinate of the point P
 *     Py ------------------------- output, y coordinate of the point P
 *     C2 ------------------------- output, pointer to pointer of uint8_t for ciphertext C2
 *     C2_bytes ------------------- output, pointer to the length of the ciphertext C2
 *     count ---------------------- input&output, pointer to volatile uint32_t counter
 *     curve_a -------------------- output, pointer to pointer of uint32_t for curve parameter a
 *     curve_b -------------------- output, pointer to pointer of uint32_t for curve parameter b
 *     curve_p -------------------- output, pointer to pointer of uint32_t for curve parameter p
 *     curve_p_h ------------------ output, pointer to pointer of uint32_t for curve parameter p's hash
 *     curve ---------------------- output, pointer to eccp_curve_st struct
 *     ctx ------------------------ output, pointer to eccp_sec_ctx_t struct
 * return: SM9_SUCCESS_S(success), SM9_ERROR_S(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_dec_s_internal_step_2(
    const uint8_t *C, 
    uint32_t C_bytes, 
    const uint8_t deB[128],
    uint32_t *tmp_r,
    uint32_t *Qx,
    uint32_t *Qy,
    uint32_t *Px,
    uint32_t *Py,
    const uint8_t **C2,
    uint32_t *C2_bytes,
    volatile uint32_t *count,
    uint32_t **curve_a, 
    uint32_t **curve_b, 
    uint32_t **curve_p, 
    uint32_t **curve_p_h,
    const eccp_curve_st **curve,
    eccp_sec_ctx_t ctx[1]
)
{
    uint16_t eccp_curve_crc16 = 0u;
    uint32_t ret = PKE_SUCCESS;

    *C2 = &(C[96]);
    *C2_bytes = C_bytes - 96u;

    (void)counter_add_one(count);

    //init sm9 curve
    *curve = eccp_curve_init(ctx, sm9_curve);
    if(NULL == curve)
    {
        ret = SM9_ERROR_S;
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //check crc16 of sm9 paras
        if(0U != ecc_crc16_check(*curve, eccp_curve_crc16))
        {
            ret = SM9_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //actually the following pointers are the same as the corresponding fields of curve
        *curve_p   = &(ctx->eccp_curve_mem[0]);
        *curve_p_h = &(ctx->eccp_curve_mem[SM9_BASE_WORD_LEN]);
        *curve_a   = &(ctx->eccp_curve_mem[SM9_BASE_WORD_LEN<<1]);
        *curve_b   = &(ctx->eccp_curve_mem[SM9_BASE_WORD_LEN*3u]);

        (void)counter_add_one(count);

        //check C1
        (void)sm9_fp_eccp_point_u8big_2_u32little(C, Px, Py);
        if(PKE_SUCCESS != eccp_check_point(*curve, Px, Py))
        {
            ret = SM9_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //copy back sm9 curve paras in PKE RAM that not erased by hardware
        uint32_copy_8_words(*curve_a, (uint32_t *)(rPKE_B(4u,SM9_STEPS)));
        uint32_copy_8_words(*curve_b, (uint32_t *)(rPKE_A(4u,SM9_STEPS)));
#if 0   //since p and p_h will be used by the following hardware calculation.
        uint32_copy_8_words(curve_p, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));
        uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM9_STEPS)));
#endif

        (void)counter_add_one(count);

        //check deB
        (void)sm9_fp2_eccp_point_u8big_2_u32little(deB, Qx, Qy);
        ret = sm9_fp2_pointVerify_internal(*curve, Qx, Qy);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //get random tmp_r in [1,n-1]
        do {
            ret = get_rand((uint8_t *)tmp_r, 32u);
            if(TRNG_SUCCESS != ret)
            {
                ret = SM9_ERROR_S;
                break;
            }
            else
            {}

            ret = uint32_integer_check_sec(tmp_r, (*curve)->eccp_n, SM9_BASE_WORD_LEN, SM9_ZERO_ALL, SM9_INTEGER_TOO_BIG, PKE_SUCCESS);
        } while(PKE_SUCCESS != ret);
    }
    else
    {}

    return ret;
}


/* function: SM9 decrypt s internal step 3
 * parameters:
 *     tmp_fp12g1 ----------------- output, temporary buffer for storing the result of pairing operation
 *     tmp_fp12g2 ----------------- output, temporary buffer for storing the result of pairing operation
 *     tmp_r ---------------------- input, pointer to the random number
 *     Qx ------------------------- input, x coordinate of the point Q
 *     Qy ------------------------- input, y coordinate of the point Q
 *     Px ------------------------- input, x coordinate of the point P
 *     Py ------------------------- input, y coordinate of the point P
 *     R1x ------------------------ output, x coordinate of the point R1
 *     R1y ------------------------ output, y coordinate of the point R1
 *     R2x ------------------------ output, x coordinate of the point R2
 *     R2y ------------------------ output, y coordinate of the point R2
 *     count ---------------------- input&output, pointer to volatile uint32_t counter
 *     curve_b -------------------- output, pointer to pointer of uint32_t for curve parameter b
 *     curve ---------------------- input, pointer to eccp_curve_st struct
 * return: SM9_SUCCESS_S(success), SM9_ERROR_S(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_dec_s_internal_step_3(
    uint32_t tmp_fp12g1[8u * 12u],  
    uint32_t tmp_fp12g2[8u * 12u],  
    const uint32_t *tmp_r,  
    const uint32_t *Qx,  
    const uint32_t *Qy,  
    const uint32_t *Px,  
    const uint32_t *Py,  
    uint32_t *R1x,  
    uint32_t *R1y,  
    uint32_t *R2x,  
    uint32_t *R2y,  
    volatile uint32_t *count,  
    uint32_t *curve_b,  
    const eccp_curve_st *curve
)
{
    uint32_t ret;

    (void)counter_add_one(count);

    // get deB1 (R1x, R1y) = [tmp_r]G2
    ret = sm9_fp2_pointMul_s_internal(curve, tmp_r, sm9_G2_Px0_Px1, sm9_G2_Py0_Py1, R1x, R1y);

    if(PKE_SUCCESS == ret)
    {
        //copy back sm9 curve paras in PKE RAM that not erased by hardware
        uint32_copy_8_words(curve_b, (uint32_t *)(rPKE_B(3u,SM9_STEPS)));
#if 0   //since p and p_h will be used by the following hardware calculation.
        uint32_copy_8_words(curve_p, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));
        uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM9_STEPS)));
#endif

        (void)counter_add_one(count);

        //(R1x, -R1y)
        uint32_copy(R2x, R1x, 16u);
#if 0
        ret = pke_sub(curve->eccp_p, R1y, R2y, SM9_BASE_WORD_LEN);
#else
        ret = pke_mod_add_sub_mul_256bits_internal(curve->eccp_p, R1y, R2y, MICROCODE_INTSUB);
#endif
    }
    else
    {}


    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

#if 0
        ret = pke_sub(curve->eccp_p, &R1y[SM9_BASE_WORD_LEN], &R2y[SM9_BASE_WORD_LEN], SM9_BASE_WORD_LEN);
#else
        ret = pke_mod_add_sub_mul_256bits_internal(curve->eccp_p, &R1y[SM9_BASE_WORD_LEN], 
                &R2y[SM9_BASE_WORD_LEN], MICROCODE_INTSUB);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //get deB = R1 + R2
        ret = sm9_fp2_pointAdd_internal(Qx, Qy, R2x, R2y, R2x, R2y);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        ret = sm9_pairing(Px, Py, R1x, R1y, tmp_fp12g1);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        ret = sm9_pairing(Px, Py, R2x, R2y, tmp_fp12g2);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        ret = sm9_fp12_mul(tmp_fp12g1, tmp_fp12g2, NULL);
    }
    else
    {}

    return ret;
}


/* function: SM9 decrypt s internal step 4
 * parameters:
 *     C_bytes -------------------- input, bytes length of the ciphertext
 *     enc_type ------------------- input, type of encryption (SM9_ENC_KDF_STREAM_CIPHER or SM9_ENC_KDF_BLOCK_CIPHER)
 *     padding_type --------------- input, type of padding(SKE_NO_PADDING or SKE_PKCS_5_7_PADDING)
 *     K2_bytes ------------------- input, bytes length of the key K2 in MAC function
 *     M -------------------------- output, message buffer to store the decrypted message
 *     M_bytes -------------------- output, pointer to the length of the message buffer
 *     counter_buf ---------------- input, buffer for storing the counter value
 *     K1 ------------------------- output, key K1 in internal encrypting
 *     K2 ------------------------- output, key K2 in internal MAC function
 *     Qx ------------------------- output, x coordinate of the point Q
 *     h2rf_para ------------------ output, buffer for storing the result of H2RF function
 *     C2 ------------------------- input, ciphertext buffer
 *     C2_bytes ------------------- input, bytes length of the ciphertext buffer
 *     count ---------------------- input&output, pointer to volatile uint32_t counter
 *     digest_node ---------------- input, array of hash nodes
 *     curve_p -------------------- output, pointer to pointer of uint32_t for curve parameter p
 *     curve_p_h ------------------ output, pointer to pointer of uint32_t for curve parameter p's hash
 * return: SM9_SUCCESS_S(success), SM9_ERROR_S(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_dec_s_internal_step_4(
    uint32_t C_bytes,  
    sm9_enc_type_e enc_type,  
    sm9_enc_padding_e padding_type,  
    uint32_t K2_bytes,  
    uint8_t *M,  
    uint32_t *M_bytes,  
    uint8_t counter_buf[4],  
    uint8_t K1[16],  
    uint8_t K2[SM9_MAX_ENC_K2_BYTE_LEN],  
    uint32_t *Qx,  
    uint32_t *h2rf_para,  
    const uint8_t *C2,  
    uint32_t C2_bytes,  
    volatile uint32_t *count,  
    hash_node_st digest_node[4],  
    uint32_t *curve_p,  
    uint32_t *curve_p_h
)
{
    uint32_t right_hand_operand_ret;
    uint32_t ret = PKE_SUCCESS;

    //copy back sm9 curve paras in PKE RAM that not erased by hardware
    uint32_copy_8_words(curve_p, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));
    uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM9_STEPS)));

    (void)counter_add_one(count);

    (void)sm9_fp12_in_pke_ram_u32little_2_u8big((uint8_t *)h2rf_para);

    (void)counter_add_one(count);

    if(SM9_ENC_KDF_STREAM_CIPHER == enc_type)  //KDF
    {
        ret = ansi_x9_63_kdf_node(HASH_SM3, digest_node, 4u, counter_buf, M, C2_bytes, K2, K2_bytes);
        if(HASH_SUCCESS == ret)
        {
            (void)counter_add_one(count);

            if(0u == uint8_BigNum_Check_Zero_sec(M, C2_bytes))
            {
                (void)counter_add_one(count);
                (void)uint8_XOR(M, C2, M, C2_bytes);
                (void)counter_add_one(count);

                *M_bytes = C2_bytes;
                
                ret = PKE_SUCCESS;
            }
            else
            {
                ret = SM9_ERROR_S;
            }
        }
        else
        {}
    }
    else if(SM9_ENC_KDF_BLOCK_CIPHER == enc_type) //SM4
    {
        if(0u == (C_bytes & 0x0Fu))
        {
            (void)counter_add_one(count);
            ret = ansi_x9_63_kdf_node(HASH_SM3, digest_node, 4u, counter_buf, K1, 16u, K2, K2_bytes);
            right_hand_operand_ret = uint8_BigNum_Check_Zero_sec(K1, 16u);
        }
        else
        {
            ret = SM9_ERROR_S;
        }


        if((HASH_SUCCESS == ret) && (0u == right_hand_operand_ret))
        {
            (void)counter_add_one(count);
        }
        else
        {
            ret = SM9_ERROR_S;
        }

        if(HASH_SUCCESS == ret)
        {
            (void)counter_add_one(count);

#if defined(SKE_HP)
            ret = ske_crypto(SKE_ALG_SM4, SKE_MODE_ECB, SKE_CRYPTO_DECRYPT, K1, 0u, NULL,
                padding_type, C2, M, C2_bytes, M_bytes);
#elif defined(SKE_LP)
            ret = ske_crypto(SKE_ALG_SM4, SKE_MODE_ECB, SKE_CRYPTO_DECRYPT, K1, 0u, NULL,
                padding_type, C2, M, C2_bytes, M_bytes);
#elif defined(SKE_SECURE)
            ret = ske_sec_crypto(SKE_ALG_SM4, SKE_MODE_ECB, SKE_CRYPTO_DECRYPT, K1, 0u, NULL,
                padding_type, C2, M, C2_bytes, M_bytes);
#else
            ret = SM9_INPUT_INVALID;
#endif
#if 0
            if(SKE_SUCCESS == ret)
            {
                ret = PKE_SUCCESS;
            }
            else
            {}
#endif
        }
        else
        {}
    }
    else
    {
        //nothing to do, just for static analysis.
    }

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //get u := MAC(K2, C2) := sm3(C2||K2)
        digest_node[0].msg_addr  = C2;
        digest_node[0].msg_bytes = C2_bytes;
        digest_node[1].msg_addr  = K2;
        digest_node[1].msg_bytes = K2_bytes;
        ret = hash_node_steps(HASH_SM3, digest_node, 2u, (uint8_t *)h2rf_para);
        if(HASH_SUCCESS == ret)
        {
            (void)counter_add_one(count);

            //check u = C3 ? for 3 times
            ret = get_rand((uint8_t *)Qx, 1u<<2);
#if 0
            if(TRNG_SUCCESS == ret)
            {
                ret = PKE_SUCCESS;
            }
            else
            {}
#endif
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: SM9 decrypt s internal step 5
 * parameters:
 *     C -------------------------- input, ciphertext to be decrypted
 *     Qx ------------------------- output, x coordinate of the point Q
 *     h2rf_para ------------------ output, buffer for storing the result of H2RF function
 *     count ---------------------- input&output, pointer to volatile uint32_t counter
 *     curve ---------------------- input, pointer to eccp_curve_st struct
 * return: SM9_SUCCESS_S(success), SM9_ERROR_S(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_dec_s_internal_step_5(
    const uint8_t *C,
    uint32_t *Qx,
    uint32_t *h2rf_para,
    volatile uint32_t *count,
    const eccp_curve_st *curve
)
{
    uint16_t eccp_curve_crc16 = 0u;
    uint32_t ret = PKE_SUCCESS;

    (void)counter_add_one(count);

    memcpy_((uint8_t *)&(h2rf_para[8]), &(C[64]), 32u);
    memcpy_((uint8_t *)&(h2rf_para[16]), &(C[64]), 32u);
    memcpy_((uint8_t *)&(h2rf_para[24]), &(C[64]), 32u);

    uint32_sleep(Qx[0] & 0x0Fu, (uint8_t)(Qx[0] >> 4));
    if(0u != uint32_cmp_sec(&(h2rf_para[8]), h2rf_para, 8u, (uint8_t)(Qx[0] >> 5)))
    {
        ret = SM9_ERROR_S;
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        Qx[0] >>= 8;
        uint32_sleep(Qx[0] & 0x0Fu, (uint8_t)(Qx[0] >> 4));
        if(0u != uint32_cmp_sec(&(h2rf_para[16]), h2rf_para, 8u, (uint8_t)(Qx[0] >> 5)))
        {
            ret = SM9_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        Qx[0] >>= 8;
        uint32_sleep(Qx[0] & 0x0Fu, (uint8_t)(Qx[0] >> 4));
        if(0u != uint32_cmp_sec(&(h2rf_para[24]), h2rf_para, 8u, (uint8_t)(Qx[0] >> 5)))
        {
            ret = SM9_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS== ret)
    {
        (void)counter_add_one(count);

        //check crc16 of sm9 paras
        if(0U != ecc_crc16_check(curve, eccp_curve_crc16))
        {
            ret = SM9_ERROR_S;
        }
        else
        {}
    }

    if(PKE_SUCCESS == ret)
    {
        if((*count) != (SM9_SEC_DEC_COUNTER + 0x17U))
        {
            ret = SM9_ERROR_S;
        }
        else
        {}
    }

    return ret;
}


/* function: SM9 decrypt(secure version)
 * parameters:
 *     IDB ------------------------ input, identity of user B, user B is the cipher receiver 
 *     IDB_bytes ------------------ input, bytes length of IDB
 *     C -------------------------- input, the cipher
 *     C_bytes -------------------- input, bytes length of the cipher
 *     deB ------------------------ input, user B's private key
 *     enc_type ------------------- input, type of encryption (SM9_ENC_KDF_STREAM_CIPHER or SM9_ENC_KDF_BLOCK_CIPHER)
 *     padding_type --------------- input, type of padding(SKE_NO_PADDING or SKE_PKCS_5_7_PADDING)
 *     K2_bytes ------------------- input, bytes length of the key in MAC function
 *     M -------------------------- output, the plain message
 *     M_bytes -------------------- output, bytes length of the plaintext message
 * return: SM9_SUCCESS_S(success), other(error)
 * caution:
 */
uint32_t sm9_dec_s(const uint8_t *IDB, uint32_t IDB_bytes, const uint8_t *C, uint32_t C_bytes, 
        const uint8_t deB[128], sm9_enc_type_e enc_type, sm9_enc_padding_e padding_type, 
        uint32_t K2_bytes, uint8_t *M, uint32_t *M_bytes)
{
    uint32_t ret = SM9_SUCCESS;
    uint8_t counter_buf[4] = {0,0,0,1};
    const uint8_t *counter = counter_buf;
    uint8_t K1[16];
    uint8_t K2[SM9_MAX_ENC_K2_BYTE_LEN];

    uint32_t tmp_fp12g1[8u*12u], tmp_fp12g2[8u*12u];
#if 0
    uint32_t tmp_r[8];
    uint32_t Px[8], Py[8], Qx[16], Qy[16];
    uint32_t R1x[16], R1y[16], R2x[16], R2y[16];
    uint8_t h2rf_para[32*12];
#else
#ifndef SUPPORT_STATIC_ANALYSIS
#define  tmp_r      ((uint32_t *)tmp_fp12g1)
#define  Qx         (((uint32_t *)tmp_fp12g1)+SM9_BASE_WORD_LEN)
#define  Qy         (((uint32_t *)tmp_fp12g1)+(3u*SM9_BASE_WORD_LEN))
#define  Px         ((uint32_t *)tmp_fp12g2)
#define  Py         (((uint32_t *)tmp_fp12g2)+SM9_BASE_WORD_LEN)
#define  R1x        (((uint32_t *)tmp_fp12g2)+(2u*SM9_BASE_WORD_LEN))
#define  R1y        (((uint32_t *)tmp_fp12g2)+(4u*SM9_BASE_WORD_LEN))
#define  R2x        (((uint32_t *)tmp_fp12g2)+(6u*SM9_BASE_WORD_LEN))
#define  R2y        (((uint32_t *)tmp_fp12g2)+(8u*SM9_BASE_WORD_LEN))
#define  h2rf_para  ((uint32_t *)tmp_fp12g2)
#else
uint32_t *tmp_r     = tmp_fp12g1;
uint32_t *Qx        = &tmp_fp12g1[SM9_BASE_WORD_LEN];
uint32_t *Qy        = &tmp_fp12g1[3u*SM9_BASE_WORD_LEN];
uint32_t *Px        = tmp_fp12g2;
uint32_t *Py        = &tmp_fp12g2[SM9_BASE_WORD_LEN];
uint32_t *R1x       = &tmp_fp12g2[2u*SM9_BASE_WORD_LEN];
uint32_t *R1y       = &tmp_fp12g2[4u*SM9_BASE_WORD_LEN];
uint32_t *R2x       = &tmp_fp12g2[6u*SM9_BASE_WORD_LEN];
uint32_t *R2y       = &tmp_fp12g2[8u*SM9_BASE_WORD_LEN];
uint32_t *h2rf_para = tmp_fp12g2;
#endif
#endif

    const uint8_t *C2;
    uint32_t C2_bytes;
    volatile uint32_t count = SM9_SEC_DEC_COUNTER;

    hash_node_st digest_node[4];

    uint32_t *curve_a, *curve_b, *curve_p, *curve_p_h;
    const eccp_curve_st *curve;
    eccp_sec_ctx_t ctx[1];

    digest_node[0].msg_addr  = C;
    digest_node[0].msg_bytes = 64u;
    digest_node[1].msg_addr  = (uint8_t *)h2rf_para;
    digest_node[1].msg_bytes = 32u*12u;
    digest_node[2].msg_addr  = IDB;
    digest_node[2].msg_bytes = IDB_bytes;
    digest_node[3].msg_addr  = counter;
    digest_node[3].msg_bytes = 4u;

    ret = sm9_dec_s_internal_step_1(IDB, IDB_bytes, C, C_bytes, deB, enc_type, padding_type, K2_bytes, M, M_bytes);
    if(PKE_SUCCESS == ret)
    {
        ret = sm9_dec_s_internal_step_2(C, C_bytes, deB, tmp_r, Qx, Qy, Px, Py, &C2, &C2_bytes, 
            &count, &curve_a,  &curve_b,  &curve_p,  &curve_p_h, &curve, ctx);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_dec_s_internal_step_3(tmp_fp12g1, tmp_fp12g2, tmp_r, Qx, Qy, Px, Py, 
                R1x, R1y, R2x, R2y, &count, curve_b, curve);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_dec_s_internal_step_4(C_bytes, enc_type, padding_type, K2_bytes, M, M_bytes, 
            counter_buf, K1, K2, Qx, h2rf_para, C2, C2_bytes, &count, digest_node, curve_p, curve_p_h);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_dec_s_internal_step_5(C, Qx, h2rf_para, &count, curve);
    }
    else
    {}

#ifndef SUPPORT_STATIC_ANALYSIS
#undef  tmp_r
#undef  Qx
#undef  Qy
#undef  Px
#undef  Py
#undef  R1x
#undef  R1y
#undef  R2x
#undef  R2y
#undef  h2rf_para
#endif

    if(PKE_SUCCESS != ret)
    {
        ret = SM9_ERROR_S;

        if (C_bytes > 96u) {
            memset_(M, 0, C_bytes-96u);
            *M_bytes = 0u;
        }
    }
    else
    {
        ret = SM9_SUCCESS_S;
    }

    (void)get_rand_fast((uint8_t *)counter_buf, sizeof(counter_buf));
    (void)get_rand_fast((uint8_t *)K1, sizeof(K1));
    (void)get_rand_fast((uint8_t *)K2, sizeof(K2));
    (void)get_rand_fast((uint8_t *)tmp_fp12g1, sizeof(tmp_fp12g1));
    (void)get_rand_fast((uint8_t *)tmp_fp12g2, sizeof(tmp_fp12g2));
    (void)get_rand_fast((uint8_t *)digest_node, sizeof(digest_node));
    (void)get_rand_fast((uint8_t *)ctx, sizeof(eccp_sec_ctx_t));
    curve = NULL;

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
    uint32_t ret = PKE_SUCCESS;

    if((NULL == IDA) || (NULL == IDB) || (NULL == deA) || (NULL == rA) || (0u == k_bytes))
    {
        ret = SM9_ERROR_S;
    }
    else if((NULL == RA) || (NULL == RB) || (NULL == k))
    {
        ret = SM9_ERROR_S;
    }
    else if(role > SM9_Role_Responsor)
    {
        ret = SM9_ERROR_S;
    }
    else if((0u == IDA_bytes) || (0u == IDB_bytes))
    {
        ret = SM9_ERROR_S;
    }
    else
    {
        //nothing to do, just for static analysis.
    }

    return ret;
}


/* function: SM9 Key exchange step 1(internal API)
 * parameters:
 *      curve -------------- input, a pointer to a pointer to the ECCP curve structure.
 *      ctx   -------------- input, a pointer to the ECCP security context structure.
 *      RA    -------------- input, a 64-byte array containing the x and y coordinates of the point RA.
 *      RB ----------------- input, a 64-byte array containing the x and y coordinates of the point RB.
 *      count -------------- input&output, a pointer to a volatile 32-bit integer used for counting operations.
 *      curve_a ------------ output, a pointer to a 32-bit integer that will receive the value of the curve parameter 'a'.
 *      curve_b ------------ output, a pointer to a 32-bit integer that will receive the value of the curve parameter 'b'.
 *      curve_p ------------ output, a pointer to a 32-bit integer that will receive the value of the curve parameter 'p'.
 *      curve_p_h ---------- output, a pointer to a 32-bit integer that will receive the value of the curve parameter 'p_h'.
 *      eccp_curve_crc16 --- input, the CRC16 value of the ECCP curve parameters.
 *      RBx ---------------- output, a pointer to a 32-bit integer that will receive the x coordinate of the point RB.
 *      RBy ---------------- output, a pointer to a 32-bit integer that will receive the y coordinate of the point RB.
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_exchangekey_s_internal_step_1(const eccp_curve_st **curve, eccp_sec_ctx_t *ctx, const uint8_t RA[64], const uint8_t RB[64], 
    volatile uint32_t *count, uint32_t **curve_a, uint32_t **curve_b, uint32_t **curve_p, uint32_t **curve_p_h, uint16_t eccp_curve_crc16, uint32_t *RBx,
    uint32_t *RBy)
{
    uint32_t ret = PKE_SUCCESS;

    (void)counter_add_one(count);

    //init sm9 curve
    *curve = eccp_curve_init(ctx, sm9_curve);
    if(NULL == (*curve))
    {
        ret = SM9_ERROR_S;
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //check crc16 of sm9 paras
        if(0U != ecc_crc16_check(*curve, eccp_curve_crc16))
        {
            ret = SM9_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //actually the following pointers are the same as the corresponding fields of curve
        *curve_p   = &(ctx->eccp_curve_mem[0]);
        *curve_p_h = &(ctx->eccp_curve_mem[SM9_BASE_WORD_LEN]);
        *curve_a   = &(ctx->eccp_curve_mem[SM9_BASE_WORD_LEN<<1]);
        *curve_b   = &(ctx->eccp_curve_mem[SM9_BASE_WORD_LEN*3u]);

        (void)counter_add_one(count);

        //just to check RA
        (void)sm9_fp_eccp_point_u8big_2_u32little(RA, RBx, RBy);
        ret = eccp_check_point(*curve, RBx, RBy);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //copy back sm9 curve paras in PKE RAM that not erased by hardware
        uint32_copy_8_words(*curve_a, (uint32_t *)(rPKE_B(4u,SM9_STEPS)));
        uint32_copy_8_words(*curve_b, (uint32_t *)(rPKE_A(4u,SM9_STEPS)));
#if 0   //since p and p_h will be used by the following hardware calculation.
        uint32_copy_8_words(curve_p, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));
        uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM9_STEPS)));
#endif

        (void)counter_add_one(count);

        //check RB
        (void)sm9_fp_eccp_point_u8big_2_u32little(RB, RBx, RBy);
        ret = eccp_check_point_internal(*curve, RBx, RBy);
    }
    else
    {}

    return ret;
}


/* function: SM9 Key exchange step 2(internal API)
 * parameters:
 *      curve_a ------------ input&output, a pointer to a 32-bit integer that will receive the value of the curve parameter 'a'.
 *      curve_b ------------ input&output, a pointer to a 32-bit integer that will receive the value of the curve parameter 'b'.
 *      count -------------- input&output, a pointer to a volatile 32-bit integer used for counting operations.
 *      fp12g -------------- input, a pointer to a 32-bit integer that will receive the value of the pairing result.
 *      crc_g -------------- input&output, a pointer to a volatile 16-bit integer that will receive the CRC16 value of the pairing result.
 *      Ppub_e ------------- input, a 64-byte array containing the x and y coordinates of the point Ppub_e.
 *      fp12g_bak ---------- output, a pointer to a 32-bit integer that will receive the backup value of the pairing result.
 *      curve -------------- input, a pointer to the ECCP curve structure.
 *      Px ----------------- output, a pointer to a 32-bit integer that will receive the x coordinate of the point P.
 *      Py ----------------- output, a pointer to a 32-bit integer that will receive the y coordinate of the point P.
 *      rA ----------------- input, a 32-byte array containing the temporary private key of the local user.
 *      tmp ---------------- output, a 8-byte array used for temporary storage.
 *      fp12 --------------- output, a pointer to a 32-bit integer that will receive the value of the pairing result.
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_exchangekey_s_internal_step_2(uint32_t *curve_a, uint32_t *curve_b, 
    volatile uint32_t *count, const uint8_t *fp12g, volatile uint16_t *crc_g, const uint8_t Ppub_e[64],
    uint32_t *fp12g_bak, const eccp_curve_st *curve, uint32_t *Px, uint32_t *Py, const uint8_t rA[32], uint32_t tmp[8],
    uint32_t *fp12)
{
    uint32_t ret = PKE_SUCCESS;

    //copy back sm9 curve paras in PKE RAM that not erased by hardware
    uint32_copy_8_words(curve_a, (uint32_t *)(rPKE_B(4u,SM9_STEPS)));
    uint32_copy_8_words(curve_b, (uint32_t *)(rPKE_A(4u,SM9_STEPS)));
#if 0   //since p and p_h will be used by the following hardware calculation.
    uint32_copy_8_words(curve_p, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));
    uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM9_STEPS)));
#endif

    (void)counter_add_one(count);

    if(NULL != fp12g)
    {
        (void)counter_add_one(count);

        *crc_g = crc16_calc(fp12g, 32u*12u, (uint16_t)0xFFFF);

        (void)counter_add_one(count);
        (void)sm9_fp12_u8big_2_u32little(fp12g, (uint32_t *)fp12g_bak);
    }
    else if(NULL != Ppub_e)
    {
        //check Ppub_e
        (void)sm9_fp_eccp_point_u8big_2_u32little(Ppub_e, Px, Py);
        ret = eccp_check_point_internal(curve, Px, Py);

        if(PKE_SUCCESS == ret)
        {
            //copy back sm9 curve paras in PKE RAM that not erased by hardware
            uint32_copy_8_words(curve_a, (uint32_t *)(rPKE_B(4u,SM9_STEPS)));
            uint32_copy_8_words(curve_b, (uint32_t *)(rPKE_A(4u,SM9_STEPS)));
#if 0       //since p and p_h will be used by the following hardware calculation.
            uint32_copy_8_words(curve_p, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));
            uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM9_STEPS)));
#endif
            (void)counter_add_one(count);

            ret = sm9_pairing(Px, Py, sm9_G2_Px0_Px1, sm9_G2_Py0_Py1, fp12g_bak);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            (void)counter_add_one(count);

            *crc_g = crc16_calc((uint8_t *)fp12g_bak, 32u*12u, (uint16_t)0xFFFF);
        }
        else
        {}
    }
    else
    {
        ret = SM9_ERROR_S;
    }

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //check rA
        u8big_to_u32little_256bits(rA, tmp);
        ret = uint32_integer_check_sec(tmp, sm9_curve->eccp_n, SM9_BASE_WORD_LEN, SM9_ZERO_ALL, SM9_INTEGER_TOO_BIG, PKE_SUCCESS);
        if(PKE_SUCCESS == ret)
        {
            (void)counter_add_one(count);

            ret = sm9_fp12_exp_s(fp12g_bak, tmp, fp12);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            (void)counter_add_one(count);

            //check rA again
            if((int32_t)0 != uint32_BigNumCmp_sec(tmp, SM9_BASE_WORD_LEN, (uint32_t *)rPKE_A(1u,SM9_STEPS), SM9_BASE_WORD_LEN))
            {
                ret = SM9_ERROR_S;
            }
            else
            {}
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: SM9 Key exchange step 3 get pairing(internal API)
 * parameters:
 *      count -------------- input&output, a pointer to a volatile 32-bit integer used for counting operations.
 *      Qx ----------------- output, a pointer to a 32-bit integer that will receive the x coordinate of the point Q.
 *      Qy ----------------- output, a pointer to a 32-bit integer that will receive the y coordinate of the point Q.
 *      curve -------------- input, a pointer to the ECCP curve structure.
 *      Q2y ---------------- output, a pointer to a 32-bit integer that will receive the y coordinate of the point Q2.
 *      Q1x ---------------- output, a pointer to a 32-bit integer that will receive the x coordinate of the point Q1.
 *      Q1y ---------------- output, a pointer to a 32-bit integer that will receive the y coordinate of the point Q1.
 *      curve_b ------------ input&output, a pointer to a 32-bit integer that will receive the value of the curve parameter 'b'.
 *      RBx ---------------- input, a pointer to a 32-bit integer that will receive the x coordinate of the point RB.
 *      RBy ---------------- input, a pointer to a 32-bit integer that will receive the y coordinate of the point RB.
 *      fp12_2 ------------- output, a pointer to a 32-bit integer that will receive the value of the pairing result.
 *      fp12_3 ------------- output, a pointer to a 32-bit integer that will receive the value of the pairing result.
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_exchangekey_s_internal_step_3_get_pairing(volatile uint32_t *count, 
    uint32_t *Qx, uint32_t *Qy, const eccp_curve_st *curve, uint32_t *Q2y, uint32_t *Q1x, uint32_t *Q1y,
    uint32_t *curve_b, const uint32_t *RBx, const uint32_t *RBy, uint32_t *fp12_2, uint32_t *fp12_3)
{
    uint32_t ret;

    (void)counter_add_one(count);

    // get (Q1x, Q1y) = [tmp_r]G2
    ret = sm9_fp2_pointMul_s_internal(curve, Q2y, sm9_G2_Px0_Px1, sm9_G2_Py0_Py1, Q1x, Q1y);

    if(PKE_SUCCESS == ret)
    {
        //copy back sm9 curve paras in PKE RAM that not erased by hardware
        uint32_copy_8_words(curve_b, (uint32_t *)(rPKE_B(3u,SM9_STEPS)));
#if 0   //since p and p_h will be used by the following hardware calculation.
        uint32_copy_8_words(curve_p, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));
        uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM9_STEPS)));
#endif

        (void)counter_add_one(count);

        //Q2y = -Q1y
#if 0
        ret = pke_sub(curve->eccp_p, Q1y, Q2y, SM9_BASE_WORD_LEN);
#else
        ret = pke_mod_add_sub_mul_256bits_internal(curve->eccp_p, Q1y, Q2y, MICROCODE_INTSUB);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);
#if 0
        ret = pke_sub(curve->eccp_p, &Q1y[SM9_BASE_WORD_LEN], &Q2y[SM9_BASE_WORD_LEN], SM9_BASE_WORD_LEN);
#else
        ret = pke_mod_add_sub_mul_256bits_internal(curve->eccp_p, &Q1y[SM9_BASE_WORD_LEN], 
                &Q2y[SM9_BASE_WORD_LEN], MICROCODE_INTSUB);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //get deA = Q1 + Q
        ret = sm9_fp2_pointAdd_internal(Qx, Qy, Q1x, Q2y, Qx, Qy);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        ret = sm9_pairing(RBx, RBy, Q1x, Q1y, fp12_2);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        ret = sm9_pairing(RBx, RBy, Qx, Qy, fp12_3);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        ret = sm9_fp12_mul(fp12_2, fp12_3, fp12_2);
    }
    else
    {}

    return ret;
}


/* function: SM9 Key exchange step 3(internal API)
 * parameters:
 *      count -------------- input&output, a pointer to a volatile 32-bit integer used for counting operations.
 *      deA ---------------- input, a 128-byte array containing the decapsulated key.
 *      Qx ----------------- output, a pointer to a 32-bit integer that will receive the x coordinate of the point Q.
 *      Qy ----------------- output, a pointer to a 32-bit integer that will receive the y coordinate of the point Q.
 *      curve -------------- input, a pointer to the ECCP curve structure.
 *      Q2y ---------------- output, a pointer to a 32-bit integer that will receive the y coordinate of the point Q2.
 *      Q1x ---------------- output, a pointer to a 32-bit integer that will receive the x coordinate of the point Q1.
 *      Q1y ---------------- output, a pointer to a 32-bit integer that will receive the y coordinate of the point Q1.
 *      curve_b ------------ input&output, a pointer to a 32-bit integer that will receive the value of the curve parameter 'b'.
 *      RBx ---------------- input, a pointer to a 32-bit integer that will receive the x coordinate of the point RB.
 *      RBy ---------------- input, a pointer to a 32-bit integer that will receive the y coordinate of the point RB.
 *      fp12_2 ------------- output, a pointer to a 32-bit integer that will receive the value of the pairing result.
 *      fp12_3 ------------- output, a pointer to a 32-bit integer that will receive the value of the pairing result.
 *      tmp ---------------- input, a 8-byte array used for temporary storage.
 *      curve_p ------------ output, a pointer to a 32-bit integer that will receive the value of the curve parameter 'p'.
 *      curve_p_h ---------- output, a pointer to a 32-bit integer that will receive the value of the curve parameter 'p' hash.
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_exchangekey_s_internal_step_3(volatile uint32_t *count, const uint8_t deA[128], 
    uint32_t *Qx, uint32_t *Qy, const eccp_curve_st *curve, uint32_t *Q2y, uint32_t *Q1x, uint32_t *Q1y,
    uint32_t *curve_b, const uint32_t *RBx, const uint32_t *RBy, uint32_t *fp12_2, uint32_t *fp12_3, const uint32_t tmp[8],
    uint32_t *curve_p, uint32_t *curve_p_h)
{
    uint32_t ret;

    (void)counter_add_one(count);

    //check deA
    (void)sm9_fp2_eccp_point_u8big_2_u32little(deA, Qx, Qy);
    ret = sm9_fp2_check_point_internal(curve, Qx, Qy);

    /******************** e(RB, deA) **********************/
    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //get random tmp_r in [1,n-1]
        do {
            ret = get_rand((uint8_t *)Q2y, 32u);
            if(TRNG_SUCCESS != ret)
            {
                ret = SM9_ERROR_S;
                break;
            }
            else
            {}

            ret = uint32_integer_check_sec(Q2y, curve->eccp_n, SM9_BASE_WORD_LEN, SM9_ZERO_ALL, SM9_INTEGER_TOO_BIG, PKE_SUCCESS);
        } while(PKE_SUCCESS != ret);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_exchangekey_s_internal_step_3_get_pairing(count, Qx, Qy, curve, Q2y, Q1x, Q1y, curve_b, RBx, 
            RBy, fp12_2, fp12_3);
    }
    else
    {}
    /******************** e(RB, deA) end **********************/

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        ret = sm9_fp12_exp_s(fp12_2, tmp, fp12_3);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //copy back sm9 curve paras in PKE RAM that not erased by hardware
        uint32_copy_8_words(curve_p, (uint32_t *)(rPKE_A(0u,SM9_STEPS)));
        uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM9_STEPS)));

        (void)counter_add_one(count);

        //check rA again
        if((int32_t)0 != uint32_BigNumCmp_sec(tmp, SM9_BASE_WORD_LEN, (uint32_t *)rPKE_A(1u,SM9_STEPS), SM9_BASE_WORD_LEN))
        {
            ret = SM9_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: SM9 Key exchange step 4 get S1 and SA(internal API)
 * parameters:
 *      count -------------- input&output, a pointer to a volatile 32-bit integer used for counting operations.
 *      fp12_2 ------------- input, a pointer to a 32-bit integer that will receive the value of the pairing result.
 *      fp12_3 ------------- input, a pointer to a 32-bit integer that will receive the value of the pairing result.
 *      fp12 --------------- input, a pointer to a 32-bit integer that will receive the value of the pairing result.
 *      IDA ---------------- input, a pointer to a 8-bit integer that will receive the identity of user A.
 *      IDB ---------------- input, a pointer to a 8-bit integer that will receive the identity of user B.
 *      RA ----------------- input, a 64-byte array containing the x and y coordinates of the point RA.
 *      RB ----------------- input, a 64-byte array containing the x and y coordinates of the point RB.
 *      role --------------- input, a variable of type sm9_exchange_role_e that will receive the role of the user.
 *      IDA_bytes ---------- input, a variable of type uint32_t that will receive the length of the identity of user A.
 *      IDB_bytes ---------- input, a variable of type uint32_t that will receive the length of the identity of user B.
 *      digest_node -------- input&output, an array of hash nodes.
 *      tag ---------------- output, a pointer to a 8-bit integer that will receive the tag value.
 *      S1 ----------------- output, a 32-byte array that will receive the value of S1.
 *      SA ----------------- output, a 32-byte array that will receive the value of SA.
 *      sponsor_g1 --------- input, a pointer to a 8-bit integer that will receive the sponsor's g1 value.
 *      tmp ---------------- output, a 8-byte array used for temporary storage.
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_exchangekey_s_internal_step_4_get_S1_SA(volatile uint32_t *count, const uint32_t *fp12_2, const uint32_t *fp12_3,
    const uint32_t *fp12, const uint8_t *IDA, const uint8_t *IDB, const uint8_t RA[64], const uint8_t RB[64], sm9_exchange_role_e role,
    uint32_t IDA_bytes, uint32_t IDB_bytes, hash_node_st digest_node[8],
    uint8_t *tag, uint8_t S1[32], uint8_t SA[32], const uint8_t *sponsor_g1,
    uint32_t tmp[8])
{
    const uint8_t *sponsor_g1_bak = sponsor_g1;
    uint32_t ret;
    
    if(SM9_Role_Sponsor == role)
    {
        sponsor_g1_bak = (const uint8_t *)fp12;

        //tmp := Hash(g2||g3||IDA||IDB||RA||RB))
        digest_node[0].msg_addr  = (const uint8_t *)fp12_2;
        digest_node[1].msg_addr  = (const uint8_t *)fp12_3;
        digest_node[2].msg_addr  = IDA;
        digest_node[2].msg_bytes = IDA_bytes;
        digest_node[3].msg_addr  = IDB;
        digest_node[3].msg_bytes = IDB_bytes;
        digest_node[4].msg_addr  = RA;
        digest_node[5].msg_addr  = RB;

        (void)counter_add_one(count);
    }
    else if(SM9_Role_Responsor == role)
    {
        sponsor_g1_bak = (const uint8_t *)fp12_2;

        //tmp = Hash(g1||g3||IDB||IDA||RB||RA)), here g1 is responsor's g1, it is expected to be sponsor's g2
        digest_node[0].msg_addr  = (const uint8_t *)fp12;
        digest_node[1].msg_addr  = (const uint8_t *)fp12_3;
        digest_node[2].msg_addr  = IDB;
        digest_node[2].msg_bytes = IDB_bytes;
        digest_node[3].msg_addr  = IDA;
        digest_node[3].msg_bytes = IDA_bytes;
        digest_node[4].msg_addr  = RB;
        digest_node[5].msg_addr  = RA;

        (void)counter_add_one(count);
    }
    else
    {
        //nothing to do, just for static analysis.
    }

    digest_node[0].msg_bytes = 32u*12u;
    digest_node[1].msg_bytes = 32u*12u;
    digest_node[4].msg_bytes = 64u;
    digest_node[5].msg_bytes = 64u;

    ret = hash_node_steps(HASH_SM3, digest_node, 6u, (uint8_t *)tmp);
    if(HASH_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        digest_node[0].msg_addr  = tag;
        digest_node[0].msg_bytes = 1u;
        digest_node[1].msg_addr  = sponsor_g1_bak;
        digest_node[1].msg_bytes = 32u*12u;
        digest_node[2].msg_addr  = (uint8_t *)tmp;
        digest_node[2].msg_bytes = 32u;

        (void)counter_add_one(count);

        *tag = (uint8_t)0x82;
        if(SM9_Role_Sponsor == role)
        {
            ret = hash_node_steps(HASH_SM3, digest_node, 3u, S1);
        }
        else
        {}

        if(SM9_Role_Responsor == role)
        {
            ret = hash_node_steps(HASH_SM3, digest_node, 3u, SA);
        }
        else
        {}
    }
    else
    {}

    if(HASH_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        *tag = (uint8_t)0x83;
        if(SM9_Role_Sponsor == role)
        {
            ret = hash_node_steps(HASH_SM3, digest_node, 3u, SA);
        }
        else
        {}
        
        if(SM9_Role_Responsor == role)
        {
            ret = hash_node_steps(HASH_SM3, digest_node, 3u, S1);
        }
        else
        {}
    }
    else
    {}
#if 0
    if(HASH_SUCCESS == ret)
    {
        ret = PKE_SUCCESS;
    }
    else
    {}
#endif

    return ret;
}


/* function: SM9 Key exchange step 4(internal API)
 * parameters:
 *      count -------------- input&output, a pointer to a volatile 32-bit integer used for counting operations.
 *      fp12_2 ------------- input, a pointer to a 32-bit integer that will receive the value of the pairing result.
 *      fp12_3 ------------- input, a pointer to a 32-bit integer that will receive the value of the pairing result.
 *      fp12 --------------- input, a pointer to a 32-bit integer that will receive the value of the pairing result.
 *      IDA ---------------- input, a pointer to a 8-bit integer that will receive the identity of user A.
 *      IDB ---------------- input, a pointer to a 8-bit integer that will receive the identity of user B.
 *      RA ----------------- input, a 64-byte array containing the x and y coordinates of the point RA.
 *      RB ----------------- input, a 64-byte array containing the x and y coordinates of the point RB.
 *      role --------------- input, a variable of type sm9_exchange_role_e that will receive the role of the user.
 *      IDA_bytes ---------- input, a variable of type uint32_t that will receive the length of the identity of user A.
 *      IDB_bytes ---------- input, a variable of type uint32_t that will receive the length of the identity of user B.
 *      digest_node -------- input&output, an array of hash nodes.
 *      counter_buf -------- output, a 1-byte array used for temporary storage.
 *      counter ------------ input, a pointer to a 8-bit integer that will receive the counter value.
 *      tag ---------------- output, a pointer to a 8-bit integer that will receive the tag value.
 *      S1 ----------------- output, a 32-byte array that will receive the value of S1.
 *      SA ----------------- output, a 32-byte array that will receive the value of SA.
 *      sponsor_g1 --------- input, a pointer to a 8-bit integer that will receive the sponsor's g1 value.
 *      k ------------------ output, a pointer to a 8-bit integer that will receive the key value.
 *      k_bytes ------------ input, a variable of type uint32_t that will receive the length of the key.
 *      tmp ---------------- output, a 8-byte array used for temporary storage.
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_exchangekey_s_internal_step_4(volatile uint32_t *count, uint32_t *fp12_2, uint32_t *fp12_3,
    uint32_t *fp12, const uint8_t *IDA, const uint8_t *IDB, const uint8_t RA[64], const uint8_t RB[64], sm9_exchange_role_e role,
    uint32_t IDA_bytes, uint32_t IDB_bytes, hash_node_st digest_node[8], uint32_t counter_buf[1],
    const uint8_t *counter, uint8_t *tag, uint8_t S1[32], uint8_t SA[32], const uint8_t *sponsor_g1,
    uint8_t *k, uint32_t k_bytes, uint32_t tmp[8])
{
    uint32_t ret;

    (void)counter_add_one(count);

    (void)sm9_fp12_u32little_2_u8big(fp12);
    (void)sm9_fp12_u32little_2_u8big(fp12_2);
    (void)sm9_fp12_u32little_2_u8big(fp12_3);

    (void)counter_add_one(count);

    if(SM9_Role_Sponsor == role)
    {
        digest_node[0].msg_addr  = IDA;
        digest_node[0].msg_bytes = IDA_bytes;
        digest_node[1].msg_addr  = IDB;
        digest_node[1].msg_bytes = IDB_bytes;
        digest_node[2].msg_addr  = RA;
        digest_node[3].msg_addr  = RB;
        digest_node[4].msg_addr  = (uint8_t *)fp12;
        digest_node[5].msg_addr  = (uint8_t *)fp12_2;
        digest_node[6].msg_addr  = (uint8_t *)fp12_3;

        (void)counter_add_one(count);
    }
    else if(SM9_Role_Responsor == role)
    {
        digest_node[0].msg_addr  = IDB;
        digest_node[0].msg_bytes = IDB_bytes;
        digest_node[1].msg_addr  = IDA;
        digest_node[1].msg_bytes = IDA_bytes;
        digest_node[2].msg_addr  = RB;
        digest_node[3].msg_addr  = RA;
        digest_node[4].msg_addr  = (uint8_t *)fp12_2;
        digest_node[5].msg_addr  = (uint8_t *)fp12;
        digest_node[6].msg_addr  = (uint8_t *)fp12_3;

        (void)counter_add_one(count);
    }
    else
    {
        //nothing to do, just for static analysis.
    }

    digest_node[2].msg_bytes = 64u;
    digest_node[3].msg_bytes = 64u;
    digest_node[4].msg_bytes = 32u*12u;
    digest_node[5].msg_bytes = 32u*12u;
    digest_node[6].msg_bytes = 32u*12u;

    //key := kdf(IDA||IDB||RA||RB||g1||g2||g3, key_bytes)
    ((uint8_t *)counter_buf)[3] = (uint8_t)1u;
    digest_node[7].msg_addr  = counter;
    digest_node[7].msg_bytes = 4u;
    ret = ansi_x9_63_kdf_node(HASH_SM3, digest_node, 8u, (uint8_t *)counter_buf, k, k_bytes, NULL, 0u);
    if(HASH_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //check value is optional
        if((NULL != S1) && (NULL != SA))
        {
            ret = sm9_exchangekey_s_internal_step_4_get_S1_SA(count, fp12_2, fp12_3, fp12, IDA, IDB, RA, RB, 
                role, IDA_bytes, IDB_bytes, digest_node, tag, S1, SA, sponsor_g1, tmp);
        }
        else
        {
            (void)counter_add_one(count);
            (void)counter_add_one(count);
            (void)counter_add_one(count);
            (void)counter_add_one(count);
        }
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);
    }
    else
    {}

    return ret;
}


/* function: SM9 Key exchange step 5(internal API)
 * parameters:
 *      count -------------- input&output, a pointer to a volatile 32-bit integer used for counting operations.
 *      eccp_curve_crc16 --- input, a variable of type uint16_t that will receive the CRC16 value of the ECCP curve.
 *      curve -------------- input, a pointer to the ECCP curve structure.
 *      fp12g -------------- input, a pointer to a 8-bit integer that will receive the value of the pairing result.
 *      crc_g2 ------------- output, a pointer to a 16-bit integer that will receive the CRC16 value of the pairing result.
 *      crc_g -------------- input, a pointer to a 16-bit integer that will receive the CRC16 value of the pairing result.
 *      fp12g_bak ---------- input, a pointer to a 32-bit integer that will receive the backup value of the pairing result.
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 */
FLAG_STATIC uint32_t sm9_exchangekey_s_internal_step_5(volatile uint32_t *count, uint16_t eccp_curve_crc16,
    const eccp_curve_st *curve, const uint8_t *fp12g, uint16_t *crc_g2, const uint16_t *crc_g, const uint32_t *fp12g_bak)
{
    uint32_t ret = PKE_SUCCESS;

    //check crc16 of sm9 paras
    if(0U != ecc_crc16_check(curve, eccp_curve_crc16))
    {
        ret = SM9_ERROR_S;
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        // calc crc of g
        if(NULL != fp12g)
        {
            *crc_g2 = crc16_calc(fp12g, 32u*12u, (uint16_t)0xFFFF);
        }
        else
        {
            *crc_g2 = crc16_calc((const uint8_t *)fp12g_bak, 32u*12u, (uint16_t)0xFFFF);
        }

        (void)counter_add_one(count);

        if((*crc_g) != (*crc_g2))
        {
            ret = SM9_ERROR_S;
        }
        else if((uint16_t)0 != ((*crc_g) ^ (*crc_g2)))
        {
            ret = SM9_ERROR_S;
        }
        else if((uint16_t)0 != ((*crc_g) - (*crc_g2)))
        {
            ret = SM9_ERROR_S;
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
        if((*count) != (SM9_SEC_EXC_COUNTER + 0x21U))
        {
            ret = SM9_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: SM9 Key exchange(secure version)
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
 * return: SM9_SUCCESS_S(success), other(error)
 * caution:
 */
uint32_t sm9_exchangekey_s(sm9_exchange_role_e role,
                        const uint8_t *IDA, uint32_t IDA_bytes,
                        const uint8_t *IDB, uint32_t IDB_bytes,
                        const uint8_t *fp12g,
                        const uint8_t Ppub_e[64],
                        const uint8_t deA[128], const uint8_t rA[32],
                        const uint8_t RA[64], const uint8_t RB[64],
                        uint32_t k_bytes,
                        uint8_t *k, uint8_t S1[32], uint8_t SA[32])
{
    uint32_t counter_buf[1] = {0u};
    const uint8_t *counter = (uint8_t *)counter_buf;
    uint32_t fp12g_bak[8u*12u];
    uint32_t fp12[8u*12u];
    uint32_t fp12_2[8u*12u];
    uint32_t fp12_3[8u*12u];
    const uint8_t *sponsor_g1 = (uint8_t *)fp12;

    hash_node_st digest_node[8];

#if 0
    uint32_t Px[8], Py[8], Qx[16], Qy[16], Q1x[16], Q1y[16], RBx[8], RBy[8];
#else
#ifndef SUPPORT_STATIC_ANALYSIS
#define Px    ((uint32_t *)digest_node)
#define Py    (((uint32_t *)digest_node) + SM9_BASE_WORD_LEN)
#define Qx    (fp12_3)
#define Qy    (fp12_3 + (2u*SM9_BASE_WORD_LEN))
#define Q1x   (fp12_3 + (4u*SM9_BASE_WORD_LEN))
#define Q1y   (fp12_3 + (6u*SM9_BASE_WORD_LEN))
#define Q2y   (fp12_3 + (8u*SM9_BASE_WORD_LEN))
#define RBx   (fp12_3 + (10u*SM9_BASE_WORD_LEN))
#define RBy   (fp12_3 + (11u*SM9_BASE_WORD_LEN))
#else
uint32_t Px[SM9_BASE_WORD_LEN<<1];
uint32_t *Py  = &Px[SM9_BASE_WORD_LEN];
uint32_t *Qx  = fp12_3;
uint32_t *Qy  = &fp12_3[2u*SM9_BASE_WORD_LEN];
uint32_t *Q1x = &fp12_3[4u*SM9_BASE_WORD_LEN];
uint32_t *Q1y = &fp12_3[6u*SM9_BASE_WORD_LEN];
uint32_t *Q2y = &fp12_3[8u*SM9_BASE_WORD_LEN];
uint32_t *RBx = &fp12_3[10u*SM9_BASE_WORD_LEN];
uint32_t *RBy = &fp12_3[11u*SM9_BASE_WORD_LEN];
#endif
#endif

    uint32_t tmp[8];//rA or digest

    volatile uint32_t count = SM9_SEC_EXC_COUNTER;
    uint32_t ret = PKE_SUCCESS;
    uint8_t tag;

    uint32_t *curve_a, *curve_b, *curve_p, *curve_p_h;
    const eccp_curve_st *curve;
    eccp_sec_ctx_t ctx[1];
    uint16_t eccp_curve_crc16 = (uint16_t)0;
    uint16_t crc_g;
    uint16_t crc_g2;

    ret = sm9_exchangekey_s_check_input(role, IDA, IDA_bytes, IDB, IDB_bytes,
                                      deA, rA, RA, RB, k_bytes, k);
    if(PKE_SUCCESS == ret)
    {
        ret = sm9_exchangekey_s_internal_step_1(&curve, ctx, RA, RB, &count, &curve_a, &curve_b, &curve_p, &curve_p_h, eccp_curve_crc16, RBx, RBy);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_exchangekey_s_internal_step_2(curve_a, curve_b, &count, fp12g, &crc_g, Ppub_e,
            fp12g_bak, curve, Px, Py, rA, tmp, fp12);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_exchangekey_s_internal_step_3(&count, deA, Qx, Qy, curve, Q2y, Q1x, Q1y, curve_b, RBx, RBy, 
            fp12_2, fp12_3, tmp, curve_p, curve_p_h);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_exchangekey_s_internal_step_4(&count, fp12_2, fp12_3,
            fp12, IDA, IDB, RA, RB, role, IDA_bytes, IDB_bytes, digest_node, counter_buf,
            counter, &tag, S1, SA, sponsor_g1, k, k_bytes, tmp);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm9_exchangekey_s_internal_step_5(&count, eccp_curve_crc16, curve, fp12g, &crc_g2, &crc_g, fp12g_bak);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = SM9_SUCCESS_S;
    }
    else
    {
        ret = SM9_ERROR_S;
    }

#ifndef SUPPORT_STATIC_ANALYSIS
#undef Px
#undef Py
#undef Qx
#undef Qy
#undef Q1x
#undef Q1y
#undef Q2y
#undef RBx
#undef RBy
#endif

    (void)get_rand_fast((uint8_t *)counter_buf, sizeof(counter_buf));
    (void)get_rand_fast((uint8_t *)fp12g_bak, sizeof(fp12g_bak));
    (void)get_rand_fast((uint8_t *)fp12, sizeof(fp12));
    (void)get_rand_fast((uint8_t *)fp12_2, sizeof(fp12_2));
    (void)get_rand_fast((uint8_t *)fp12_3, sizeof(fp12_3));
    (void)get_rand_fast((uint8_t *)digest_node, sizeof(digest_node));
    (void)get_rand_fast((uint8_t *)tmp, sizeof(tmp));
    (void)get_rand_fast((uint8_t *)ctx, sizeof(eccp_sec_ctx_t));
    curve = NULL;
    crc_g = (uint16_t)0;
    crc_g2 = (uint16_t)0;
    sponsor_g1 = NULL;

    return ret;
}

#endif

