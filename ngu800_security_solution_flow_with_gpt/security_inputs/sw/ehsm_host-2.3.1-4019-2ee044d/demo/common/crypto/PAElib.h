#ifndef _PAElib_H_
#define _PAElib_H_
/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include <stdlib.h>
#include "Type.h"
/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/

 /***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
void U32_endian_convert(U8 *in, U8 *out, U32 wordLen);
void U32_BigInt_reverse(U32 *in, U32 *out, U32 wordLen);
void U8_BigInt_reverse(U8 *in, U8 *out, U32 byteLen);
void U8_BigInt_reverse_with_padding0(U8 *in, U8 *out, U32 byteLen);
void U32_Copy(U32 dst[], U32 src[], U32 wordLen);
void U32_Clear(U32 a[], U32 wordLen);
U8 Bigint_Check_1(U32 a[], U32 aWordLen);
U8 Bigint_Check_p_1(U32 a[], U32 p[], U32 wordLen);
U8 Bigint_Check_NULL(U32 a[], U32 aWordLen);
U8 BigintU8_Check_NULL(U8 a[], U32 aByteLen);
U32 Get_WordLen(U32 a[], U32 aWordLen);
U32 Get_BitLen(U32 a[], U32 aWordLen);
U8  Get_BitValue(U32 a[], U32 bitLen);
U32 Get_Multiple2_Number(U32 a[]);
U8  Big_Compare(U32 a[], U32 aWordLen, U32 b[], U32 bWordLen);
U32 Big_Div2(U32 a[], S32 aWordLen);
U32 Big_Div2_(U32 a[], S32 aWordLen, U32 out[]);
U32 Big_Div2n(U32 a[], S32 aWordLen, U32 n);
U32 Big_Mul2(U32 a[], S32 aWordLen);
U32 Big_Mul2_(U32 a[], S32 aWordLen, U32 out[]);
U32 Big_Mul2n(U32 a[], S32 aWordLen, U8 n);
U32 Big_Add(U32 a[], U32 aWordLen, U32 b[], U32 bWordLen);
U32 Big_Sub(U32 a[], U32 aWordLen, U32 b[], U32 bWordLen);
U32 Big_Subb(U32 a[], U32 aWordLen, U32 b[], U32 bWordLen);
U8 Big_Sign_Add(U32 a[], U8 flag_a, U32 *aWordLen, U32 b[], U8 flag_b, U32 bWordLen);
U8 Big_Sign_Sub(U32 a[], U8 flag_a, U32 *aWordLen, U32 b[], U8 flag_b, U32 bWordLen);
U8 Big_ModInv(U32 x[], U32 xWordLen, U32 n[], U32 nWordLen, U32 xinv[], U32 *xinvWordLen);
U8 Big_ModInv_Odd_1(U32 x[], U32 xWordLen, U32 n[], U32 nWordLen, U32 xinv[], U32 *xinvWordLen);
U8 Big_MontPartInv(U32 x[], U32 xWordLen, U32 n[], U32 nWordLen, U32 xinv[], U32 *xinvWordLen, U32 *k);
U8 Big_ModInv_Odd_2(U32 x[], U32 xWordLen, U32 n[], U32 nWordLen, U32 xinv[], U32 *xinvWordLen);
U8 Big_Div(U32 a[], U32 aWordLen, U32 b[], U32 bWordLen, U32 q[], U32 *qWordLen, U32 r[], U32 *rWordLen);
U8 Big_Div_U32(U32 a[], U32 aWordLen, U32 b, U32 q[], U32 *qWordLen, U32 r[]);
U8 Big_Mod_1(U32 a[], U32 aWordLen, U32 b[], U32 bWordLen, U32 c[], U32 *cWordLen);
U8 Big_Mod_2(U32 a[], U32 aWordLen, U32 b[], U32 bWordLen, U32 c[], U32 *cWordLen);
U32 Mod64_16(U64 a, U64 b);
U8 Big_Mod_U32_1(U32 a[], U32 aWordLen, U32 b, U32 *c);
U8 Big_Mod_U32_2(U32 a[], U32 aWordLen, U32 b, U32 *c);
U8 Big_Mod_U32_3(U32 a[], U32 aWordLen, U32 b, U32 *c);
U8 Big_Mod_U16(U32 a[], U32 aWordLen, U16 b, U16 *c);
void Big_Mul_1(U32 a[], U32 aWordLen, U32 b[], U32 bWordLen, U32 c[], U32 *cWordLen);
void Big_Mul_2(U32 a[], U32 aWordLen, U32 b[], U32 bWordLen, U32 c[], U32 *cWordLen);
void Big_Square(U32 a[], U32 aWordLen, U32 c[], U32 *cWordLen);
void MontCal_n1(U32 N[]);
//void Change_N();
U8 MontMulFIPS_AA(U32 A[], U32 *AWordLen, U32 N[], U32 NWordLen);
U8 MontMulFIPS_AB(U32 A[], U32 *AWordLen, U32 B[], U32 BWordLen, U32 N[], U32 NWordLen);
U8 MontMulFIPS_AB_(U32 A[], U32 AWordLen, U32 B[], U32 BWordLen, U32 N[], U32 NWordLen, U32 out[], U32 * outWordLen);
U8 MontMulCIOS_AB(U32 A[], U32 *AWordLen, U32 B[], U32 BWordLen, U32 N[], U32 NWordLen);
void Big_a2aR_mod_n(U32 a[], U32 n[], U32 aR[], U32 nWordLen);
void Big_aR2a_mod_n(U32 aR[], U32 n[], U32 out[], U32 nWordLen);
U8 Big_ModExp(U32 a[], U32 aWordLen, U32 e[], U32 eWordLen, U32 n[], U32 nWordLen, U32 c[], U32 *cWordLen);
U8 Big_ModExp_SW(U32 a[], U32 aWordLen, U32 e[], U32 eWordLen, U32 n[], U32 nWordLen, U32 c[], U32 *cWordLen);
U8 Big_ModMul_1(U32 a[], U32 aWordLen, U32 b[], U32 bWordLen, U32 n[], U32 nWordLen, U32 c[], U32 *cWordLen);
U8 Big_ModMul_2(U32 a[], U32 aWordLen, U32 b[], U32 bWordLen, U32 n[], U32 nWordLen, U32 c[], U32 *cWordLen);


#endif
