
#include "../../crypto_hal/pke.h"



#ifdef PKE_SEC

#include "eccp_sec_common.h"
#include "../../crypto_include/crypto_common/utility_sec.h"




#if (defined(PKE_LP) || defined(PKE_SECURE))
//result of crc16(p||p_h||p_n0||a||b||Gx||Gy||n||n_h||n_n0)
#define SM2_CURVE_CRC16                    (0x2A42)
#define BRAINPOOLP160R1_CURVE_CRC16        (0x1A8C)
#define SECP192R1_CURVE_CRC16              (0xDFC4)
#define SECP224R1_CURVE_CRC16              (0x781C)
#define SECP256R1_CURVE_CRC16              (0xA389)
#define SECP384R1_CURVE_CRC16              (0xA477)
#define BRAINPOOLP512R1_CURVE_CRC16        (0x6A20)
#define SECP521R1_CURVE_CRC16              (0xDA47)
#else
//result of crc16(p||p_h||a||b||Gx||Gy||n||n_h||half_Gx||half_Gy)
#define SM2_CURVE_CRC16                    (0xED1F)
#define SM9_CURVE_CRC16                    (0xB160)
#define SECP160K1_CURVE_CRC16              (0x2E0F)
#define SECP192K1_CURVE_CRC16              (0xA280)
#define SECP224K1_CURVE_CRC16              (0xE6F4)
#define SECP256K1_CURVE_CRC16              (0x0B26)
#define BRAINPOOLP160R1_CURVE_CRC16        (0x5F15)
#define SECP160R1_CURVE_CRC16              (0xBCCA)
#define SECP160R2_CURVE_CRC16              (0xF36C)
#define SECP192R1_CURVE_CRC16              (0xC82F)
#define SECP224R1_CURVE_CRC16              (0x4B0B)
#define SECP256R1_CURVE_CRC16              (0xC360)
#define SECP384R1_CURVE_CRC16              (0x2BED)
#define BRAINPOOLP512R1_CURVE_CRC16        (0xAC2E)
#define SECP521R1_CURVE_CRC16              (0x4E3C)
#define ANDERS_1024_1_CURVE_CRC16          (0x54E3)
#endif



/* function: check whether eccp curve defined internal(p bit length is 160)
 * parameters:
 *     curve ---------------------- input, eccp_curve struct pointer
 * return: 1(yes), 0(no)
 * caution:
 *     1. curve should not be null
 */
FLAG_STATIC uint32_t is_eccp_curve_defined_internal_curve_width_160(const eccp_curve_st *curve)
{
    uint32_t ret = 0u;
    uint32_t pWordLen = (160u + 31u)>>5;

#ifdef SUPPORT_BRAINPOOLP160R1
    if(0 == uint32_BigNumCmp(curve->eccp_p, pWordLen, brainpoolp160r1->eccp_p, pWordLen))
    {
        ret = 1u;
    }
    else
    {}
#endif

#ifdef SUPPORT_SECP160K1
    if(0 == uint32_BigNumCmp(curve->eccp_Gy, pWordLen, secp160k1->eccp_Gy, pWordLen)) //since secp160k1 and secp160r2 have the same p
    {
        ret = 1u;
    }
    else
    {}
#endif

#ifdef SUPPORT_SECP160R1
    if(0 == uint32_BigNumCmp(curve->eccp_p, pWordLen, secp160r1->eccp_p, pWordLen))
    {
        ret = 1u;
    }
    else
    {}
#endif

#ifdef SUPPORT_SECP160R2
    if(0 == uint32_BigNumCmp(curve->eccp_Gy, pWordLen, secp160r2->eccp_Gy, pWordLen))
    {
        ret = 1u;
    }
    else
    {}
#endif

    return ret;
}


/* function: check whether eccp curve defined internal(p bit length is 192)
 * parameters:
 *     curve ---------------------- input, eccp_curve struct pointer
 * return: 1(yes), 0(no)
 * caution:
 *     1. curve should not be null
 */
FLAG_STATIC uint32_t is_eccp_curve_defined_internal_curve_width_192(const eccp_curve_st *curve)
{
    uint32_t ret = 0u;
    uint32_t pWordLen = (192u + 31u)>>5;

#ifdef SUPPORT_SECP192R1
    if(0 == uint32_BigNumCmp(curve->eccp_p, pWordLen, secp192r1->eccp_p, pWordLen))
    {
        ret = 1u;
    }
    else
    {}
#endif

#ifdef SUPPORT_SECP192K1
    if(0 == uint32_BigNumCmp(curve->eccp_p, pWordLen, secp192k1->eccp_p, pWordLen))
    {
        ret = 1u;
    }
    else
    {}
#endif

    return ret;
}


