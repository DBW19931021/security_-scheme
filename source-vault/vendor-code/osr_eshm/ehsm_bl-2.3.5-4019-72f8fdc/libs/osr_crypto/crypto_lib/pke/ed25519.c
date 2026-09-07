#include "../../crypto_include/pke_config.h"


#ifdef SUPPORT_C25519

#include "../../crypto_include/pke/ed25519.h"
#include "../../crypto_include/crypto_common/utility.h"
#include "../../crypto_include/trng/trng.h"
#include "../../crypto_include/hash_hmac/hash.h"


//"SigEd25519 no Ed25519 collisions"
static const uint8_t Ed25519_sign_string[] = {
    0x53,0x69,0x67,0x45,0x64,0x32,0x35,0x35,0x31,0x39,0x20,0x6e,0x6f,0x20,0x45,0x64,
    0x32,0x35,0x35,0x31,0x39,0x20,0x63,0x6f,0x6c,0x6c,0x69,0x73,0x69,0x6f,0x6e,0x73};


/* function: edwards25519 curve point mul(random point), Q=[k]P, secure version
 * parameters:
 *     curve ---------------------- input, edwards25519 curve struct pointer
 *     k -------------------------- input, scalar, it could be 0 here
 *     Px ------------------------- input, x coordinate of point P
 *     Py ------------------------- input, y coordinate of point P
 *     Qx ------------------------- output, x coordinate of point Q
 *     Qy ------------------------- output, y coordinate of point Q
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure input point P is on the curve
 *     2. even if the input point P is valid, the output may be neutral point (0,1), it is valid
 *     3. please make sure the curve is edwards25519
 *     4. k could be zero here.
 *     5. please set hardware operand width 256u before calling this.
 *     6. before calling this function, please make sure the modulus and the pre-calculated mont 
 *        arguments of modulus are located in the right address.
 */
static uint32_t ed25519_pointMul_s_internal(const edward_curve_st *curve, const uint32_t *k, 
        const uint32_t *Px, const uint32_t *Py, uint32_t *Qx, uint32_t *Qy)
{
#if 1
    uint32_t ret;

    if(0u != uint32_BigNum_Check_Zero(k, 8u))
    {
        uint32_clear_8_words(Qx);
        pke_set_operand_uint32_value_256bits(Qy, 1u);

        ret = PKE_SUCCESS;
    }
    else
    {
        ret = ed25519_pointMul_internal(curve, k, Px, Py, Qx, Qy);
    }

    return ret;
#else
    uint32_t ret;
    uint32_t pWordLen = GET_WORD_LEN(curve->p_bitLen);
    uint32_t nWordLen = GET_WORD_LEN(curve->n_bitLen);

    if(0u != uint32_BigNum_Check_Zero(k, nWordLen))
    {
        uint32_clear(Qx, pWordLen);
        pke_set_operand_uint32_value_256bits(Qy, 1u);

        ret = PKE_SUCCESS;
    }
    else
    {
        ret = ed25519_pointMul_internal(curve, k, Px, Py, Qx, Qy);
    }

    return ret;
#endif
}


/* function: edwards25519 curve point mul(random point), Q=[k]P, secure version
 * parameters:
 *     curve ---------------------- input, edwards25519 curve struct pointer
 *     k -------------------------- input, scalar, it could be 0 here
 *     Px ------------------------- input, x coordinate of point P
 *     Py ------------------------- input, y coordinate of point P
 *     Qx ------------------------- output, x coordinate of point Q
 *     Qy ------------------------- output, y coordinate of point Q
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure input point P is on the curve
 *     2. even if the input point P is valid, the output may be neutral point (0,1), it is valid
 *     3. please make sure the curve is edwards25519
 *     4. k could be zero here.
 */
static uint32_t ed25519_pointMul_s(const edward_curve_st *curve, const uint32_t *k, 
        const uint32_t *Px, const uint32_t *Py, uint32_t *Qx, uint32_t *Qy)
{
    uint32_t ret;

#if 0
    ret = pke_set_modulus_and_pre_monts(ed25519->p, ed25519->p_h, ed25519->p_bitLen);
#else
    ret = pke_load_modulus_and_pre_monts_256bits(ed25519->p, ed25519->p_h);
#endif
    if(PKE_SUCCESS == ret)
    {
        ret = ed25519_pointMul_s_internal(curve, k, Px, Py, Qx, Qy);
    }
    else
    {}

    return ret;
}


/* Function: get Ed25519 public key from private key
 * Parameters:
 *     prikey --------------------- input, private key, 32 bytes, little-endian
 *     pubkey --------------------- output, public key, 32 bytes, little-endian
 * Return: EdDSA_SUCCESS(success); other(error)
 * Caution:
 *     1.
 */
