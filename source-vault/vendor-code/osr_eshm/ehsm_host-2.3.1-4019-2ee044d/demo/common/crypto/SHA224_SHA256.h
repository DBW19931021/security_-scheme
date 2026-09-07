#ifndef SHA224_SHA256_H
#define SHA224_SHA256_H
/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/
typedef unsigned int   U32;
typedef unsigned short U16;
typedef unsigned char  U8;
typedef signed char   S8;


/* type to hold the SHA256 context */
typedef struct
{   U32 count[2];   
    U32 *hash;    //hash[8];
    U32 wbuf[16];
}SHA256_Ctx;

/* type to hold the SHA224 context */
typedef SHA256_Ctx SHA224_Ctx;


/* type to hold the HMAC_SHA256 context */
typedef struct
{     
    U32 K0[16];
    SHA256_Ctx sha256_ctx[1];
}HMAC_SHA256_Ctx;

/* type to hold the HMAC_SHA224 context */
typedef HMAC_SHA256_Ctx HMAC_SHA224_Ctx;

/***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
void SHA256_Init(SHA256_Ctx * ctx, U8 digest[32]);
void SHA256_Process(SHA256_Ctx * ctx, U8 * message, U32 byteLen);
void SHA256_Done(SHA256_Ctx * ctx);
void SHA256_Hash(U8 * message, U32 byteLen, U8 digest[32]);

void SHA224_Init(SHA224_Ctx * ctx, U8 digest[32]);
void SHA224_Process(SHA224_Ctx * ctx, U8 * message, U32 byteLen);
void SHA224_Done(SHA224_Ctx * ctx);
void SHA224_Hash(U8 * message, U32 byteLen, U8 digest[32]);

void HMAC_SHA256_Init(HMAC_SHA256_Ctx *ctx, U8 *key, U32 keyByteLen, U8 *mac);
void HMAC_SHA256_Process(HMAC_SHA256_Ctx *ctx, const U8 *input, U32 byteLen);
void HMAC_SHA256_Done(HMAC_SHA256_Ctx *ctx);
void HMAC_SHA256(U8 *key, U32 keyByteLen, U8 *msg, U32 msgByteLen, U8 *mac);

void HMAC_SHA224_Init(HMAC_SHA224_Ctx *ctx, U8 *key, U32 keyByteLen, U8 *mac);
void HMAC_SHA224_Process(HMAC_SHA224_Ctx *ctx, const U8 *input, U32 byteLen);
void HMAC_SHA224_Done(HMAC_SHA224_Ctx *ctx);
void HMAC_SHA224(U8 *key, U32 keyByteLen, U8 *msg, U32 msgByteLen, U8 *mac);

#endif