/* function: check whether eccp curve defined internal(p bit length is 224)
 * parameters:
 *     curve ---------------------- input, eccp_curve struct pointer
 * return: 1(yes), 0(no)
 * caution:
 *     1. curve should not be null
 */
FLAG_STATIC uint32_t is_eccp_curve_defined_internal_curve_width_224(const eccp_curve_st *curve)
{
    uint32_t ret = 0u;
    uint32_t pWordLen = (224u + 31u)>>5;

#ifdef SUPPORT_SECP224R1
    if(0 == uint32_BigNumCmp(curve->eccp_p, pWordLen, secp224r1->eccp_p, pWordLen))
    {
        ret = 1u;
    }
    else
    {}
#endif

#ifdef SUPPORT_SECP224K1
    if(0 == uint32_BigNumCmp(curve->eccp_p, pWordLen, secp224k1->eccp_p, pWordLen))
    {
        ret = 1u;
    }
    else
    {}
#endif

    return ret;
}


/* function: check whether eccp curve defined internal(p bit length is 256)
 * parameters:
 *     curve ---------------------- input, eccp_curve struct pointer
 * return: 1(yes), 0(no)
 * caution:
 *     1. curve should not be null
 */
FLAG_STATIC uint32_t is_eccp_curve_defined_internal_curve_width_256(const eccp_curve_st *curve)
{
    uint32_t ret = 0u;
    uint32_t pWordLen = (256u + 31u)>>5;

#ifdef SUPPORT_SECP256R1
    if(0 == uint32_BigNumCmp(curve->eccp_p, pWordLen, secp256r1->eccp_p, pWordLen))
    {
        ret= 1u;
    }
    else
    {}
#endif

#ifdef SUPPORT_SECP256K1
    if(0 == uint32_BigNumCmp(curve->eccp_p, pWordLen, secp256k1->eccp_p, pWordLen))
    {
        ret = 1u;
    }
    else
    {}
#endif

#ifdef SUPPORT_SM2
    if(0 == uint32_BigNumCmp(curve->eccp_p, pWordLen, sm2_curve->eccp_p, pWordLen))
    {
        ret = 1u;
    }
    else
    {}
#endif

#ifdef SUPPORT_SM9
    if(0 == uint32_BigNumCmp(curve->eccp_p, pWordLen, sm9_curve->eccp_p, pWordLen))
    {
        ret = 1u;
    }
    else
    {}
#endif

    return ret;
}


#if (ECCP_MAX_BIT_LEN >= 384U)
/* function: check whether eccp curve defined internal(p bit length is 384)
 * parameters:
 *     curve ---------------------- input, eccp_curve struct pointer
 * return: 1(yes), 0(no)
 * caution:
 *     1. curve should not be null
 */
FLAG_STATIC uint32_t is_eccp_curve_defined_internal_curve_width_384(const eccp_curve_st *curve)
{
    uint32_t ret = 0u;
    uint32_t pWordLen = (384u + 31u)>>5;

#ifdef SUPPORT_SECP384R1
    if(0 == uint32_BigNumCmp(curve->eccp_p, pWordLen, secp384r1->eccp_p, pWordLen))
    {
        ret = 1u;
    }
    else
    {}
#endif

    return ret;
}
#endif


#if (ECCP_MAX_BIT_LEN >= 512U)
/* function: check whether eccp curve defined internal(p bit length is 512)
 * parameters:
 *     curve ---------------------- input, eccp_curve struct pointer
 * return: 1(yes), 0(no)
 * caution:
 *     1. curve should not be null
 */
FLAG_STATIC uint32_t is_eccp_curve_defined_internal_curve_width_512(const eccp_curve_st *curve)
{
    uint32_t ret = 0u;
    uint32_t pWordLen = (512u + 31u)>>5;

#ifdef SUPPORT_BRAINPOOLP512R1
    if(0 == uint32_BigNumCmp(curve->eccp_p, pWordLen, brainpoolp512r1->eccp_p, pWordLen))
    {
        ret = 1u;
    }
    else
    {}
#endif

    return ret;
}
#endif


#if (ECCP_MAX_BIT_LEN >= 521U)
/* function: check whether eccp curve defined internal(p bit length is 521)
 * parameters:
 *     curve ---------------------- input, eccp_curve struct pointer
 * return: 1(yes), 0(no)
 * caution:
 *     1. curve should not be null
 */
FLAG_STATIC uint32_t is_eccp_curve_defined_internal_curve_width_521(const eccp_curve_st *curve)
{
    uint32_t ret = 0u;
    uint32_t pWordLen = (521u + 31u)>>5;

#ifdef SUPPORT_SECP521R1
    if(0 == uint32_BigNumCmp(curve->eccp_p, pWordLen, secp521r1->eccp_p, pWordLen))
    {
        ret = 1u;
    }
    else
    {}
#endif

    return ret;
}
#endif


