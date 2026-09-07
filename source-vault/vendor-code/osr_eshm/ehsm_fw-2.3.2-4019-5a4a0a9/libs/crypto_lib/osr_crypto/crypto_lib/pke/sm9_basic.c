
#include "../../crypto_hal/pke.h"


#ifdef SUPPORT_SM9


#include "./sm9_internal.h"
#include "../../crypto_include/pke/sm9.h"
#include "../../crypto_include/crypto_common/utility.h"
#ifdef PKE_SEC
#include "../../crypto_include/crypto_common/utility_sec.h"
#endif


//sm9 para (n-1), for private key checking
const uint32_t sm9p256v1_n_1[8]  = {0xD69ECF24u,0xE56EE19Cu,0x18EA8BEEu,0x49F2934Bu,0xF58EC744u,0xD603AB4Fu,0x02A3A6F1u,0xB6400000u};

//[2^128]P2, for [k]P2 of high speed
const uint32_t sm9_G2_Px0_Px1[16] = {
    0xAF82D65Bu,0xF9B7213Bu,0xD19C17ABu,0xEE265948u,0xD34EC120u,0xD2AAB97Fu,0x92130B08u,0x37227552u,  //Px0
    0xD8806141u,0x54806C11u,0x0F5E93C4u,0xF1DD2C19u,0xB441A01Fu,0x597B6027u,0x78640C98u,0x85AEF3D0u,  //Px1
};
const uint32_t sm9_G2_Py0_Py1[16] = {
    0xC999A7C7u,0x6215BBA5u,0xA71A0811u,0x47EFBA98u,0x3D278FF2u,0x5F317015u,0x19BE3DA6u,0xA7CF28D5u,  //Py0
    0x84EBEB96u,0x856DC76Bu,0xA347C8BDu,0x0736A96Fu,0x2CBEE6EDu,0x66BA0D26u,0x2E845C12u,0x17509B09u,  //Py1
};



/* function: sm9 GF(p) eccp point from U8 big-endian array to U32 little-endian array.
 * parameters:
 *     in ------------------------- input, an point in U8 big-endian, 64 bytes.
 *     Qx ------------------------- output, x coordinate of point in GF(p), 8 words
 *     Qy ------------------------- output, y coordinate of point in GF(p), 8 words
 * return: 
 * caution:
 *     1. input is in U8 big-endian array and output is in U32 little-endian.
 */