uint32_t ed25519_get_pubkey_from_prikey(const uint8_t prikey[32], uint8_t pubkey[32])
{
    uint32_t h[16];
    uint32_t ret;

    if((NULL == prikey) || (NULL == pubkey))
    {
        ret = EdDSA_POINTOR_NULL;
    }
    else
    {
        ret = hash(HASH_SHA512, prikey, 32u, (uint8_t *)h);
        if(HASH_SUCCESS == ret)
        {
            //decode to get the scalar
            x25519_ed25519_decode_scalar((uint8_t *)h, (uint8_t *)h);

            ret = ed25519_pointMul_s(ed25519, h, ed25519->Gx, ed25519->Gy, h, &h[8]);
            if(PKE_SUCCESS == ret)
            {
                //encode pubkey
                memcpy_(pubkey, (uint8_t *)(&h[8]), Ed25519_BYTE_LEN);
                if(0u != (h[0]&1u))
                {
                    pubkey[Ed25519_BYTE_LEN-1u] |= (uint8_t)0x80;
                }
                else
                {}

                ret = EdDSA_SUCCESS;
            }
            else
            {}
        }
        else
        {}
    }

    return ret;
}


/* Function: generate Ed25519 random key pair
 * Parameters:
 *     prikey --------------------- output, private key, 32 bytes, little-endian
 *     pubkey --------------------- output, public key, 32 bytes, little-endian
 * Return: EdDSA_SUCCESS(success); other(error)
 * Caution:
 *     1.
 */
uint32_t ed25519_getkey(uint8_t prikey[32], uint8_t pubkey[32])
{
    uint32_t ret;

    if((NULL == prikey) || (NULL == pubkey))
    {
        ret = EdDSA_POINTOR_NULL;
    }
    else
    {
        ret = get_rand(prikey, Ed25519_BYTE_LEN);
        if(TRNG_SUCCESS == ret)
        {
            ret = ed25519_get_pubkey_from_prikey(prikey, pubkey);
        }
        else
        {}
    }

    return ret;
}


/* Function: Ed25519 sign/verify check input
 * Parameters:
 *     mode ----------------------- input, Ed25519 signature mode
 *     prikey_pubkey -------------- input, pointer to private key or public key, 32 bytes, U8 little-endian
 *     ctx ------------------------ input, pointer to ctx, 0-255 bytes
 *     ctx_bytes ------------------ input, pointer to byte length of ctx
 *     M -------------------------- input, pointer to the message, requirements are determined by mode
 *     msg_bytes ------------------ input, pointer to byte length of M, requirements are determined by mode
 *     RS ------------------------- input, pointer to signature
 * Return: PKE_SUCCESS(success); other(error)
 * Caution:
 *     1. for signing, prikey_pubkey is pointer to private key, for verifying, prikey_pubkey 
 *        is pointer to public key
 *     2. if mode is not Ed25519_PH_WITH_PH_M, M could be empty(please set M to be NULL), 
 *        so no need to check M and msg_bytes, otherwise, M is sha512 digest, occupies 64 bytes, 
 *        and msg_bytes is not involved actually
 *     3. if mode is Ed25519_DEFAULT, ctx is not involved, no need to check ctx and ctx_bytes
 *     4. if mode is Ed25519_CTX, ctx can not be empty(ctx_bytes is from 1 to 255)
 *     5. if mode is Ed25519_PH or Ed25519_PH_WITH_PH_M, ctx_bytes is from 0 to 255, default 
 *        value is 0, thus ctx could be empty
 */
static uint32_t ed25519_check_input(ed25519_mode_e mode, const uint8_t *prikey_pubkey,
        const uint8_t *ctx, uint8_t *ctx_bytes, const uint8_t *M, uint32_t *msg_bytes, 
        const uint8_t *RS)
{
    uint32_t ret;

    if(mode > Ed25519_PH_WITH_PH_M)
    {
        ret = EdDSA_INVALID_INPUT;
    }
    else if((NULL == prikey_pubkey) || (NULL == RS))
    {
        ret = EdDSA_POINTOR_NULL;
    }
    else
    {
        ret = PKE_SUCCESS;

        //if mode is not Ed25519_PH_WITH_PH_M, M could be empty, 
        //so M could be NUll, msg_bytes could be 0, no need to check them
        if(Ed25519_PH_WITH_PH_M != mode)
        {
            if(NULL == M)
            {
                *msg_bytes = 0u;
            }
            else
            {}
        }
        else
        {
            if(NULL == M)
            {
                ret = EdDSA_INVALID_INPUT;
            }
            else
            {
                *msg_bytes = 64u;  //SHA512 digest
            }
        }
    }

    if(PKE_SUCCESS == ret)
    {
        if((Ed25519_CTX == mode) && ((NULL == ctx) || (((uint8_t)0) == ctx_bytes[0])))      //in this case ctx can not be empty
        {
            ret = EdDSA_INVALID_INPUT;
        }
        else if(((Ed25519_PH == mode) || (Ed25519_PH_WITH_PH_M == mode)) && (NULL == ctx))  //in this case ctx could be empty
        {
            *ctx_bytes = 0;
        }
        else
        {
            //Ed25519_DEFAULT mode, ctx is useless
        }
    }
    else
    {}

    return ret;
}