#if (ECCP_MAX_BIT_LEN >= 1024U)
/* function: check whether eccp curve defined internal(p bit length is 1024)
 * parameters:
 *     curve ---------------------- input, eccp_curve struct pointer
 * return: 1(yes), 0(no)
 * caution:
 *     1. curve should not be null
 */
FLAG_STATIC uint32_t is_eccp_curve_defined_internal_curve_width_1024(const eccp_curve_st *curve)
{
    uint32_t ret = 0u;
    uint32_t pWordLen = (1024u + 31u)>>5;

#ifdef SUPPORT_ANDERS_1024_1
    if(0 == uint32_BigNumCmp(curve->eccp_p, pWordLen, anders_1024_1->eccp_p, pWordLen))
    {
        ret = 1u;
    }
    else
    {}
#endif

    return ret;
}
#endif


/* function: check whether eccp curve defined internal(p bit length is 160,192,224,256)
 * parameters:
 *     curve ---------------------- input, eccp_curve struct pointer
 * return: 1(yes), 0(no)
 * caution:
 *     1. curve should not be null
 */
FLAG_STATIC uint32_t is_eccp_curve_defined_internal_curve_width_between_160_and_256(const eccp_curve_st *curve)
{
    uint32_t ret = 0u;

    if(160U == curve->eccp_p_bitLen)
    {
        ret = is_eccp_curve_defined_internal_curve_width_160(curve);
    }
    else
    {}

    if(192U == curve->eccp_p_bitLen)
    {
        ret = is_eccp_curve_defined_internal_curve_width_192(curve);
    }
    else
    {}

    if(224U == curve->eccp_p_bitLen)
    {
        ret = is_eccp_curve_defined_internal_curve_width_224(curve);
    }
    else
    {}

    if(256U == curve->eccp_p_bitLen)
    {
        ret = is_eccp_curve_defined_internal_curve_width_256(curve);
    }
    else
    {}

    return ret;
}


#if (ECCP_MAX_BIT_LEN > 256U)
/* function: check whether eccp curve defined internal(p bit length is 320,384,512)
 * parameters:
 *     curve ---------------------- input, eccp_curve struct pointer
 * return: 1(yes), 0(no)
 * caution:
 *     1. curve should not be null
 */
FLAG_STATIC uint32_t is_eccp_curve_defined_internal_curve_width_between_257_and_512(const eccp_curve_st *curve)
{
    uint32_t ret = 0u;

#if (ECCP_MAX_BIT_LEN >= 384U)
    if(384U == curve->eccp_p_bitLen)
    {
        ret = is_eccp_curve_defined_internal_curve_width_384(curve);
    }
    else
    {}
#endif

#if (ECCP_MAX_BIT_LEN >= 512U)
    if(512U == curve->eccp_p_bitLen)
    {
        ret = is_eccp_curve_defined_internal_curve_width_512(curve);
    }
    else
    {}
#endif

    return ret;
}
#endif


#if (ECCP_MAX_BIT_LEN > 512U)
/* function: check whether eccp curve defined internal(p bit length is greater than 512)
 * parameters:
 *     curve ---------------------- input, eccp_curve struct pointer
 * return: 1(yes), 0(no)
 * caution:
 *     1. curve should not be null
 */
FLAG_STATIC uint32_t is_eccp_curve_defined_internal_curve_width_greater_than_512(const eccp_curve_st *curve)
{
    uint32_t ret = 0u;

#if (ECCP_MAX_BIT_LEN >= 521U)
    if(521U == curve->eccp_p_bitLen)
    {
        ret = is_eccp_curve_defined_internal_curve_width_521(curve);
    }
    else
    {}
#endif

#if (ECCP_MAX_BIT_LEN >= 1024U)
    if(1024U == curve->eccp_p_bitLen)
    {
        ret = is_eccp_curve_defined_internal_curve_width_1024(curve);
    }
    else
    {}
#endif

    return ret;
}
#endif


/* function: check whether eccp curve defined internal
 * parameters:
 *     curve ---------------------- input, eccp_curve struct pointer
 * return: 1(yes), 0(no)
 * caution:
 *     1. curve should not be null
 */
