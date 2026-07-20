#ifndef _ECCP_H_
#define _ECCP_H_
/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "Type.h"
#include "PAElib.h"
#include "ECCP_curve.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/
 typedef struct
{
    U32 *x; 
    U32 *y; 
}ECCP_POINT;

typedef struct
{
	U32 *x; 
	U32 *y;
	U32 *z;
}ECCP_JACOBIPOINT;

/***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/
//定义返回值错误码
enum ECCP_RET_CODE
{
	ECCPSuccess = 0, 
	ECCPBufferNull,            // 空指针
	ECCPInputLenInvalid,       // 输入长度非法，如长度为0等
	ECCPPointHeadNot04,        // 公钥或曲线上点不以04开头
	ECCPPubKeyError,           // 公钥错误
	ECCPNotInCurve,            // 点不在曲线上
	ECCPIntegerTooBig,         // 整数，如随机数K等过大
	ECCPZeroALL,               // 全0
	ECCPDecryVerifyFailed,     // 解密校验失败
	ECCPVerifyFailed,          // 签名验证失败
	ECCPExchangeRoleInvalid,   // 密钥交换用户角色无效
	ECCPZeroPoint,             // 0点，即无穷远点
	ECCPInOutSameBuffer        // 输入输出同一个buffer
};
/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
void ECCP_Init(U32 *p, U32 *a, U32 *b, U32 *Gx, U32 *Gy, U32 *n, U32 PBitLen, U32 NBitLen, U32 COMB_n, U32 *COMB_iGR, U32 *COMB_iG);

void ECCP_Init_1(ECCP_CURVE *curve);

void ECCP_Point2Char(ECCP_POINT *P, U8 *c);

void ECCP_Char2Point(U8 *c, ECCP_POINT *P);

U8 ECCP_IntegerCheck(U32 k[], U32 kWORDLEN, U32 n[], U32 nWORDLEN);

U32 ECCP_ModAdd(U32 *a, U32 aWordLen, U32 *b, U32 bWordLen, U32 *N);

U32 ECCP_ModSub(U32 *a, U32 aWordLen, U32 *b, U32 bWordLen, U32 *N);

U8 ECCP_PointMul(U32 *k, U32 kWordLen, ECCP_POINT *P, ECCP_POINT *Q);

U8 ECCP_PointMul_G(U32 *k, U32 kWordLen, ECCP_POINT *Q);

U8 ECCP_PointMul_Comb(U32 *k, U32 kWordLen, U8 n, U32 *iG, ECCP_POINT *Q);

U8 ECCP_PointMul_Comb_Residue(U32 *k, U32 kWordLen, U8 n, U32 *iGR, ECCP_POINT *Q);

U8 ECCP_PointAdd(ECCP_POINT *Pin, ECCP_POINT *Pin2, ECCP_POINT *Pout);

U8 ECCP_TestPoint(ECCP_POINT *P);

void ECCP_Close(void);

#endif