void sm9_fp_eccp_point_u8big_2_u32little(const uint8_t *in, uint32_t *Qx, uint32_t *Qy)
{
#ifdef SUPPORT_STATIC_ANALYSIS   //just for static analysis.
    if(NULL != in)
    {
#endif
        u8big_to_u32little_256bits(in, Qx);
        u8big_to_u32little_256bits(&(in[SM9_BASE_BYTE_LEN]), Qy);
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif
}


/* function: sm9 GF(p) eccp point from U32 little-endian array to U8 big-endian array.
 * parameters:
 *     Qx ------------------------- input, x coordinate of point in GF(p), 8 words
 *     Qy ------------------------- input, y coordinate of point in GF(p), 8 words
 *     out ------------------------ output, an point in U8 big-endian, 64 bytes.
 * return: 
 * caution:
 *     1. input is in U32 little-endian array and output is in U8 big-endian.
 */
void sm9_fp_eccp_point_u32little_2_u8big(const uint32_t *Qx, const uint32_t *Qy, uint8_t *out)
{
#ifdef SUPPORT_STATIC_ANALYSIS   //just for static analysis.
    if(NULL != out)
    {
#endif
        u32little_to_u8big_256bits(Qx, out);
        u32little_to_u8big_256bits(Qy, &(out[SM9_BASE_BYTE_LEN]));
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif
}


/* function: sm9 GF(p^2) eccp point from U8 big-endian array to U32 little-endian array.
 * parameters:
 *     in ------------------------- input, an point in U8 big-endian, 128 bytes.
 *     Qx ------------------------- output, x coordinate of point in GF(p^2), 2*8=16 words
 *     Qy ------------------------- output, y coordinate of point in GF(p^2), 2*8=16 words
 * return: 
 * caution:
 *     1. input is in U8 big-endian array and output is in U32 little-endian.
 */
void sm9_fp2_eccp_point_u8big_2_u32little(const uint8_t *in, uint32_t *Qx, uint32_t *Qy)
{
#ifdef SUPPORT_STATIC_ANALYSIS   //just for static analysis.
    if((NULL != in) && (NULL != Qx) && (NULL != Qy))
    {
#endif
        u8big_to_u32little_256bits(&(in[SM9_BASE_BYTE_LEN]), Qx);
        u8big_to_u32little_256bits(in, &(Qx[SM9_BASE_WORD_LEN]));
        u8big_to_u32little_256bits(&(in[3u*SM9_BASE_BYTE_LEN]), Qy);
        u8big_to_u32little_256bits(&(in[2u*SM9_BASE_BYTE_LEN]), &(Qy[SM9_BASE_WORD_LEN]));
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif
}


/* function: sm9 GF(p^2) eccp point from U32 little-endian array to U8 big-endian array.
 * parameters:
 *     Qx ------------------------- input, x coordinate of point in GF(p^2), 2*8=16 words
 *     Qy ------------------------- input, y coordinate of point in GF(p^2), 2*8=16 words
 *     out ------------------------ output, an point in U8 big-endian, 128 bytes.
 * return: 
 * caution:
 *     1. input is in U32 little-endian array and output is in U8 big-endian.
 */
void sm9_fp2_eccp_point_u32little_2_u8big(const uint32_t *Qx, const uint32_t *Qy, uint8_t *out)
{
#ifdef SUPPORT_STATIC_ANALYSIS   //just for static analysis.
    if((NULL != Qx) && (NULL != Qy) && (NULL != out))
    {
#endif
        u32little_to_u8big_256bits(&(Qx[SM9_BASE_WORD_LEN]), out);
        u32little_to_u8big_256bits(Qx, &(out[SM9_BASE_BYTE_LEN]));
        u32little_to_u8big_256bits(&(Qy[SM9_BASE_WORD_LEN]), &(out[2u*SM9_BASE_BYTE_LEN]));
        u32little_to_u8big_256bits(Qy, &(out[3u*SM9_BASE_BYTE_LEN]));
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif
}


/* function: sm9 GF(p^12) element from U8 big-endian array to U32 little-endian array.
 * parameters:
 *     in ------------------------- input, an element in GF(p^12), 2*32=384 bytes.
 *     out ------------------------ output, an element in GF(p^12), 12*8=96 words.
 * return: 
 * caution:
 *     1. input is in U8 big-endian array and output is in U32 little-endian.
 */
void sm9_fp12_u8big_2_u32little(const uint8_t *in, uint32_t *out)
{
    uint32_t i;

#ifdef SUPPORT_STATIC_ANALYSIS   //just for static analysis.
    if((NULL != in) && (NULL != out))
    {
#endif
        for(i = 12u; i > 0u; i--)
        {
            u8big_to_u32little_256bits(&in[(i-1u)*SM9_BASE_BYTE_LEN], (uint32_t *)(&out[(12u-i)*SM9_BASE_WORD_LEN]));
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif
}


/* function: sm9 GF(p^12) element from U32 little-endian array to U8 big-endian array.
 * parameters:
 *     a -------------------------- input/output, an element in GF(p^12), 12*8=96 words, or 12*32=384 bytes.
 * return: 
 * caution:
 *     1. input is in U32 little-endian array and output is in U8 big-endian.
 */
void sm9_fp12_u32little_2_u8big(uint32_t *a)
{
#if 1
    uint32_t tmp[8];
    uint32_t i;

#ifdef SUPPORT_STATIC_ANALYSIS   //just for static analysis.
    if(NULL != a)
    {
#endif
        for(i = 0u; i < 6u; i++)
        {
            u8big_to_u32little_256bits((uint8_t *)(&a[8u*i]), tmp);
            u8big_to_u32little_256bits((uint8_t *)(&a[8u*(12u-1u-i)]), &a[8u*i]);
            uint32_copy_8_words(&a[8u*(12u-1u-i)], tmp);
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif
#else
    reverse_byte_array((uint8_t *)in, out, 32u*12u);
#endif
}


//sm9 GF(p^12) element in pke ram from U32 little-endian array to U8 big-endian array.
void sm9_fp12_in_pke_ram_u32little_2_u8big(uint8_t *g)
{
    uint32_t i;

#ifdef SUPPORT_STATIC_ANALYSIS   //just for static analysis.
    const uint32_t *p = (uint32_t *)rPKE_A(4u,SM9_STEPS);

    if(NULL != g)
    {
        for(i = 0u; i < 3u; i++)
        {
            u32little_to_u8big_256bits(&p[(3u+(4u*(2u-i)))*(SM9_STEPS>>2)], &g[(4u*i)*SM9_BASE_BYTE_LEN]);
            u32little_to_u8big_256bits(&p[(2u+(4u*(2u-i)))*(SM9_STEPS>>2)], &g[((4u*i)+1u)*SM9_BASE_BYTE_LEN]);
            u32little_to_u8big_256bits(&p[(1u+(4u*(2u-i)))*(SM9_STEPS>>2)], &g[((4u*i)+2u)*SM9_BASE_BYTE_LEN]);
            u32little_to_u8big_256bits(&p[(0u+(4u*(2u-i)))*(SM9_STEPS>>2)], &g[((4u*i)+3u)*SM9_BASE_BYTE_LEN]);
        }
    }
    else
    {}
#else
    for(i = 0u; i < 3u; i++)
    {
        u32little_to_u8big_256bits((uint32_t *)rPKE_A(7u+(4u*(2u-i)),SM9_STEPS), &g[(4u*i)*SM9_BASE_BYTE_LEN]);
        u32little_to_u8big_256bits((uint32_t *)rPKE_A(6u+(4u*(2u-i)),SM9_STEPS), &g[((4u*i)+1u)*SM9_BASE_BYTE_LEN]);
        u32little_to_u8big_256bits((uint32_t *)rPKE_A(5u+(4u*(2u-i)),SM9_STEPS), &g[((4u*i)+2u)*SM9_BASE_BYTE_LEN]);
        u32little_to_u8big_256bits((uint32_t *)rPKE_A(4u+(4u*(2u-i)),SM9_STEPS), &g[((4u*i)+3u)*SM9_BASE_BYTE_LEN]);
    }
#endif
}


/* function: g = e(P1, P2)
 * parameters:
 *     P1 ------------------------- input, an ECC point in GF(p)
 *     P2 ------------------------- input, an ECC point in GF(p^2)
 *     g -------------------------- output, out = e(P1, P2), an element in GF(p^12)
 * return: SM9_SUCCESS(success), other(error)
 * caution:
 *     1. all input and output are U8 big-endian.
 */
uint32_t sm9_pairing_calc(const uint8_t P1[64], const uint8_t P2[128], uint8_t g[32*12])
{
    uint32_t ret;
    uint32_t tmp_buf[8u+8u+16u+16u];
    const uint32_t *Px, *Py, *Qx, *Qy;

    if(NULL == g)
    {
        ret = SM9_BUFFER_NULL;
    }
    else
    {
        ret = pke_load_modulus_and_pre_monts_256bits(sm9_curve->eccp_p, sm9_curve->eccp_p_h);
        if(PKE_SUCCESS == ret)
        {
            if(NULL != P1)
            {
                (void)sm9_fp_eccp_point_u8big_2_u32little(P1, tmp_buf, &tmp_buf[8u]);
                Px = tmp_buf;
                Py = &tmp_buf[8u];
            }
            else
            {
                Px = sm9_curve->eccp_Gx;
                Py = sm9_curve->eccp_Gy;
            }

            if(NULL != P2)
            {
                (void)sm9_fp2_eccp_point_u8big_2_u32little(P2, &tmp_buf[16u], &tmp_buf[32u]);
                Qx = &tmp_buf[16u];
                Qy = &tmp_buf[32u];
            }
            else
            {
                Qx = sm9_G2_Px0_Px1;
                Qy = sm9_G2_Py0_Py1;
            }

            ret = sm9_pairing(Px, Py, Qx, Qy, NULL);
            if(PKE_SUCCESS == ret)
            {
                (void)sm9_fp12_in_pke_ram_u32little_2_u8big(g);
                ret = SM9_SUCCESS;
            }
            else
            {}
        }
        else
        {}
    }

    return ret;
}


#if 0
/* function: out = g^r
 * parameters:
 *     g -------------------------- input, an element in GF(p^12)
 *     r -------------------------- input, exponent
 *     out ------------------------ output, out = g^r, also an element in GF(p^12)
 * return: SM9_SUCCESS(success), other(error)
 * caution:
 *     1. all input and output are U8 big-endian.
 */
uint32_t sm9_fp12_exp(uint8_t g[32*12], uint8_t r[32], uint8_t out[32*12])
{
    uint32_t tmp_g[12*8];
    uint32_t tmp_r[8];
    uint32_t ret;

    do {
        if((NULL == g) || (NULL == r) || (NULL == out))
        {
            ret= SM9_BUFFER_NULL;
            break;
        }
        else
        {}

#if 0
        ret = pke_set_modulus_and_pre_monts(sm9_curve->eccp_p, sm9_curve->eccp_p_h, sm9_curve->eccp_p_bitLen);
#else
        ret = pke_load_modulus_and_pre_monts_256bits(sm9_curve->eccp_p, sm9_curve->eccp_p_h);
#endif
        if(PKE_SUCCESS != ret)
        {
            break;
        }
        else
        {}

        (void)u8big_to_u32little_256bits((uint8_t *)r, (uint32_t *)tmp_r);
        (void)sm9_fp12_u8big_2_u32little(g, tmp_g);

        ret = sm9_fp12_exp_s(tmp_g, tmp_r, NULL);
        if(PKE_SUCCESS != ret)
        {
            break;
        }
        else
        {}

        (void)sm9_fp12_in_pke_ram_u32little_2_u8big(out);

        ret = SM9_SUCCESS;
    } while(0);

    return ret;
}
#endif


extern const uint32_t sm9_n_compl[8];   //just for static analysis.
extern const uint32_t sm9_n2[8];
extern const uint32_t sm9_n2_h[8];
extern const uint32_t sm9_n2_inv[8];
extern const uint32_t sm9_n2_compl[8];

//the following is for sm9_h1_h2_mod()
const uint32_t sm9_n_compl[8]  = {0x296130DBu,0x1A911E63u,0xE7157411u,0xB60D6CB4u,0x0A7138BBu,0x29FC54B0u,0xFD5C590Eu,0x49BFFFFFu}; //2^(256) mod n
const uint32_t sm9_n2[8]       = {0xFFFFF8A5u,0xFFFFFFFFu,0xFFFFFFFFu,0xFFFFFFFFu,0xFFFFFFFFu,0xFFFFFFFFu,0xFFFFFFFFu,0xFFFFFFFFu};
const uint32_t sm9_n2_h[8]     = {0x00361A59u,0u,0u,0u,0u,0u,0u,0u};
#if 0
const uint32_t sm9_n2_n1[8]    = {0x0E00D0D3u,0x5DABFB1Bu,0x4FCDF81Du,0xE4212C2Fu,0x36A738F6u,0x02B814AAu,0xE7EFAF84u,0xAE9061E2u};
#endif
const uint32_t sm9_n2_inv[8]   = {0xD81AE00Bu,0x73FFE2F2u,0x3CA00AC3u,0x6EE46995u,0x859A2700u,0xECEE7342u,0xE23BB01Cu,0xB707F075u}; //n^(-1) mod n2
const uint32_t sm9_n2_compl[8] = {0x0000075Bu,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u}; //2^(256) mod n2


/* function: result = high||low mod modulus
 * parameters:
 *     modulus -------------------- input, modulus
 *     modulus_h ------------------ input, R^2 mod modulus
 *     high_comp ------------------ input, 2^(256) mod modulus
 *     high ----------------------- input, high 256 bits of 512 bits
 *     low ------------------------ input, low 256 bits of 512 bits
 *     result --------------------- output, high||low mod modulus
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is called by sm9_h1_h2_mod()
 */
FLAG_STATIC uint32_t sm9_h1_h2_mod_primitive(const uint32_t modulus[8], const uint32_t modulus_h[8], 
        const uint32_t high_comp[8], const uint32_t high[8], const uint32_t low[8], uint32_t result[8])
{
    uint32_t ret;

#if 0
    ret = pke_set_modulus_and_pre_monts(modulus, modulus_h, SM9_BASE_BIT_LEN);
#else
    ret = pke_load_modulus_and_pre_monts_256bits(modulus, modulus_h);
#endif
    if(PKE_SUCCESS == ret)
    {
#if 0
        ret = pke_modmul_internal(high, high_comp, result, SM9_BASE_WORD_LEN);
#else
        ret = pke_mod_add_sub_mul_256bits_internal(high, high_comp, result, MICROCODE_MODMUL);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
#if 0
        ret = pke_modadd(modulus, low, result, result, SM9_BASE_WORD_LEN);
#else
        ret = pke_mod_add_sub_mul_256bits_internal(low, result, result, MICROCODE_MODADD);
#endif
    }
    else
    {}

    return ret;
}


/* function: h = (ha mod (n-1)) + 1
 * parameters:
 *     ha ------------------------- input, a big number of 320bits(40 bytes)
 *     h -------------------------- output, h = (ha mod (n-1)) + 1
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is called by H1() or H2(), i.e. sm9_h1_h2()
 *     2. ha is a internal big number of 320bits(40 bytes), h is a big number
 *        of 256bits.
 */
FLAG_STATIC uint32_t sm9_h1_h2_mod(const uint8_t ha[40], uint32_t h[8])
{
    uint32_t h2[SM9_BASE_WORD_LEN<<1];
    uint32_t *h1 = &h2[SM9_BASE_WORD_LEN];
    uint32_t h3[SM9_BASE_WORD_LEN+2u];
    uint32_t ret;

    //h1 = high 8 byte of ha
    h1[0] = ((uint32_t)ha[7]) | ((uint32_t)ha[6] << 8u) | ((uint32_t)ha[5] << 16u) | ((uint32_t)ha[4] << 24u);
    h1[1] = ((uint32_t)ha[3]) | ((uint32_t)ha[2] << 8u) | ((uint32_t)ha[1] << 16u) | ((uint32_t)ha[0] << 24u);
    h1[2] = 0u;
    h1[3] = 0u;
    h1[4] = 0u;
    h1[5] = 0u;
    h1[6] = 0u;
    h1[7] = 0u;

    //h2 = low 32 byte of ha
    u8big_to_u32little_256bits(&ha[8u], h2);

    /************ get h3 = ha mod n ************/
    ret = sm9_h1_h2_mod_primitive(sm9_curve->eccp_n, sm9_curve->eccp_n_h, 
            sm9_n_compl, h1, h2, h3);
    if(PKE_SUCCESS == ret)
    {
        /************ get h1||h2 = (ha - h3) = k1*n ************/
        h3[SM9_BASE_WORD_LEN]      = 0u;
        h3[SM9_BASE_WORD_LEN + 1u] = 0u;
        ret = pke_sub(h2, h3, h2, SM9_BASE_WORD_LEN+2u);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        /************ get h1 = h1||h2 mod n2 = k1*n mod n2, n2 > n. ************/
        ret = sm9_h1_h2_mod_primitive(sm9_n2, sm9_n2_h, sm9_n2_compl, h1, h2, h1);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        /************ get h2 = k1 mod n2, n2 > n. ************/
#if 0
        ret = pke_modmul_internal(h1, sm9_n2_inv, h2, SM9_BASE_WORD_LEN);
#else
        ret = pke_mod_add_sub_mul_256bits_internal(h1, sm9_n2_inv, h2, MICROCODE_MODMUL);
#endif
    }
    else
    {}

#if 0
    if(PKE_SUCCESS != ret)
    {
        /************ get h3 = h3 mod (n-1) ************/
        uint32_clear_8_words(h);
#if 0
        ret = pke_modadd(sm9p256v1_n_1, h3, h, h3, SM9_BASE_WORD_LEN);
#else
        ret = pke_mod_add_sub_mul_256bits_internal(h3, h, h3, MICROCODE_INTADD);
#endif
    }
    else
    {}
#endif

    if(PKE_SUCCESS == ret)
    {
        /************ get h = k1+h3 mod (n-1) = ha mod (n-1) ************/
#if 0
        ret = pke_modadd(sm9p256v1_n_1, h3, h2, h, SM9_BASE_WORD_LEN);
#else
        ret = pke_modadd_modsub_256bits(sm9p256v1_n_1, h3, h2, h, MICROCODE_MODADD);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //h += 1
        ret = uint32_big_num_little_endian_add_little(h, SM9_BASE_WORD_LEN, 1u, (uint8_t)1);
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

    uint32_clear(h2, sizeof(h2)>>2);
    uint32_clear(h3, sizeof(h3)>>2);

    return ret;
}


/* function: h = H1(Z,n) or H2(Z,n), h is a big number of 256bits.
 * parameters:
 *     tag ------------------------ input, tag of 1 byte, 0x01 for H1(), 0x02 for H2()
 *     z1 ------------------------- input, z1
 *     z1_bytes ------------------- input, byte length of z1
 *     z2 ------------------------- input, z2
 *     z2_bytes ------------------- input, byte length of z2
 *     h -------------------------- output, a big number less than n.
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is H1(Z,n) or H2(Z,n), Z=z1||z2, the difference of the two is 
 *        that the two tag values are different, actually, 0x01 for H1(), 0x02 for H2().
 *        n is order of the elliptic curve.
 *     2. h = (ha mod (n-1))+1, ha is a internal big number of 320bits(40 bytes), h is 
 *        a big number of 256bits.
 */
uint32_t sm9_h1_h2(uint8_t tag, const uint8_t *z1, uint32_t z1ByteLen, const uint8_t *z2, 
        uint32_t z2ByteLen, uint32_t *h)
{
    uint32_t ret;
    uint8_t cnt[4] = {0x00, 0x00, 0x00, 0x02};
    uint8_t ha[40];
    hash_node_st digest_node[4];
    uint8_t tag_bak = tag;  //for static analysis

    digest_node[0].msg_addr  = (uint8_t *)(&tag_bak);
    digest_node[0].msg_bytes = 1u;
    digest_node[1].msg_addr  = z1;
    digest_node[1].msg_bytes = z1ByteLen;
    digest_node[2].msg_addr  = z2;
    digest_node[2].msg_bytes = z2ByteLen;
    digest_node[3].msg_addr  = cnt;
    digest_node[3].msg_bytes = 4u;

    ret = hash_node_steps(HASH_SM3, digest_node, 4u, ha);
    if(HASH_SUCCESS == ret)
    {
        memcpy_(&ha[32u], ha, 8u);

        cnt[3] = (uint8_t)0x01;
        ret = hash_node_steps(HASH_SM3, digest_node, 4u, ha);
    }
    else
    {}

    if(HASH_SUCCESS == ret)
    {
        ret = sm9_h1_h2_mod(ha, h);
    }
    else
    {}

    return ret;
}

#endif