uint32_t is_eccp_curve_defined_internal(const eccp_curve_st *curve)
{
    uint32_t ret = 0u;

    if((curve->eccp_p_bitLen >= 160U) && (curve->eccp_p_bitLen <= 256U))
    {
        ret = is_eccp_curve_defined_internal_curve_width_between_160_and_256(curve);
    }
    else
    {}

#if (ECCP_MAX_BIT_LEN > 256U)
    if((curve->eccp_p_bitLen > 256U) && (curve->eccp_p_bitLen <= 512U))
    {
        ret = is_eccp_curve_defined_internal_curve_width_between_257_and_512(curve);
    }
    else
    {}
#endif

#if (ECCP_MAX_BIT_LEN > 512U)
    if((curve->eccp_p_bitLen > 512U) && (curve->eccp_p_bitLen <= 1024U))
    {
        ret = is_eccp_curve_defined_internal_curve_width_greater_than_512(curve);
    }
    else
    {}
#endif

    return ret;
}


/* function: calculate eccp curve crc16
 * parameters:
 *     curve ---------------------- input, eccp_curve struct pointer
 * return: crc value
 * caution:
 *     1. curve should not be null
 */
uint16_t calc_eccp_curve_crc16(const eccp_curve_st *curve)
{
    uint32_t pByteLen = ((curve->eccp_p_bitLen + 31U) & (~0x1FU))>>3;  //((curve->eccp_p_bitLen + 31)/32)*4
    uint32_t nByteLen = ((curve->eccp_n_bitLen + 31U) & (~0x1FU))>>3;  //((curve->eccp_n_bitLen + 31)/32)*4
    const uint8_t *addr;
    uint16_t crc = (uint16_t)0xFFFF;

    addr = (const uint8_t *)(curve->eccp_p);
    crc = crc16_calc(addr, pByteLen, crc);

    addr = (const uint8_t *)(curve->eccp_p_h);
    if (NULL != addr)
    {
        crc = crc16_calc(addr, pByteLen, crc);
    }
    else
    {}

#if (defined(PKE_LP) || defined(PKE_SECURE))
    addr = (const uint8_t *)(curve->eccp_p_n0);
    if (NULL != addr)
    {
        crc = crc16_calc(addr, 4u, crc);
    }
    else
    {}
#endif

    addr = (const uint8_t *)(curve->eccp_a);
    crc = crc16_calc(addr, pByteLen, crc);

    addr = (const uint8_t *)(curve->eccp_b);
    crc = crc16_calc(addr, pByteLen, crc);

    addr = (const uint8_t *)(curve->eccp_Gx);
    crc = crc16_calc(addr, pByteLen, crc);

    addr = (const uint8_t *)(curve->eccp_Gy);
    crc = crc16_calc(addr, pByteLen, crc);

    addr = (const uint8_t *)(curve->eccp_n);
    crc = crc16_calc(addr, nByteLen, crc);

    addr = (const uint8_t *)(curve->eccp_n_h);
    if (NULL != addr)
    {
        crc = crc16_calc(addr, nByteLen, crc);
    }
    else
    {}

#if (defined(PKE_LP) || defined(PKE_SECURE))
    addr = (const uint8_t *)(curve->eccp_n_n0);
    if (NULL != addr)
    {
        crc = crc16_calc(addr, 4u, crc);
    }
    else
    {}
#else
    addr = (const uint8_t *)(curve->eccp_half_Gx);
    crc = crc16_calc(addr, pByteLen, crc);

    addr = (const uint8_t *)(curve->eccp_half_Gy);
    crc = crc16_calc(addr, pByteLen, crc);
#endif

    return crc;
}


/* function: check crc16 value of ecc curve(p is 160 bits)
 * parameters:
 *     curve ---------------------- input, eccp_curve struct pointer
 * return: crc16 of the curve
 * caution:
 *     1. curve must be defined internally
 */
FLAG_STATIC uint16_t ecc_crc16_check_curve_width_160(const eccp_curve_st *curve)
{
    uint32_t pWordLen = (160u + 31u)>>5;
    uint16_t crc16 = (uint16_t)0u;

#ifdef SUPPORT_BRAINPOOLP160R1
    if(0 == uint32_BigNumCmp(curve->eccp_p, pWordLen, brainpoolp160r1->eccp_p, pWordLen))
    {
        crc16 = BRAINPOOLP160R1_CURVE_CRC16;
    }
    else
    {}
#endif

#ifdef SUPPORT_SECP160K1
    if(0 == uint32_BigNumCmp(curve->eccp_Gy, pWordLen, secp160k1->eccp_Gy, pWordLen))
    {
        crc16 = SECP160K1_CURVE_CRC16;
    }
    else
    {}
#endif

#ifdef SUPPORT_SECP160R1
    if(0 == uint32_BigNumCmp(curve->eccp_p, pWordLen, secp160r1->eccp_p, pWordLen))
    {
        crc16 = SECP160R1_CURVE_CRC16;
    }
    else
    {}
#endif

#ifdef SUPPORT_SECP160R2
    if(0 == uint32_BigNumCmp(curve->eccp_Gy, pWordLen, secp160r2->eccp_Gy, pWordLen))
    {
        crc16 = SECP160R2_CURVE_CRC16;
    }
    else
    {}
#endif

    return crc16;
}