/* Function: Ed25519 sign step 1(internal API)
 * Parameters:
 *     mode ----------------------- input, Ed25519 signature mode
 *     phflag --------------------- output, if mode is Ed25519_PH or Ed25519_PH_WITH_PH_M, phflag is 1,
 *                                         and if mode is Ed25519_CTX, phflag is 0, otherwise phflag is useless 
 *     prikey --------------------- input, public key, 32 bytes, little-endian
 *     M -------------------------- input, message, requirements are determined by mode
 *     msg_bytes ------------------ input, byte length of M, requirements are determined by mode
 *     digest --------------------- output, SHA512(prikey), actually it is secret scalar s and prefix
 *     PH_M ----------------------- output, if mode is Ed25519_PH, it is SHA512(M), otherwise it is not involved
 * Return: PKE_SUCCESS(success); other(error)
 * Caution:
 *     1. if mode is not Ed25519_PH_WITH_PH_M, M could be empty(please set M to be NULL), 
 *        so no need to check M and msg_bytes, otherwise, M is sha512 digest, occupies 64 bytes, 
 *        and msg_bytes is not involved
 */
static uint32_t ed25519_sign_internal_step_1(ed25519_mode_e mode, uint8_t *phflag, 
        const uint8_t *prikey, const uint8_t *M, uint32_t msg_bytes, 
        uint8_t *digest, uint8_t *PH_M)
{
    uint32_t ret;

    //get private scalar s and prefix 
    ret = hash(HASH_SHA512, prikey, Ed25519_BYTE_LEN, digest);
    if(HASH_SUCCESS == ret)
    {
        //decode to get the scalar s
        x25519_ed25519_decode_scalar((uint8_t *)digest, (uint8_t *)digest);

        //set flag F
        if(Ed25519_CTX == mode)
        {
            *phflag = 0;
        }
        else if((Ed25519_PH == mode) || (Ed25519_PH_WITH_PH_M == mode))
        {
            *phflag = 1;
        }
        else
        {
            //Ed25519_DEFAULT mode, phflag is useless
        }

        //PH_M
        if(Ed25519_PH == mode)
        {
            ret = hash(HASH_SHA512, M, msg_bytes, (uint8_t *)PH_M);
        }
        else
        {}
    }
    else
    {}

    if(HASH_SUCCESS == ret)
    {
        ret = PKE_SUCCESS;
    }
    else
    {}

    return ret;
}


/* Function: Ed25519 sign step 2(internal API)
 * Parameters:
 *     mode ----------------------- input, Ed25519 signature mode
 *     phflag --------------------- input, if mode is Ed25519_PH or Ed25519_PH_WITH_PH_M, phflag is 1,
 *                                         and if mode is Ed25519_CTX, phflag is 0, otherwise phflag is useless 
 *     ctx ------------------------ input, 0-255 bytes
 *     ctx_bytes ------------------ input, byte length of ctx
 *     prefix --------------------- input, second half of SHA512(prikey), 32 bytes, little-endian
 *     M -------------------------- input, message, requirements are determined by mode
 *     msg_bytes ------------------ input, byte length of M, requirements are determined by mode
 *     PH_M ----------------------- input, SHA512(M), just for Ed25519_PH mode
 *     k -------------------------- output, k = SHA512(dom2(F, C) || prefix || PH(M))
 * Return: PKE_SUCCESS(success); other(error)
 * Caution:
 *     1. if mode is not Ed25519_PH_WITH_PH_M, M could be empty(please set M to be NULL), 
 *        so no need to check M and msg_bytes, otherwise, M is sha512 digest, occupies 64 bytes, 
 *        and msg_bytes is not involved
 */
