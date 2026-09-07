#ifndef _AES_H
#define _AES_H

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
/*
typedef unsigned char	    U_8;
typedef unsigned short int	U_16;
typedef unsigned long	    U_32;
typedef char			    S_8 ;
*/

typedef struct {
	U8 xts_aes_nr;
	U8 xts_aes_en_de;
	U8 xts_aes_first_block;
	U32 xts_aes_rk[60];
	U8 xts_aes_t[16];
} XTS_AES_CTX;

//CCM模式下context定义
typedef struct {
	U8 M;
	U8 L;
	U32 T[4];
} CCM_AES_CTX;

typedef struct
{
	U32 T[4];
	U32 msg_buf[4];
	U8 leftByteLen;
	U8 macByteLen;
} aes_cmac_ctx;

typedef struct
{
	U32 T[4];
	U32 msg_buf[4];
	U8 leftByteLen;
	U8 macByteLen;
	U8 padding;
} aes_cbc_mac_ctx;

typedef enum
{
	AES_NO_PADDING,
	AES_ZERO_PADDING,
} AES_PADDING;
/***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
U_8 AES_KeyExpansion(U_8 *key, U_8 keyByteLen, U_8 En_De);
U_8 AES_Crypto(U_8 *in, int inByteLen, U_8 mode, U_8 iv[16], U_8 *out);//ECB CBC


U8 AES_CFB_1(U8 *in, U32 inBitLen, U8 IV[16], U8 En_De, U8 *out);
U8 AES_CFB_8(U8 *in, U32 inByteLen, U8 IV[16], U8 En_De, U8 *out);
U8 AES_CFB_128(U8 *in, U32 inByteLen, U8 IV[16], U8 En_De, U8 *out);

U8 AES_OFB_1(U8 *in, U32 inBitLen, U8 IV[16], U8 *out);
U8 AES_OFB_8(U8 *in, U32 inByteLen, U8 IV[16], U8 *out);
U8 AES_OFB_128(U8 *in, U32 inByteLen, U8 IV[16], U8 *out);

U8 AES_CTR(U8 *in, U32 inByteLen, U8 CTR[16], U8 *out);

U8 AES_GCM_Encrypt(U8 *P, U32 PBitLen, U8 *iv, U32 ivBitLen, U8 *A, U32 ABitLen, U8 *C, U8 *T, U32 TBitLen);

U8 AES_GCM_Decrypt(U8 *C, U32 CBitLen, U8 *iv, U32 ivBitLen, U8 *A, U32 ABitLen, U8 *P, U8 *T, U32 TBitLen);


U8 XTS_AES_init(XTS_AES_CTX *ctx, U8 *key, U8 keyByteLen, U8 i[16], U8 En_De);
U8 XTS_AES_crypto_block(XTS_AES_CTX *ctx, U8 *in, U32 inByteLen, U8 *out);
U8 XTS_AES_crypto_remainder(XTS_AES_CTX *ctx, U8 *in, U32 inByteLen, U8 *out);
U8 XTS_AES_crypto(U8 *key, U8 keyByteLen, U8 i[16], U8 En_De, U8 *in, U32 inByteLen, U8 *out);//单个指令


U8 AES_CCM_encrypt(U8 *key, U8 keyByteLen, U8 *nonce, U8 M, U8 L, U8 *a, U64 aByteLen, U8 *msg, U64 msgByteLen, U8 *cipher, U8 *U);
U8 AES_CCM_decrypt(U8 *key, U8 keyByteLen, U8 *nonce, U8 M, U8 L, U8 *a, U64 aByteLen, U8 *cipher, U64 cipherByteLen, U8 *msg, U8 *U);

U8 AES_CMAC_init(aes_cmac_ctx *ctx, U8 *key, U8 keyByteLen, U8 macByteLen);
U8 AES_CMAC_update(aes_cmac_ctx *ctx, U8 *msg, U32 msgByteLen);
U8 AES_CMAC_final(aes_cmac_ctx *ctx, U8 *mac);
U8 AES_CMAC(U8 *key, U8 keyByteLen, U8 *msg, U32 msgByteLen, U8 *mac, U8 macByteLen);

U8 AES_CBC_MAC_init(aes_cbc_mac_ctx *ctx, U8 *key, U8 keyByteLen, AES_PADDING padding, U8 macByteLen);
U8 AES_CBC_MAC_update(aes_cbc_mac_ctx *ctx, U8 *msg, U32 msgByteLen);
U8 AES_CBC_MAC_final(aes_cbc_mac_ctx *ctx, U8 *mac);
U8 AES_CBC_MAC(U8 *key, U8 keyByteLen, U8 *msg, U32 msgByteLen, AES_PADDING padding, U8 *mac, U8 macByteLen);
#endif