/* function: check crc16 value of ecc curve(p is 192 bits)
 * parameters:
 *     curve ---------------------- input, eccp_curve struct pointer
 * return: crc16 of the curve
 * caution:
 *     1. curve must be defined internally
 */
FLAG_STATIC uint16_t ecc_crc16_check_curve_width_192(const eccp_curve_st *curve)
{
    uint32_t pWordLen = (192u + 31u)>>5;
    uint16_t crc16 = (uint16_t)0u;

#ifdef SUPPORT_SECP192R1
    if(0 == uint32_BigNumCmp(curve->eccp_p, pWordLen, secp192r1->eccp_p, pWordLen))
    {
        crc16 = SECP192R1_CURVE_CRC16;
    }
    else
    {}
#endif

#ifdef SUPPORT_SECP192K1
    if(0 == uint32_BigNumCmp(curve->eccp_p, pWordLen, secp192k1->eccp_p, pWordLen))
    {
        crc16 = SECP192K1_CURVE_CRC16;
    }
    else
    {}
#endif

    return crc16;
}


/* function: check crc16 value of ecc curve(p is 224 bits)
 * parameters:
 *     curve ---------------------- input, eccp_curve struct pointer
 * return: crc16 of the curve
 * caution:
 *     1. curve must be defined internally
 */
FLAG_STATIC uint16_t ecc_crc16_check_curve_width_224(const eccp_curve_st *curve)
{
    uint32_t pWordLen = (224u + 31u)>>5;
    uint16_t crc16 = (uint16_t)0u;

#ifdef SUPPORT_SECP224R1
    if(0 == uint32_BigNumCmp(curve->eccp_p, pWordLen, secp224r1->eccp_p, pWordLen))
    {
        crc16 = SECP224R1_CURVE_CRC16;
    }
    else
    {}
#endif

#ifdef SUPPORT_SECP224K1
    if(0 == uint32_BigNumCmp(curve->eccp_p, pWordLen, secp224k1->eccp_p, pWordLen))
    {
        crc16 = SECP224K1_CURVE_CRC16;
    }
    else
    {}
#endif

    return crc16;
}


/* function: check crc16 value of ecc curve(p is 256 bits)
 * parameters:
 *     curve ---------------------- input, eccp_curve struct pointer
 * return: crc16 of the curve
 * caution:
 *     1. curve must be defined internally
 */
FLAG_STATIC uint16_t ecc_crc16_check_curve_width_256(const eccp_curve_st *curve)
{
    uint32_t pWordLen = (256u + 31u)>>5;
    uint16_t crc16 = (uint16_t)0u;

#ifdef SUPPORT_SECP256R1
    if(0 == uint32_BigNumCmp(curve->eccp_p, pWordLen, secp256r1->eccp_p, pWordLen))
    {
        crc16 = SECP256R1_CURVE_CRC16;
    }
    else
    {}
#endif

#ifdef SUPPORT_SECP256K1
    if(0 == uint32_BigNumCmp(curve->eccp_p, pWordLen, secp256k1->eccp_p, pWordLen))
    {
        crc16 = SECP256K1_CURVE_CRC16;
    }
    else
    {}
#endif

#ifdef SUPPORT_SM2
    if(0 == uint32_BigNumCmp(curve->eccp_p, pWordLen, sm2_curve->eccp_p, pWordLen))
    {
        crc16 = SM2_CURVE_CRC16;
    }
    else
    {}
#endif

#ifdef SUPPORT_SM9
    if(0 == uint32_BigNumCmp(curve->eccp_p, pWordLen, sm9_curve->eccp_p, pWordLen))
    {
        crc16 = SM9_CURVE_CRC16;
    }
    else
    {}
#endif

    return crc16;
}


#if (ECCP_MAX_BIT_LEN >= 384U)
/* function: check crc16 value of ecc curve(p is 384 bits)
 * parameters:
 *     curve ---------------------- input, eccp_curve struct pointer
 * return: crc16 of the curve
 * caution:
 *     1. curve must be defined internally
 */
FLAG_STATIC uint16_t ecc_crc16_check_curve_width_384(const eccp_curve_st *curve)
{
    uint32_t pWordLen = (384u + 31u)>>5;
    uint16_t crc16 = (uint16_t)0u;

    if(384U == curve->eccp_p_bitLen)
    {
#ifdef SUPPORT_SECP384R1
        if(0 == uint32_BigNumCmp(curve->eccp_p, pWordLen, secp384r1->eccp_p, pWordLen))
        {
            crc16 = SECP384R1_CURVE_CRC16;
        }
        else
        {}
#endif
    }
    else
    {}

    return crc16;
}
#endif