static uint32_t ed25519_sign_internal_step_2(ed25519_mode_e mode, uint8_t phflag, 
        const uint8_t *ctx, uint8_t ctx_bytes, const uint8_t *prefix, const uint8_t *M, 
        uint32_t msg_bytes, const uint8_t *PH_M, uint8_t *k)
{
    uint32_t ret;
    hash_ctx_st sha512_ctx[1];
    uint8_t buf[2];

    //get k := SHA512(dom2(F, C) || prefix || PH(M))
    ret = hash_init(sha512_ctx, HASH_SHA512);
    if((HASH_SUCCESS == ret) && (Ed25519_DEFAULT != mode))
    {
        //dom2(phflag, ctx)
        ret = hash_update(sha512_ctx, Ed25519_sign_string, sizeof(Ed25519_sign_string));
        if(HASH_SUCCESS == ret)
        {
            buf[0] = phflag;
            buf[1] = ctx_bytes;
            ret = hash_update(sha512_ctx, buf, 2u);
        }
        else
        {}

        if(HASH_SUCCESS == ret)
        {
            ret = hash_update(sha512_ctx, ctx, ctx_bytes);
        }
        else
        {}
    }
    else
    {}

    //prefix
    if(HASH_SUCCESS == ret)
    {
        ret = hash_update(sha512_ctx, prefix, Ed25519_BYTE_LEN);
    }
    else
    {}

    //PH(M)
    if(HASH_SUCCESS == ret)
    {
        if(Ed25519_PH == mode)
        {
            ret = hash_update(sha512_ctx, PH_M, 64u);
        }
        else
        {
            ret = hash_update(sha512_ctx, M, msg_bytes);
        }
    }
    else
    {}

    if(HASH_SUCCESS == ret)
    {
        ret = hash_final(sha512_ctx, (uint8_t *)k);
    }
    else
    {}

    if(HASH_SUCCESS == ret)
    {
        ret = PKE_SUCCESS;
    }
    else
    {}

    return ret;
}


/* Function: out = k mod n
 * Parameters:
 *     k -------------------------- input, 16 words, little-endian
 *     out ------------------------ output, out = k mod n, 8 words, little-endian
 * Return: PKE_SUCCESS(success); other(error)
 * Caution:
 *     1. k and out can not point the same buffer
 */
static uint32_t ed25519_k_mod_n(uint32_t *k, uint32_t *out)
{
    uint32_t ret;

    //out = k mod n
#if defined(PKE_LP)
    ret = pke_mod(&(k[Ed25519_WORD_LEN-1u]), Ed25519_WORD_LEN+1u, ed25519->n, ed25519->n_h, ed25519->n_n0,
        Ed25519_WORD_LEN, out);
#else
    ret = pke_mod(&(k[Ed25519_WORD_LEN-1u]), Ed25519_WORD_LEN+1u, ed25519->n, ed25519->n_h,
        Ed25519_WORD_LEN, out);
#endif
    if(PKE_SUCCESS == ret)
    {
        uint32_copy_8_words(&(k[Ed25519_WORD_LEN-1u]), out);
#if defined(PKE_LP)
        ret = pke_mod(k, (Ed25519_WORD_LEN<<1)-1u, ed25519->n, ed25519->n_h, ed25519->n_n0, Ed25519_WORD_LEN, out);
#else
        ret = pke_mod(k, (Ed25519_WORD_LEN<<1)-1u, ed25519->n, ed25519->n_h, Ed25519_WORD_LEN, out);
#endif
    }
    else
    {}

    return ret;
}


/* Function: Ed25519 sign step 3(internal API)
 * Parameters:
 *     k -------------------------- input, k = SHA512(dom2(F, C) || prefix || PH(M))
 *     r -------------------------- output, r = k mod n, 8 words, little-endian
 *     R -------------------------- output, encode([r]G), and it is the first half of the signature
 * Return: PKE_SUCCESS(success); other(error)
 * Caution:
 *     1. 
 */
