#ifndef _SM4_H_
#define _SM4_H_
/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "Type.h"
//#include "routine.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/

 /***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/
/* data type 
typedef unsigned char U8;
typedef signed char S8;
typedef unsigned int U32;
*/

//定义加解密操作
typedef enum 
{
	SM4_DECRYPT = 0,
	SM4_ENCRYPT
}SM4_CRYPT;


//XTS模式下context定义
typedef struct {
	U32 xts_sm4_rk_buf[32];
	U32 xts_sm4_t[4];
	SM4_CRYPT xts_sm4_en_de;
	U8 xts_sm4_first_block;
} XTS_SM4_CTX;

//CCM模式下context定义
typedef struct {
	U8 M;
	U8 L;
	U32 T[4];
} CCM_SM4_CTX;

typedef struct
{
	U32 T[4];
	U32 msg_buf[4];
	U8 leftByteLen;
	U8 macByteLen;
} sm4_cmac_ctx;

typedef struct
{
	U32 T[4];
	U32 msg_buf[4];
	U8 leftByteLen;
	U8 macByteLen;
	U8 padding;
} sm4_cbc_mac_ctx;

typedef enum
{
	SM4_NO_PADDING,
	SM4_ZERO_PADDING,
} SM4_PADDING;

/***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/
//定义返回值错误码
enum SM4_RET_CODE
{
	SM4Success = 0, 
	SM4BufferNull,            // 空指针
	SM4InputTooLong,          // 输入消息太长
	SM4InputLenInvalid,       // 输入长度非法，如长度为0，或者非分组长度倍数等
	SM4CryptInvalid,          // 非加解密操作，无效
	SM4InOutSameBuffer,       // 输入输出同一个buffer
	SM4GCMCheckFail,          // GCM模式解密校验失败
	SM4CCMCheckFail           // CCM模式解密校验失败
};

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
/***************************************************************************************************
 *     与SM3中的情况类似，但对某些不支持非字对齐的平台，需保证U8类型的入参地址字对齐：
 *     SM4_Init中的key
 *     SM4_ECB中的in和out
 *     SM4_CBC中的in和out
 *     SM4_CTR中的in和out
 ***************************************************************************************************/


U8 SM4_Init(U8 key[16]);

U8 SM4_ECB(U8 *in, U32 inByteLen, U8 En_De, U8 *out);

U8 SM4_CBC(U8 *in, U32 inByteLen, U8 iv[16], U8 En_De, U8 *out);

U8 SM4_CFB_1(U8 *in, U32 inBitLen, U8 IV[16], U8 En_De, U8 *out);

U8 SM4_CFB_8(U8 *in, U32 inByteLen, U8 IV[16], U8 En_De, U8 *out);

U8 SM4_CFB_128(U8 *in, U32 inByteLen, U8 IV[16], U8 En_De, U8 *out);

U8 SM4_OFB_1(U8 *in, U32 inBitLen, U8 IV[16], U8 *out);

U8 SM4_OFB_8(U8 *in, U32 inByteLen, U8 IV[16], U8 *out);

U8 SM4_OFB_128(U8 *in, U32 inByteLen, U8 IV[16], U8 *out);

U8 SM4_CTR(U8 *in, U32 inByteLen, U8 CTR[16], U8 *out);

U8 SM4_GCM_Encrypt_Bit(U8 *P, U32 PBitLen, U8 *iv, U32 ivBitLen, U8 *A, U32 ABitLen, U8 *C, U8 *T, U32 TBitLen);

U8 SM4_GCM_Decrypt_Bit(U8 *C, U32 CBitLen, U8 *iv, U32 ivBitLen, U8 *A, U32 ABitLen, U8 *P, U8 *T, U32 TBitLen);

U8 SM4_GCM_Encrypt(U8 *P, U32 PByteLen, U8 *iv, U32 ivByteLen, U8 *A, U32 AByteLen, U8 *C, U8 *T, U32 TByteLen);

U8 SM4_GCM_Decrypt(U8 *C, U32 CByteLen, U8 *iv, U32 ivByteLen, U8 *A, U32 AByteLen, U8 *P, U8 *T, U32 TByteLen);

U8 SM4_Close(void);


U8 XTS_SM4_init(XTS_SM4_CTX *ctx, U8 key[32], U8 i[16], SM4_CRYPT En_De);
U8 XTS_SM4_crypto_block(XTS_SM4_CTX *ctx, U8 *in, U32 inByteLen, U8 *out);
U8 XTS_SM4_crypto_remainder(XTS_SM4_CTX *ctx, U8 *in, U32 inByteLen, U8 *out);
U8 XTS_SM4_crypto(U8 key[32], U8 i[16], SM4_CRYPT En_De, U8 *in, U32 inByteLen, U8 *out);

U8 SM4_CCM_encrypt(U8 key[16], U8 *nonce, U8 M, U8 L, U8 *a, U64 aByteLen, U8 *msg, U64 msgByteLen, U8 *cipher, U8 *U);
U8 SM4_CCM_decrypt(U8 key[16], U8 *nonce, U8 M, U8 L, U8 *a, U64 aByteLen, U8 *cipher, U64 cipherByteLen, U8 *msg, U8 *U);

U8 SM4_CMAC_init(sm4_cmac_ctx *ctx, U8 *key, U8 macByteLen);
U8 SM4_CMAC_update(sm4_cmac_ctx *ctx, U8 *msg, U32 msgByteLen);
U8 SM4_CMAC_final(sm4_cmac_ctx *ctx, U8 *mac);
U8 SM4_CMAC(U8 *key, U8 *msg, U32 msgByteLen, U8 *mac, U8 macByteLen);

U8 SM4_CBC_MAC_init(sm4_cbc_mac_ctx *ctx, U8 *key, SM4_PADDING padding, U8 macByteLen);
U8 SM4_CBC_MAC_update(sm4_cbc_mac_ctx *ctx, U8 *msg, U32 msgByteLen);
U8 SM4_CBC_MAC_final(sm4_cbc_mac_ctx *ctx, U8 *mac);
U8 SM4_CBC_MAC(U8 *key, U8 *msg, U32 msgByteLen, SM4_PADDING padding, U8 *mac, U8 macByteLen);

#endif