#if (ECCP_MAX_BIT_LEN >= 512U)
/* function: check crc16 value of ecc curve(p is 512 bits)
 * parameters:
 *     curve ---------------------- input, eccp_curve struct pointer
 * return: crc16 of the curve
 * caution:
 *     1. curve must be defined internally
 */
FLAG_STATIC uint16_t ecc_crc16_check_curve_width_512(const eccp_curve_st *curve)
{
    uint32_t pWordLen = (512u + 31u)>>5;
    uint16_t crc16 = (uint16_t)0u;

    if(512U == curve->eccp_p_bitLen)
    {
#ifdef SUPPORT_BRAINPOOLP512R1
        if(0 == uint32_BigNumCmp(curve->eccp_p, pWordLen, brainpoolp512r1->eccp_p, pWordLen))
        {
            crc16 = BRAINPOOLP512R1_CURVE_CRC16;
        }
        else
        {}
#endif
    }
    else
    {}

    return crc16;
}
#endif


#if (ECCP_MAX_BIT_LEN >= 521U)
/* function: check crc16 value of ecc curve(p is 521 bits)
 * parameters:
 *     curve ---------------------- input, eccp_curve struct pointer
 * return: crc16 of the curve
 * caution:
 *     1. curve must be defined internally
 */
FLAG_STATIC uint16_t ecc_crc16_check_curve_width_521(const eccp_curve_st *curve)
{
    uint32_t pWordLen = (521u + 31u)>>5;
    uint16_t crc16 = (uint16_t)0u;

    if(521U == curve->eccp_p_bitLen)
    {
#ifdef SUPPORT_SECP521R1
        if(0 == uint32_BigNumCmp(curve->eccp_p, pWordLen, secp521r1->eccp_p, pWordLen))
        {
            crc16 = SECP521R1_CURVE_CRC16;
        }
        else
        {}
#endif
    }
    else
    {}

    return crc16;
}
#endif


#if (ECCP_MAX_BIT_LEN >= 1024U)
/* function: check crc16 value of ecc curve(p is 1024 bits)
 * parameters:
 *     curve ---------------------- input, eccp_curve struct pointer
 * return: crc16 of the curve
 * caution:
 *     1. curve must be defined internally
 */
FLAG_STATIC uint16_t ecc_crc16_check_curve_width_1024(const eccp_curve_st *curve)
{
    uint32_t pWordLen = (1024u + 31u)>>5;
    uint16_t crc16 = (uint16_t)0u;

    if(1024U == curve->eccp_p_bitLen)
    {
#ifdef SUPPORT_ANDERS_1024_1
        if(0 == uint32_BigNumCmp(curve->eccp_p, pWordLen, anders_1024_1->eccp_p, pWordLen))
        {
            crc16 = ANDERS_1024_1_CURVE_CRC16;
        }
        else
        {}
#endif
    }
    else
    {}

    return crc16;
}
#endif


/* function: check crc16 value of ecc curve(p bit length is 160,192,224,256)
 * parameters:
 *     curve ---------------------- input, eccp_curve struct pointer
 * return: crc16 of the curve
 * caution:
 *     1. curve must be defined internally
 */
FLAG_STATIC uint16_t ecc_crc16_check_curve_width_between_160_and_256(const eccp_curve_st *curve)
{
    uint16_t crc16 = (uint16_t)0u;

    if(160U == curve->eccp_p_bitLen)
    {
        crc16 = ecc_crc16_check_curve_width_160(curve);
    }
    else
    {}

    if(192U == curve->eccp_p_bitLen)
    {
        crc16 = ecc_crc16_check_curve_width_192(curve);
    }
    else
    {}

    if(224U == curve->eccp_p_bitLen)
    {
        crc16 = ecc_crc16_check_curve_width_224(curve);
    }
    else
    {}

    if(256U == curve->eccp_p_bitLen)
    {
        crc16 = ecc_crc16_check_curve_width_256(curve);
    }
    else
    {}

    return crc16;
}


#if (ECCP_MAX_BIT_LEN > 256U)
/* function: check crc16 value of ecc curve(p bit length is 320,384,512)
 * parameters:
 *     curve ---------------------- input, eccp_curve struct pointer
 * return: crc16 of the curve
 * caution:
 *     1. curve must be defined internally
 */