static uint32_t ed25519_sign_internal_step_3(uint32_t *k, uint32_t *r, uint8_t *R)
{
    uint32_t ret;

    //r = k mod n
    ret = ed25519_k_mod_n(k, r);
    if(PKE_SUCCESS == ret)
    {
        ret = ed25519_pointMul_s(ed25519, r, ed25519->Gx, ed25519->Gy, k, &k[Ed25519_WORD_LEN]);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        memcpy_(R, (uint8_t *)(&k[Ed25519_WORD_LEN]), Ed25519_BYTE_LEN);
        if(0u != (k[0] & 1u))
        {
            R[Ed25519_BYTE_LEN-1u] |= (uint8_t)0x80;
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* Function: hash update R||A, A is the public key, this is one part of Ed25519 sign step 4(internal API)
 * Parameters:
 *     sha512_ctx ----------------- input, hash_ctx_st struct pointer, must be initialized
 *     R -------------------------- input, first half of the signature, 32 bytes
 *     pubkey --------------------- input, public key, 32 bytes
 *     s -------------------------- input, secret scalar s
 *     k -------------------------- input, tmporary buffer, to store the public key point [s]G if pubkey is NULL
 * Return: HASH_SUCCESS(success); other(error)
 * Caution:
 *     1. if pubkey is NULL, it will generate the public key by scalar s.
 */
static uint32_t ed25519_sign_internal_step_4_hash_update_R_and_pubkey(hash_ctx_st *sha512_ctx, 
        const uint8_t *R, const uint8_t *pubkey, const uint32_t *s, uint32_t *k)
{
    uint32_t ret;

    ret = hash_update(sha512_ctx, R, Ed25519_BYTE_LEN);
    if(HASH_SUCCESS == ret)
    {
        if(NULL == pubkey)
        {
            ret = ed25519_pointMul_s_internal(ed25519, s, ed25519->Gx, ed25519->Gy, 
                    k, &k[Ed25519_WORD_LEN]);
            if(PKE_SUCCESS == ret)
            {
                if(0u != (k[0] & 1u))
                {
                    k[(Ed25519_WORD_LEN<<1)-1u] |= 0x80000000u;
                }
                else
                {}

                ret = hash_update(sha512_ctx, (uint8_t *)(&k[Ed25519_WORD_LEN]), Ed25519_BYTE_LEN);
            }
            else 
            {}
        }
        else
        {
            ret = hash_update(sha512_ctx, pubkey, Ed25519_BYTE_LEN);
        }
    }
    else
    {}

    return ret;
}


/* Function: Ed25519 sign step 4(internal API)
 * Parameters:
 *     mode ----------------------- input, Ed25519 signature mode
 *     phflag --------------------- input, if mode is Ed25519_PH or Ed25519_PH_WITH_PH_M, phflag is 1,
 *                                         and if mode is Ed25519_CTX, phflag is 0, otherwise phflag is useless 
 *     ctx ------------------------ input, 0-255 bytes
 *     ctx_bytes ------------------ input, byte length of ctx
 *     R -------------------------- input, first half of the signature, 32 bytes
 *     s -------------------------- input, secret scalar s
 *     pubkey --------------------- input, public key, 32 bytes
 *     M -------------------------- input, message, requirements are determined by mode
 *     msg_bytes ------------------ input, byte length of M, requirements are determined by mode
 *     PH_M ----------------------- input, SHA512(M), just for Ed25519_PH mode
 *     k -------------------------- output, k = SHA512(dom2(F, C) || R || A || PH(M))
 * Return: PKE_SUCCESS(success); other(error)
 * Caution:
 *     1. if mode is not Ed25519_PH_WITH_PH_M, M could be empty(please set M to be NULL), 
 *        so no need to check M and msg_bytes, otherwise, M is sha512 digest, occupies 64 bytes, 
 *        and msg_bytes is not involved
 */
static uint32_t ed25519_sign_internal_step_4(ed25519_mode_e mode, uint8_t phflag, 
        const uint8_t *ctx, uint8_t ctx_bytes, const uint8_t *R, const uint32_t *s, 
        const uint8_t *pubkey, const uint8_t *M, uint32_t msg_bytes, const uint8_t *PH_M, 
        uint32_t *k)
{
    uint32_t ret;
    hash_ctx_st sha512_ctx[1];
    uint8_t buf[2];

    //get k := SHA512(dom2(F, C) || R || A || PH(M))
    ret = hash_init(sha512_ctx, HASH_SHA512);
    if((HASH_SUCCESS == ret) && (Ed25519_DEFAULT != mode))
    {
        //dom2(phflag, ctx)
#if 1
        ret = hash_update(sha512_ctx, Ed25519_sign_string, sizeof(Ed25519_sign_string));
        if(HASH_SUCCESS == ret)
        {
            buf[0] = phflag;
            buf[1] = ctx_bytes;
            ret = hash_update(sha512_ctx, buf, 2u);
        }
        else
        {}
        
        if(HASH_SUCCESS == ret)
        {
            ret = hash_update(sha512_ctx, ctx, ctx_bytes);
        }
        else
        {}
#else
        memcpy_(sha512_ctx->hash_buffer, Ed25519_sign_string, sizeof(Ed25519_sign_string));
        sha512_ctx->hash_buffer[sizeof(Ed25519_sign_string)] = phflag;
        sha512_ctx->hash_buffer[1u + sizeof(Ed25519_sign_string)] = ctx_bytes;
        sha512_ctx->total[0] = 2u + sizeof(Ed25519_sign_string);
#endif
    }
    else
    {}

    //R and pubkey(A)
    if(HASH_SUCCESS == ret)
    {
        ret = ed25519_sign_internal_step_4_hash_update_R_and_pubkey(sha512_ctx, R, pubkey, s, k);
    }
    else
    {}

    //PH(M)
    if(HASH_SUCCESS == ret)
    {
        if(Ed25519_PH == mode)
        {
            ret = hash_update(sha512_ctx, PH_M, 64u);
        }
        else
        {
            ret = hash_update(sha512_ctx, M, msg_bytes);
        }
    }
    else
    {}

    if(HASH_SUCCESS == ret)
    {
        ret = hash_final(sha512_ctx, (uint8_t *)k);
    }
    else
    {}

    if(HASH_SUCCESS == ret)
    {
        ret = PKE_SUCCESS;
    }
    else
    {}

    return ret;
}


/* Function: Ed25519 sign step 5(internal API)
 * Parameters:
 *     r -------------------------- input, r = SHA512(dom2(F, C) || prefix || PH(M)) mod n, 8 words, little-endian
 *     k -------------------------- input, k = SHA512(dom2(F, C) || R || A || PH(M)), 16 words, little-endian
 *     secret_s ------------------- input, secret scalar s, first hals of SHA512(prikey)
 *     tmp ------------------------ input, temporary buffer, 8 words
 *     S -------------------------- output, S = (r + k * s) mod n, second half of the signature, 32 bytes, little-endian
 * Return: PKE_SUCCESS(success); other(error)
 * Caution:
 *     1. 
 */
static uint32_t ed25519_sign_internal_step_5(const uint32_t *r, uint32_t *k, const uint32_t *secret_s, 
        uint32_t *tmp, uint8_t *S)
{
    uint32_t ret;

    //tmp = k mod n
#if defined(PKE_LP)
    ret = pke_mod(&(k[Ed25519_WORD_LEN-1u]), Ed25519_WORD_LEN+1u, ed25519->n, ed25519->n_h, ed25519->n_n0, Ed25519_WORD_LEN, tmp);
#else
    ret = pke_mod(&(k[Ed25519_WORD_LEN-1u]), Ed25519_WORD_LEN+1u, ed25519->n, ed25519->n_h, Ed25519_WORD_LEN, tmp);
#endif
    if(PKE_SUCCESS == ret)
    {
        uint32_copy_8_words(&k[Ed25519_WORD_LEN-1u], tmp);
#if defined(PKE_LP)
        ret = pke_mod(k, (Ed25519_WORD_LEN<<1)-1u, ed25519->n, ed25519->n_h, ed25519->n_n0, Ed25519_WORD_LEN, tmp);
#else
        ret = pke_mod(k, (Ed25519_WORD_LEN<<1)-1u, ed25519->n, ed25519->n_h, Ed25519_WORD_LEN, tmp);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //k = s mod n
#if defined(PKE_LP)
        ret = pke_mod(secret_s, Ed25519_WORD_LEN, ed25519->n, ed25519->n_h, ed25519->n_n0, Ed25519_WORD_LEN, k);
#else
        ret = pke_mod(secret_s, Ed25519_WORD_LEN, ed25519->n, ed25519->n_h, Ed25519_WORD_LEN, k);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //k = k*s
#if 0
        ret = pke_modmul(ed25519->n, tmp, k, k, Ed25519_WORD_LEN);
#else
        ret = pke_load_modulus_and_pre_monts_256bits(ed25519->n, ed25519->n_h);
        if(PKE_SUCCESS == ret)
        {
            ret = pke_mod_add_sub_mul_256bits_internal(tmp, k, k, MICROCODE_MODMUL);
        }
        else
        {}
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //k = (r+k*s)mod n
#if 0
        ret = pke_modadd(ed25519->n, k, r, k, Ed25519_WORD_LEN);
#else
        ret = pke_mod_add_sub_mul_256bits_internal(k, r, k, MICROCODE_MODADD);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        memcpy_(S, (uint8_t *)k, Ed25519_BYTE_LEN);
    }
    else
    {}

    return ret;
}


/* Function: Ed25519 sign
 * Parameters:
 *     mode ----------------------- input, Ed25519 signature mode
 *     prikey --------------------- input, private key, 32 bytes, little-endian
 *     pubkey --------------------- input, public key, 32 bytes, little-endian, if no pubkey, please set it to be NULL
 *     ctx ------------------------ input, 0-255 bytes
 *     ctxByteLen ----------------- input, byte length of ctx
 *     M -------------------------- input, message, requirements are determined by mode
 *     MByteLen ------------------- input, byte length of M, requirements are determined by mode
 *     RS ------------------------- output, signature
 * Return: EdDSA_SUCCESS(success); other(error)
 * Caution:
 *     1. if no public key, please set pubkey to be NULL, it will be generated inside
 *     2. if mode is not Ed25519_PH_WITH_PH_M, M could be empty(please set M to be NULL), 
 *        so no need to check M and MByteLen, otherwise, M is sha512 digest, occupies 64 bytes, 
 *        and MByteLen is not involved
 *     3. if mode is Ed25519_DEFAULT, ctx is not involved, no need to check ctx and ctxByteLen
 *     4. if mode is Ed25519_CTX, ctx can not be empty(ctx length is from 1 to 255)
 *     5. if mode is Ed25519_PH or Ed25519_PH_WITH_PH_M, ctx length is from 0 to 255, default 
 *        length is 0, thus ctx could be empty
 */
uint32_t ed25519_sign(ed25519_mode_e mode, const uint8_t prikey[32], const uint8_t pubkey[32], 
        const uint8_t *ctx, uint8_t ctxByteLen, const uint8_t *M, uint32_t MByteLen, 
        uint8_t RS[64])
{
    uint32_t ret;
    uint32_t h[16];
    const uint32_t *s = h;
    const uint8_t *prefix = (uint8_t *)(&h[Ed25519_WORD_LEN]);

    uint32_t *r = &h[Ed25519_WORD_LEN];
    uint32_t k[Ed25519_WORD_LEN<<1];
    uint32_t PH_M[Ed25519_WORD_LEN<<1];

    uint32_t msg_bytes = MByteLen;
    uint8_t ctx_bytes  = ctxByteLen;
    uint8_t phflag = 0;

    ret = ed25519_check_input(mode, prikey, ctx, &ctx_bytes, M, &msg_bytes, RS);
    if(PKE_SUCCESS == ret)
    {
        //get phflag, h=s||prefix=SHA512(prikey), PH_M=SHA512(M) for Ed25519_PH mode
        ret = ed25519_sign_internal_step_1(mode, &phflag, prikey, M, msg_bytes, 
                (uint8_t *)h, (uint8_t *)PH_M);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //get k := SHA512(dom2(F, C) || prefix || PH(M))
        ret = ed25519_sign_internal_step_2(mode, phflag, ctx, ctx_bytes, prefix, 
                M, msg_bytes, (uint8_t *)PH_M, (uint8_t *)k);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //get r := k mod n, R := [r]B
        ret = ed25519_sign_internal_step_3(k, r, RS);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //get k := SHA512(dom2(F, C) || R || A || PH(M))
        ret = ed25519_sign_internal_step_4(mode, phflag, ctx, ctx_bytes, RS, s, 
                pubkey, M, msg_bytes, (uint8_t *)PH_M, k);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //get S = (r + k * s) mod n
        ret = ed25519_sign_internal_step_5(r, k, s, PH_M, &RS[Ed25519_BYTE_LEN]);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = EdDSA_SUCCESS;
    }
    else
    {}

    return ret;
}


/* Function: Ed25519 verify step 1(internal API)
 * Parameters:
 *     mode ----------------------- input, Ed25519 signature mode
 *     phflag --------------------- output, if mode is Ed25519_PH or Ed25519_PH_WITH_PH_M, phflag is 1,
 *                                         and if mode is Ed25519_CTX, phflag is 0, otherwise phflag is useless 
 *     M -------------------------- input, message, requirements are determined by mode
 *     msg_bytes ------------------ input, byte length of M, requirements are determined by mode
 *     RS ------------------------- input, signature
 *     S -------------------------- output, big integer S, second half of the signature
 *     PH_M ----------------------- output, if mode is Ed25519_PH, it is SHA512(M), otherwise it is not involved
 * Return: PKE_SUCCESS(success); other(error)
 * Caution:
 *     1. if mode is not Ed25519_PH_WITH_PH_M, M could be empty(please set M to be NULL), 
 *        so no need to check M and msg_bytes, otherwise, M is sha512 digest, occupies 64 bytes, 
 *        and msg_bytes is not involved
 */
static uint32_t ed25519_verify_internal_step_1(ed25519_mode_e mode, uint8_t *phflag, 
        const uint8_t *M, uint32_t msg_bytes, const uint8_t *RS, uint32_t *S, 
        uint8_t *PH_M)
{
    uint32_t ret;

    //get S (S should be less than order of the base point)
    memcpy_((uint8_t *)S, &RS[Ed25519_BYTE_LEN], Ed25519_BYTE_LEN);
    if(uint32_BigNumCmp(S, Ed25519_WORD_LEN, ed25519->n, Ed25519_WORD_LEN) >= 0)
    {
        ret = EdDSA_INVALID_INPUT;
    }
    else
    {
        ret = HASH_SUCCESS;
    }

    if(HASH_SUCCESS == ret)
    {
        //set flag F
        if(Ed25519_CTX == mode)
        {
            *phflag = 0;
        }
        else if((Ed25519_PH == mode) || (Ed25519_PH_WITH_PH_M == mode))
        {
            *phflag = 1;
        }
        else
        {
            //Ed25519_DEFAULT mode, phflag is useless
        }

        //PH_M
        if(Ed25519_PH == mode)
        {
            ret = hash(HASH_SHA512, M, msg_bytes, (uint8_t *)PH_M);
        }
        else
        {}
    }
    else
    {}

    if(HASH_SUCCESS == ret)
    {
        ret = PKE_SUCCESS;
    }
    else
    {}

    return ret;
}


/* Function: Ed25519 verify step 3(internal API)
 * Parameters:
 *     pubkey --------------------- input, public key, 32 bytes, little-endian
 *     RS ------------------------- input, signature, 64 bytes, little-endian
 *     k -------------------------- input, k = SHA512(dom2(F, C) || R || A || PH(M)), 16 wrods
 *     S -------------------------- input, big integer, 8 words
 *     tmp ------------------------ input, temporary buffer, 16 words
 * Return: PKE_SUCCESS(success); other(error)
 * Caution:
 *     1. if return value is PKE_SUCCESS, the signature is valid, otherwise it is invalid.
 */
static uint32_t ed25519_verify_internal_step_3(const uint8_t *pubkey, const uint8_t *RS, 
        uint32_t *k, uint32_t *S, uint32_t *tmp)
{
    uint32_t ret;
    uint32_t pub_x[Ed25519_WORD_LEN], *pub_y = &S[0];

    //k = k mod n
    ret = ed25519_k_mod_n(k, tmp);
    if(PKE_SUCCESS == ret)
    {
        uint32_copy_8_words(k, tmp);

        //get [S]B
        ret = ed25519_pointMul_s(ed25519, S, ed25519->Gx, ed25519->Gy, tmp, &tmp[Ed25519_WORD_LEN]);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //get [k]A'
        ret = ed25519_decode_point_internal(pubkey, (uint8_t *)pub_x,(uint8_t *)pub_y);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = ed25519_pointMul_s_internal(ed25519, k, pub_x, pub_y, pub_x, pub_y);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //get R
        ret = ed25519_decode_point_internal(RS, (uint8_t *)k,(uint8_t *)(&k[Ed25519_WORD_LEN]));
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //R + [k]A'
        ret = ed25519_pointAdd_internal(ed25519, k, &k[Ed25519_WORD_LEN], 
                pub_x, pub_y, k, &k[Ed25519_WORD_LEN]);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //check whether [S]B = R + [k]A
        if((int32_t)0 != uint32_BigNumCmp(k, Ed25519_WORD_LEN, tmp, Ed25519_WORD_LEN))
        {
            ret = EdDSA_VERIFY_FAIL;
        }
        else if((int32_t)0 != uint32_BigNumCmp(&k[Ed25519_WORD_LEN], Ed25519_WORD_LEN, &tmp[Ed25519_WORD_LEN], Ed25519_WORD_LEN))
        {
            ret = EdDSA_VERIFY_FAIL;
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


/* Function: Ed25519 verify
 * Parameters:
 *     mode ----------------------- input, Ed25519 signature mode
 *     pubkey --------------------- input, public key, 32 bytes, little-endian
 *     ctx ------------------------ input, 0-255 bytes
 *     ctxByteLen ----------------- input, byte length of ctx
 *     M -------------------------- input, message, requirements are determined by mode
 *     MByteLen ------------------- input, byte length of M, requirements are determined by mode
 *     RS ------------------------- input, signature, 64 bytes, little-endian
 * Return: EdDSA_SUCCESS(success); other(error)
 * Caution:
 *     1. if mode is not Ed25519_PH_WITH_PH_M, M could be empty(please set M to be NULL), 
 *        so no need to check M and MByteLen, otherwise, M is sha512 value, occupies 64 bytes, 
 *        and MByteLen is not involved
 *     2. if mode is Ed25519_DEFAULT, ctx is not involved, no need to check ctx and ctxByteLen
 *     3. if mode is Ed25519_CTX, ctx can not be empty(ctx length is from 1 to 255)
 *     4. if mode is Ed25519_PH or Ed25519_PH_WITH_PH_M, ctx length is from 0 to 255, default 
 *        length is 0, thus ctx could be empty
 */
uint32_t ed25519_verify(ed25519_mode_e mode, const uint8_t pubkey[32], const uint8_t *ctx, 
        uint8_t ctxByteLen, const uint8_t *M, uint32_t MByteLen, const uint8_t RS[64])
{
    uint32_t ret;
    uint32_t k[Ed25519_WORD_LEN<<1];
    uint32_t S[Ed25519_WORD_LEN];
    uint32_t PH_M[Ed25519_WORD_LEN<<1];

    uint32_t msg_bytes = MByteLen;
    uint8_t ctx_bytes  = ctxByteLen;
    uint8_t phflag = 0;

    ret = ed25519_check_input(mode, pubkey, ctx, &ctx_bytes, M, &msg_bytes, RS);
    if(PKE_SUCCESS == ret)
    {
        //get and check S, get phflag, get PH_M=SHA512(M) for Ed25519_PH mode
        ret = ed25519_verify_internal_step_1(mode, &phflag, M, msg_bytes, RS, S, (uint8_t *)PH_M);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //get k := SHA512(dom2(F, C) || R || A || PH(M))
        ret = ed25519_sign_internal_step_4(mode, phflag, ctx, ctx_bytes, RS, NULL, 
                pubkey, M, msg_bytes, (uint8_t *)PH_M, k);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //check whether [S]B = R + [k]A'
        ret = ed25519_verify_internal_step_3(pubkey, RS, k, S, PH_M);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = EdDSA_SUCCESS;
    }
    else
    {}

    return ret;
}

#endif