FLAG_STATIC uint16_t ecc_crc16_check_curve_width_between_257_and_512(const eccp_curve_st *curve)
{
    uint16_t crc16 = (uint16_t)0u;

#if (ECCP_MAX_BIT_LEN >= 384U)
    if(384U == curve->eccp_p_bitLen)
    {
        crc16 = ecc_crc16_check_curve_width_384(curve);
    }
    else
    {}
#endif

#if (ECCP_MAX_BIT_LEN >= 512U)
    if(512U == curve->eccp_p_bitLen)
    {
        crc16 = ecc_crc16_check_curve_width_512(curve);
    }
    else
    {}
#endif

    return crc16;
}
#endif


#if (ECCP_MAX_BIT_LEN > 512U)
/* function: check crc16 value of ecc curve(p bit length is greater than 512)
 * parameters:
 *     curve ---------------------- input, eccp_curve struct pointer
 * return: crc16 of the curve
 * caution:
 *     1. curve must be defined internally
 */
FLAG_STATIC uint16_t ecc_crc16_check_curve_width_greater_than_512(const eccp_curve_st *curve)
{
    uint16_t crc16 = (uint16_t)0u;

#if (ECCP_MAX_BIT_LEN >= 521U)
    if(521U == curve->eccp_p_bitLen)
    {
        crc16 = ecc_crc16_check_curve_width_521(curve);
    }
    else
    {}
#endif

#if (ECCP_MAX_BIT_LEN >= 1024U)
    if(1024U == curve->eccp_p_bitLen)
    {
        crc16 = ecc_crc16_check_curve_width_1024(curve);
    }
    else
    {}
#endif

    return crc16;
}
#endif

/* function: check crc16 value of ecc curve
 * parameters:
 *     curve ---------------------- input, eccp_curve struct pointer
 *     curve_crc16 ---------------- input, expected crc16
 * return: 0(success), 1(error)
 * caution:
 *     1. curve should not be null
 */
uint32_t ecc_crc16_check(const eccp_curve_st *curve, uint16_t curve_crc16)
{
    uint16_t crc16 = curve_crc16;

    if(0U != is_eccp_curve_defined_internal(curve))
    {
        if((curve->eccp_p_bitLen >= 160U) && (curve->eccp_p_bitLen <= 256U))
        {
            crc16 = ecc_crc16_check_curve_width_between_160_and_256(curve);
        }
        else
        {}

#if (ECCP_MAX_BIT_LEN > 256U)
        if((curve->eccp_p_bitLen > 256U) && (curve->eccp_p_bitLen <= 512U))
        {
            crc16 = ecc_crc16_check_curve_width_between_257_and_512(curve);
        }
        else
        {}
#endif

#if (ECCP_MAX_BIT_LEN > 512U)
        if((curve->eccp_p_bitLen > 512U) && (curve->eccp_p_bitLen <= 1024U))
        {
            crc16 = ecc_crc16_check_curve_width_greater_than_512(curve);
        }
        else
        {}
#endif
    }
    else
    {}

    return (crc16 == calc_eccp_curve_crc16(curve))?0U:1U;
}


/* function: init internal sec eccp curve struct eccp_curve_st
 * parameters:
 *     ctx ------------------------ input, eccp_sec_ctx_t struct pointer
 *     curve ---------------------- input, eccp_curve_st struct pointer
 * return: eccp_curve_st pointer(success), NULL(error)
 * caution:
 */
eccp_curve_st * eccp_curve_init(eccp_sec_ctx_t *ctx, const eccp_curve_st *curve)
{
    uint32_t pWordLen, nWordLen;
    eccp_curve_st * ctx_curve = ctx->curve;

    if((curve->eccp_p_bitLen < 160U) || (curve->eccp_p_bitLen > ECCP_MAX_BIT_LEN))
    {
        ctx_curve = NULL;
    }
    else
    {
        pWordLen = GET_WORD_LEN(curve->eccp_p_bitLen);
        nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);

        ctx_curve->eccp_p_bitLen = curve->eccp_p_bitLen;
        ctx_curve->eccp_n_bitLen = curve->eccp_n_bitLen;
        ctx_curve->eccp_p        = ctx->eccp_curve_mem;
        ctx_curve->eccp_p_h      = &ctx_curve->eccp_p[pWordLen];
        ctx_curve->eccp_a        = &ctx_curve->eccp_p_h[pWordLen];
        ctx_curve->eccp_b        = &ctx_curve->eccp_a[pWordLen];
        ctx_curve->eccp_Gx       = &ctx_curve->eccp_b[pWordLen];
        ctx_curve->eccp_Gy       = &ctx_curve->eccp_Gx[pWordLen];
        ctx_curve->eccp_n        = &ctx_curve->eccp_Gy[pWordLen];
        ctx_curve->eccp_n_h      = &ctx_curve->eccp_n[nWordLen];
#if (defined(PKE_LP) || defined(PKE_SECURE))
        ctx_curve->eccp_n_n0     = &ctx_curve->eccp_n_h[nWordLen];
        ctx_curve->eccp_p_n0     = &ctx_curve->eccp_n_n0[1];
#else
        ctx_curve->eccp_half_Gx  = &ctx_curve->eccp_n_h[nWordLen];
        ctx_curve->eccp_half_Gy  = &ctx_curve->eccp_half_Gx[pWordLen];
#endif

        uint32_copy(ctx->eccp_curve_mem, curve->eccp_p, pWordLen);                  //p
        if (NULL != curve->eccp_p_h)
        {
            uint32_copy(&ctx->eccp_curve_mem[pWordLen], curve->eccp_p_h, pWordLen); //p_h
        }
        else
        {
            ctx_curve->eccp_p_h = NULL;
        }

#if (defined(PKE_LP) || defined(PKE_SECURE))
        if (NULL != curve->eccp_p_n0)
        {
            uint32_copy(&ctx->eccp_curve_mem[pWordLen*6u+nWordLen*2u+1u], curve->eccp_p_n0, 1);  //p_n0
        }
        else
        {
            ctx_curve->eccp_p_n0 = NULL;
        }
#endif

        uint32_copy(&ctx->eccp_curve_mem[pWordLen<<1], curve->eccp_a,  pWordLen);  //a
        uint32_copy(&ctx->eccp_curve_mem[pWordLen*3u], curve->eccp_b,  pWordLen);  //b
        uint32_copy(&ctx->eccp_curve_mem[pWordLen<<2], curve->eccp_Gx, pWordLen);  //Gx
        uint32_copy(&ctx->eccp_curve_mem[pWordLen*5u], curve->eccp_Gy, pWordLen);  //Gy
        uint32_copy(&ctx->eccp_curve_mem[pWordLen*6u], curve->eccp_n,  nWordLen);  //n

        if (NULL != curve->eccp_n_h)
        {
            uint32_copy(&ctx->eccp_curve_mem[(pWordLen*6u)+nWordLen], curve->eccp_n_h, nWordLen);     //n_h
        }
        else
        {
            ctx_curve->eccp_n_h = NULL;
        }

#if (defined(PKE_LP) || defined(PKE_SECURE))
        if (NULL != curve->eccp_n_n0)
        {
            uint32_copy(&ctx->eccp_curve_mem[pWordLen*6u+nWordLen*2u], curve->eccp_n_n0, 1);       //n_n0
        }
        else
        {
            ctx_curve->eccp_n_n0 = NULL;
        }
#else
        uint32_copy(&ctx->eccp_curve_mem[(pWordLen*6u)+(nWordLen*2u)], curve->eccp_half_Gx, pWordLen);  //half_Gx
        uint32_copy(&ctx->eccp_curve_mem[(pWordLen*7u)+(nWordLen*2u)], curve->eccp_half_Gy, pWordLen);  //half_Gy
#endif
    }

    return ctx_curve;
}


/* function: uninit eccp curve struct
 * parameters: 
 *     ctx ------------------------ input, eccp_sec_ctx_t struct pointer
 * return: none
 * caution:
 */
void eccp_curve_uninit(eccp_sec_ctx_t *ctx)
{
#if 0
    uint32_clear((uint32_t *)ctx, (sizeof(eccp_sec_ctx_t))/4);
#else
    uint32_t i = (sizeof(eccp_sec_ctx_t))>>2;
    uint32_t *a = ctx->eccp_curve_mem;

    while(i > 15U) //or while(i >= 16U)
    {
        a[i-1U] = 0U;
        a[i-2U] = 0U;
        a[i-3U] = 0U;
        a[i-4U] = 0U;

        a[i-5U] = 0U;
        a[i-6U] = 0U;
        a[i-7U] = 0U;
        a[i-8U] = 0U;

        a[i-9U] = 0U;
        a[i-10U] = 0U;
        a[i-11U] = 0U;
        a[i-12U] = 0U;

        a[i-13U] = 0U;
        a[i-14U] = 0U;
        a[i-15U] = 0U;
        a[i-16U] = 0U;

        i -= 16U;
    }

    while(i > 7U) //or while(i >= 8U)
    {
        a[i-1U] = 0U;
        a[i-2U] = 0U;
        a[i-3U] = 0U;
        a[i-4U] = 0U;

        a[i-5U] = 0U;
        a[i-6U] = 0U;
        a[i-7U] = 0U;
        a[i-8U] = 0U;

        i -= 8U;
    }

    while(i != 0U) //or while(i >= 1U)
    {
        i--;
        a[i] = 0U;
    }
#endif
}

#endif

