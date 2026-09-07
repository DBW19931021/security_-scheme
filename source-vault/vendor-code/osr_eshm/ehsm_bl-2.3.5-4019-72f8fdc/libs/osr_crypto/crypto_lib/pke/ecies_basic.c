
#include "../../crypto_include/pke_config.h"


#ifdef SUPPORT_ECIES

#include "../../crypto_include/pke/ecies.h"
#include "../../crypto_include/crypto_common/utility.h"
#include "../../crypto_include/trng/trng.h"
#include "../../crypto_include/hash_hmac/hash_kdf.h"




/*************************************************************************************
 *                       KDF function Implementation
 *************************************************************************************/

/* function: ANSI-X9.63 KDF function core, this will be used in ANSI-X963 KDF,
 *     IEEE 1363a KDF2, ISO 18033-2 kdf1, ISO 18033-2 kdf2, etc.
 * parameters:
 *     hash_alg ------------------- input, hash algorithm used in KDF.
 *     Z -------------------------- input, shared secret value, such as DH shared secret value,
 *                                         the initial key to be extended in ECIES.
 *     Z_bytes -------------------- input, byte length of Z.
 *     counter -------------------- input, initial counter value, 4 bytes, big-endian.
 *     shared_info ---------------- input, additional shared information, this is optional.
 *     shared_info_bytes ---------- input, byte length of shared_info.
 *     k1 ------------------------- output, extended key k1, actually the whole output is k1||k2.
 *     k1_bytes ------------------- input, byte length of k1.
 *     k2 ------------------------- output, extended key k2, actually the whole output is k1||k2.
 *     k2_bytes ------------------- input, byte length of k2.
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. for standard ANSI-X9.63, the initial counter is {0x00,0x00,0x00,0x01}.
 *     2. shared_info is optional, if there is no such item, please set this parameter to NULL,
 *        and set shared_info_bytes to 0.
 *     3. the whole output is k1||k2, if your output is entire, please set k1 and k1_bytes as your
 *        output parameters, and set K2 and k2_bytes to NULL and 0 respectively.
 */
FLAG_STATIC uint32_t ansi_x963_kdf_core(hash_alg_e hash_alg, const uint8_t *Z, uint32_t Z_bytes,
        uint8_t *counter, const uint8_t *shared_info,uint32_t shared_info_bytes, uint8_t *k1,
        uint32_t k1_bytes, uint8_t *k2, uint32_t k2_bytes)
{
#if 0
    hash_node_st hash_node[3] = {
        {Z, Z_bytes},
        {counter, 4u},
        {shared_info, shared_info_bytes},
    };
#else
    hash_node_st hash_node[3];

    hash_node[0].msg_addr  = Z;
    hash_node[0].msg_bytes = Z_bytes;
    hash_node[1].msg_addr  = counter;
    hash_node[1].msg_bytes = 4u;
    hash_node[2].msg_addr  = shared_info;
    hash_node[2].msg_bytes = shared_info_bytes;
#endif

    return ansi_x9_63_kdf_node(hash_alg, hash_node, 3, counter, k1, k1_bytes, k2, k2_bytes);
}


/* function: ANSI-X963 KDF function.
 *     this function refers to ANSI-X9.63-2001, or SEC1-v2-2009 section 3.6.1, or rfc8418 section-2.1
 * parameters:
 *     hash_alg ------------------- input, hash algorithm used in KDF.
 *     Z -------------------------- input, shared secret value, such as DH shared secret value,
 *                                         the initial key to be extended in ECIES.
 *     Z_bytes -------------------- input, byte length of Z.
 *     shared_info ---------------- input, additional shared information, this is optional.
 *     shared_info_bytes ---------- input, byte length of shared_info.
 *     k1 ------------------------- output, extended key k1, actually the whole output is k1||k2.
 *     k1_bytes ------------------- input, byte length of k1.
 *     k2 ------------------------- output, extended key k2, actually the whole output is k1||k2.
 *     k2_bytes ------------------- input, byte length of k2.
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. for standard ANSI-X9.63-KDF, the initial counter is {0x00,0x00,0x00,0x01}.
 *     2. shared_info is optional, if there is no such item, please set this parameter to NULL,
 *        and set shared_info_bytes to 0.
 *     3. the whole output is k1||k2, if your output is entire, please set k1 and k1_bytes as your
 *        output parameters, and set K2 and k2_bytes to NULL and 0 respectively.
 */
uint32_t ansi_x963_2001_kdf(hash_alg_e hash_alg, const uint8_t *Z, uint32_t Z_bytes,
        const uint8_t *shared_info, uint32_t shared_info_bytes, uint8_t *k1, uint32_t k1_bytes,
        uint8_t *k2, uint32_t k2_bytes)
{
    uint8_t counter[4] = {0x00,0x00,0x00,0x01};

    return ansi_x963_kdf_core(hash_alg, Z, Z_bytes, counter, shared_info, shared_info_bytes,
            k1, k1_bytes, k2, k2_bytes);
}


/* function: NIST-SP800-56A-Concatenation-KDF function
 *     this function refers to NIST.SP.800-56Ar1.pdf  chapter 5.8.1
 * parameters:
 *     hash_alg ------------------- input, hash algorithm used in KDF.
 *     Z -------------------------- input, shared secret value, such as DH shared secret value,
 *                                         the initial key to be extended in ECIES.
 *     Z_bytes -------------------- input, byte length of Z.
 *     other_info ----------------- input, additional other information.
 *     other_info_bytes ----------- input, byte length of other_info.
 *     k1 ------------------------- output, extended key k1, actually the whole output is k1||k2.
 *     k1_bytes ------------------- input, byte length of k1.
 *     k2 ------------------------- output, extended key k2, actually the whole output is k1||k2.
 *     k2_bytes ------------------- input, byte length of k2.
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. for standard NIST-SP800-56A-Concatenation-KDF, the initial counter is {0x00,0x00,0x00,0x01}.
 *     2. other_info is not optional, it consist of some items by concatenation, please see NIST-SP800-56Ar1.
 *     3. the whole output is k1||k2, if your output is entire, please set k1 and k1_bytes as your
 *        output parameters, and set K2 and k2_bytes to NULL and 0 respectively.
 */
uint32_t nist_sp800_56a_concatenation_kdf(hash_alg_e hash_alg, const uint8_t *Z, uint32_t Z_bytes,
        const uint8_t *other_info, uint32_t other_info_bytes, uint8_t *k1, uint32_t k1_bytes,
        uint8_t *k2, uint32_t k2_bytes)
{
    uint8_t counter[4] = {0x00,0x00,0x00,0x01};

    return ansi_x963_kdf_core(hash_alg, Z, Z_bytes, counter, other_info, other_info_bytes,
            k1, k1_bytes, k2, k2_bytes);
}



/*************************************************************************************
 *                       KDF structure Implementation
 *  xxx_imp is kdf structure function interface.
 *************************************************************************************/

void kdf_base_init(E_KDF_BASE *base_ctx, E_KDF_TYPE kdf_type)
{
    memset_((uint8_t *)base_ctx, 0, sizeof(E_KDF_BASE));

    base_ctx->kdf_type = kdf_type;
}

FLAG_STATIC uint32_t ansi_x963_kdf_imp(const E_KDF_BASE *self)
{
    const e_kdf_ansi_x963_2001_ctx_st *kdf_ctx = (const e_kdf_ansi_x963_2001_ctx_st *)self;
    uint32_t ret;
#if 0
    print_buf_U8(kdf_ctx->base.input,kdf_ctx->base.input_bytes, "msg");print_buf_U8(kdf_ctx->shared_info,kdf_ctx->shared_info_bytes, "msg");
#endif
    ret = ansi_x963_2001_kdf(kdf_ctx->hash_alg, kdf_ctx->base.input, kdf_ctx->base.input_bytes,
        kdf_ctx->shared_info, kdf_ctx->shared_info_bytes, kdf_ctx->base.out1,
        kdf_ctx->base.out1_bytes, kdf_ctx->base.out2, kdf_ctx->base.out2_bytes);
    if(HASH_SUCCESS == ret)
    {
#if 0
        print_buf_U8(kdf_ctx->base.out1,kdf_ctx->base.out1_bytes, "out1");print_buf_U8(kdf_ctx->base.out2,kdf_ctx->base.out2_bytes, "out2");
#endif
        ret = ECIES_SUCCESS;
    }
    else
    {}

    return ret;
}

/* function: ANSI-X963 KDF CTX init.
 * parameters:
 *     kdf_ctx -------------------- input, ctx to be initialized.
 *     shared_info ---------------- input, optional, shared secret value.
 *     shared_info_bytes ---------- input, byte length of shared_info.
 *     hash_alg ------------------- input, hash algorithm used in KDF.
 * return:
 * caution:
 *     1. after this initialization, before kdf calculation, the following fields of
 *       (E_KDF_BASE *)kdf_ctx must be set by hand.
 *       input, input_bytes, out1, out1_bytes.
 *       out2 and out2_bytes could be ignored if the whole kdf output is in out1, otherwise
 *       these two also must be initialized.
 */
void ansi_x963_2001_kdf_init(e_kdf_ansi_x963_2001_ctx_st *kdf_ctx, uint8_t *shared_info,
        uint32_t shared_info_bytes, hash_alg_e hash_alg)
{
    kdf_base_init(&kdf_ctx->base, X963_KDF);

    kdf_ctx->base.kdf_fun_imp  = ansi_x963_kdf_imp;

    kdf_ctx->shared_info       = shared_info;
    kdf_ctx->shared_info_bytes = shared_info_bytes;
    kdf_ctx->hash_alg          = hash_alg;
}



/*************************************************************************************
 *                       ENC structure Implementation
 *************************************************************************************/

FLAG_STATIC void enc_base_init(E_ENC_BASE *base_ctx, uint32_t key_bytes, uint8_t *input, uint32_t input_bytes,
        ENC_TYPE type)
{
    memset_((uint8_t *)base_ctx, 0, sizeof(E_ENC_BASE));

    //no base_ctx->key, since enc key depends on ENC_TYPE
    //no base_ctx->output, since this depends on 1st part of the whole output(when encrypting)
    //no base_ctx->output_bytes, since this is encryption or decryption output
    base_ctx->key_bytes    = key_bytes;
    base_ctx->input        = input;
    base_ctx->input_bytes  = input_bytes;
    base_ctx->enc_type     = type;
}


FLAG_STATIC uint32_t xor_enc_imp(E_ENC_BASE *self)
{
#if 0
    E_XOR_ENC_CTX *ctx = (E_XOR_ENC_CTX *)self;
#endif

    uint8_XOR(self->key, self->input, self->output, self->key_bytes);
    self->output_bytes = self->key_bytes;

    return ECIES_SUCCESS;
}


/* function: XOR Encryption ENC CTX init.
 * parameters:
 *     enc_ctx -------------------- input, ctx to be initialized
 *     input ---------------------- input, internal plaintext or ciphertext
 *     input_bytes ---------------- input, byte length of key and input
 * return:
 * caution:
 *     1. in XOR encryption, the encryption key length is equal to the message length
 *     2. after this initialization, before encryption or decryption, the following fields of
 *       (E_ENC_BASE *)enc_ctx must be set by hand.
 *       key, output
 */
void e_xor_enc_init(E_XOR_ENC_CTX *enc_ctx, uint8_t *input, uint32_t input_bytes)
{
    enc_base_init(&enc_ctx->base, input_bytes, input, input_bytes, XOR_ENC);
    enc_ctx->base.enc_fun_imp = xor_enc_imp;
    enc_ctx->base.dec_fun_imp = xor_enc_imp;
}



/*************************************************************************************
 *                       MAC structure Implementation
 *************************************************************************************/

FLAG_STATIC void mac_base_init(E_MAC_BASE *base_ctx, uint8_t *key, uint32_t key_bytes)
{
    memset_((uint8_t *)base_ctx, 0, sizeof(E_MAC_BASE));

    base_ctx->key         = key;
    base_ctx->key_bytes   = key_bytes;
}

FLAG_STATIC uint32_t hmac_imp(const E_MAC_BASE *self)
{
    const e_hmac_ctx_st *ctx;
    hash_node_st node[2];
    uint32_t ret;

    ctx = (const e_hmac_ctx_st *)self;

    node[0].msg_addr  = ctx->base.msg;
    node[0].msg_bytes = ctx->base.msg_bytes;
    node[1].msg_addr  = ctx->base.appendix;
    node[1].msg_bytes = ctx->base.appendix_bytes;
#if 0
    print_buf_U8(node[0].msg_addr, node[0].msg_bytes, "msg");print_buf_U8(node[1].msg_addr, node[1].msg_bytes, "msg");print_buf_U8(ctx->base.key, ctx->base.key_bytes, "key");
#endif
    ret = hmac_node_steps(ctx->hash_alg, ctx->base.key, 0, ctx->base.key_bytes, node, 2, ctx->base.mac);
    if(HASH_SUCCESS == ret)
    {
        ret = ECIES_SUCCESS;
    }
    else
    {}
#if 0
    print_buf_U8(ctx->base.mac, ctx->base.mac_bytes, "mac");
#endif
    return ret;
}

/* function: HMAC MAC CTX init.
 * parameters:
 *     mac_ctx -------------------- input, ctx to be initialized
 *     key_buffer ----------------- input, hmac key buffer, to store hmac key
 *     key_bytes ------------------ input, hmac key byte length
 *     appendix ------------------- input, appendix followed by cipher
 *     appendix_bytes ------------- input, byte length of appendix
 *     hash_alg ------------------- input, hash algorithm used in hmac
 * return:
 * caution:
 *     1. after this initialization, before mac calculation, the following fields of
 *       (E_MAC_BASE *)mac_ctx must be set by hand.
 *       msg, msg_bytes, mac
 */
void e_hmac_init(e_hmac_ctx_st *mac_ctx, uint8_t *key_buffer, uint32_t key_bytes, uint8_t *appendix,
        uint32_t appendix_bytes, hash_alg_e hash_alg)
{
    mac_base_init(&mac_ctx->base, key_buffer, key_bytes);

    //no base_ctx->msg and base_ctx->msg_bytes, since this is the cipher
    //no mac, since this depends on the encrypting.
    mac_ctx->base.mac_bytes      = ((uint32_t)hash_get_digest_word_len(hash_alg)) << 2;
    mac_ctx->base.appendix       = appendix;
    mac_ctx->base.appendix_bytes = appendix_bytes;
    mac_ctx->base.mac_imp        = hmac_imp;

    mac_ctx->hash_alg            = hash_alg;
}


/*************************************************************************************
  *                       ECIES function Implementation
  *************************************************************************************/

#ifdef ECIES_SUPPORT_EC_POINT_COMPRESSED
/* function: private function.
 *           lucas sequences:
 *           U_0 = 0, U_1 = 1, and U_k = P * U_{k - 1} - Q * U_{k - 2} for k >= 2
 *           V_0 = 2, V_1 = P, and V_k = P * V_{k - 1} - Q * V_{k - 2} for k >= 2
 *           ref: ANSI-X963-2001 D1.3 Generating Lucas Sequences
 * parameters:
 *     p -------------------------- input, modulus, a odd prime.
 *     P -------------------------- input, initial value of lucas sequence parameter P (P is upper case !).
 *     Q -------------------------- input, initial value of lucas sequence parameter Q.
 *     k -------------------------- input, the subscript value of a lucas sequence.
 *     pBitLen -------------------- bit length of p.
 *     u -------------------------- output, U_k mod p.
 *     v -------------------------- output, V_k mod p.
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. k can not be zero
 */
uint32_t lucas_sequences(const uint32_t *p, uint32_t *P, uint32_t *Q, uint32_t *k, uint32_t pBitLen,
        uint32_t *u, uint32_t *v)
{
    uint32_t delta[ECCP_MAX_WORD_LEN];
    uint32_t inv_2[ECCP_MAX_WORD_LEN];  //2^(-1) mod p
    uint32_t tmp1[ECCP_MAX_WORD_LEN];
    uint32_t tmp2[ECCP_MAX_WORD_LEN];
    uint32_t tmp3[ECCP_MAX_WORD_LEN];
    uint32_t pWordLen = GET_WORD_LEN(pBitLen);
    uint32_t i, ret;

    //get inv_2 = 2 ^ (-1) mod p
    inv_2[0] = 2;
    ret = pke_modinv(p, inv_2, inv_2, pWordLen, 1);
    if(PKE_SUCCESS != ret)
    {
        goto END;
    }
    else
    {}

    /*********** get delta = (P ^ 2 - 4 * Q) mod p ***********/
    // delta = (P ^ 2) mod p
    ret = pke_modmul(p, P, P, delta, pWordLen);
    if(PKE_SUCCESS != ret)
    {
        goto END;
    }
    else
    {}

    // tmp1 = (4 * Q) mod p
    uint32_clear(tmp1, pWordLen);
    tmp1[0] = 4;
    ret = pke_modmul_internal(tmp1, Q, tmp1, pWordLen);
    if(PKE_SUCCESS != ret)
    {
        goto END;
    }
    else
    {}

    // delta = (P ^ 2) - (4 * Q) mod p
    ret = pke_modsub(p, delta, tmp1, delta, pWordLen);
    if(PKE_SUCCESS != ret)
    {
        goto END;
    }
    else
    {}

    /*********** travesal k binary ***********/
    //set u1 and v1
    uint32_clear(u, pWordLen);     //u = 1
    u[0] = 1;
    uint32_copy(v, P, pWordLen);   //v = P

    i = get_valid_bits((const uint32_t *)k, pWordLen);
    if(0 == i)
    {
        //set u0 and v0
        u[0] = 0;                  //u = 0
        uint32_clear(v, pWordLen); //v = 2
        v[0] = 2;

        ret = PKE_SUCCESS;
        goto END;
    }
    else
    {}

    i--;
    while(0U != (i--))
    {
        /*********** (u,v)=(uv mod p, (v^2 + delta*u^2)/2 mod p) ***********/
        //tmp3 = (u * v) mod p ------ (u)
        ret = pke_modmul_internal(u, v, tmp3, pWordLen);
        if(PKE_SUCCESS != ret)
        {
            goto END;
        }
        else
        {}

        // tmp1 = (v ^ 2) mod p
        ret = pke_modmul_internal(v, v, tmp1, pWordLen);
        if(PKE_SUCCESS != ret)
        {
            goto END;
        }
        else
        {}

        //tmp2 = u ^ 2 mod p
        ret = pke_modmul_internal(u, u, tmp2, pWordLen);
        if(PKE_SUCCESS != ret)
        {
            goto END;
        }
        else
        {}

        //tmp2 = delta * (u ^ 2) mod p
        ret = pke_modmul_internal(delta, tmp2, tmp2, pWordLen);
        if(PKE_SUCCESS != ret)
        {
            goto END;
        }
        else
        {}

        //tmp2 = (v ^ 2) + delta * (u ^ 2) mod p
        ret = pke_modadd(p, tmp1, tmp2, tmp2, pWordLen);
        if(PKE_SUCCESS != ret)
        {
            goto END;
        }
        else
        {}

        //v = tmp2/2 mod p
        ret = pke_modmul_internal(tmp2, inv_2, v, pWordLen);
        if(PKE_SUCCESS != ret)
        {
            goto END;
        }
        else
        {}
        uint32_copy(u, tmp3, pWordLen);

        if(get_bit_value_by_index((const uint32_t *)k, i))
        {
            /*********** (u, v) = ((Pu + v)/2 mod p, (Pv + delta * u)/2 mod p) ***********/
            //tmp1 = P * u mod p
            ret = pke_modmul_internal(P, u, tmp1, pWordLen);
            if(PKE_SUCCESS != ret)
            {
                goto END;
            }
            else
            {}

            //tmp2 = (P * u + v) mod p
            ret = pke_modadd(p, tmp1, v, tmp2, pWordLen);
            if(PKE_SUCCESS != ret)
            {
                goto END;
            }
            else
            {}

            //tmp3 = (P * u + v)/2 mod p ---- (u)
            ret = pke_modmul_internal(tmp2, inv_2, tmp3, pWordLen);
            if(PKE_SUCCESS != ret)
            {
                goto END;
            }
            else
            {}

            //v = (P * v + delta * u) / 2 mod p
            // tmp2 = (P * v) mod p
            ret = pke_modmul_internal(P, v, tmp2, pWordLen);
            if(PKE_SUCCESS != ret)
            {
                goto END;
            }
            else
            {}

            //tmp1 = (delta * u) mod p
            ret = pke_modmul_internal(delta, u, tmp1, pWordLen);
            if(PKE_SUCCESS != ret)
            {
                goto END;
            }
            else
            {}

            //tmp2 = (P * v + delta * u) mod p
            ret = pke_modadd(p, tmp2, tmp1, tmp2, pWordLen);
            if(PKE_SUCCESS != ret)
            {
                goto END;
            }
            else
            {}

            //v = (P * v + delta * u)/2 mod p
            ret = pke_modmul_internal(tmp2, inv_2, v, pWordLen);
            if(PKE_SUCCESS != ret)
            {
                goto END;
            }
            else
            {}
            uint32_copy(u, tmp3, pWordLen);
        }
        else
        {}
    }

    ret = PKE_SUCCESS;

END:

    uint32_clear(delta, ECCP_MAX_WORD_LEN);
    uint32_clear(inv_2, ECCP_MAX_WORD_LEN);
    uint32_clear(tmp1, ECCP_MAX_WORD_LEN);
    uint32_clear(tmp2, ECCP_MAX_WORD_LEN);
    uint32_clear(tmp3, ECCP_MAX_WORD_LEN);

    return ret;
}


/* function: private function.
 *           Quadratic residue:
 *           x ^ 2 = a mod p  -----> x = ?
 *           ref: ANSI-X963-2001 D1.4 Finding Square Roots Modulo a Prime
 * parameters:
 *     p ------------------------ input, modulus, a odd prime.
 *     a ------------------------ input, integer a with 0 < a < p.
 *     pBitLen ------------------ input, bit length of p.
 *     x ------------------------ output, a square root (mod p) of a if one exists.
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *    1. p must be a prime.
 */
uint32_t quadratic_residue(const uint32_t *p, uint32_t *a, uint32_t pBitLen, uint32_t *x)
{
    uint32_t tmp[ECCP_MAX_WORD_LEN];
    uint32_t u[ECCP_MAX_WORD_LEN];
    uint32_t gama[ECCP_MAX_WORD_LEN];
    uint32_t i[ECCP_MAX_WORD_LEN];
    uint32_t flex_num[ECCP_MAX_WORD_LEN];
    uint32_t pWordLen = GET_WORD_LEN(pBitLen);
    uint32_t ret;

    uint32_clear(flex_num, ECCP_MAX_WORD_LEN);

    //set mod p
#if (defined(PKE_LP) || defined(PKE_SECURE))
    ret = pke_pre_calc_mont(p, pBitLen, NULL, NULL);
#else
    ret = pke_pre_calc_mont(p, pBitLen, NULL);
#endif
    if(PKE_SUCCESS != ret)
    {
        goto END;
    }
    else
    {}

    // p = 3 mod 4 ?
    if((p[0] & 0x00000003U) == 0x00000003U)
    {
        // 4 * u + 3 = p ---- u = ?
        // u = p - 3
        uint32_copy(u, p, pWordLen);

        // u = u / 4
        (void)Big_Div2n(u, pWordLen, 2);

        //tmp = u + 1
        flex_num[0] = 1;
        ret = pke_add(u, flex_num, tmp, pWordLen);
        if(PKE_SUCCESS != ret)
        {
            goto END;
        }
        else
        {}

        //u = a ^ (u+1) mod p
        ret = pke_modexp_internal(tmp, a, u, pWordLen, pWordLen);
        if(PKE_SUCCESS != ret)
        {
            goto END;
        }
        else
        {}

        // tmp = (u ^ 2) mod p
#if (defined(PKE_LP) || defined(PKE_SECURE))
        pke_set_exe_cfg(PKE_EXE_CFG_ALL_NON_MONT);
#endif
        ret = pke_modmul_internal(u, u, tmp, pWordLen);
        if(PKE_SUCCESS != ret)
        {
            goto END;
        }
        else
        {}

        if(0 == uint32_BigNumCmp(tmp, pWordLen, a, pWordLen))
        {
            uint32_copy(x, u, pWordLen);
            ret = PKE_SUCCESS;
        }
        else
        {
            ret = PKE_ERROR;
        }
    }
    else if((p[0] & 0x00000007U) == 0x00000005U)// p = 5 mod 8 ?
    {
        // p = 8 * u + 5 ---- u = ?
        // u = p - 5
        uint32_copy(u, p, pWordLen);

        // u = u / 8
        (void)Big_Div2n(u, pWordLen, 3);

        // tmp = (2 * a) mod p
        ret = pke_modadd(p, a, a, tmp, pWordLen);
        if(PKE_SUCCESS != ret)
        {
            goto END;
        }
        else
        {}

        // gama = (tmp ^ u) mod p
        ret = pke_modexp_internal(u, tmp, gama, pWordLen, pWordLen);
        if(PKE_SUCCESS != ret)
        {
            goto END;
        }
        else
        {}

        /*********** i = 2 * a * (gama ^ 2) mod p ***********/
        // u = gama ^ 2 mod p
#if (defined(PKE_LP) || defined(PKE_SECURE))
        pke_set_exe_cfg(PKE_EXE_CFG_ALL_NON_MONT);
#endif
        ret = pke_modmul_internal(gama, gama, u, pWordLen);
        if(PKE_SUCCESS != ret)
        {
            goto END;
        }
        else
        {}

        // i = tmp * u mod p
        ret = pke_modmul_internal(tmp, u, i, pWordLen);
        if(PKE_SUCCESS != ret)
        {
            goto END;
        }
        else
        {}

        /*********** x = a * gama * (i - 1) mod p ***********/
        // get tmp = i-1
        flex_num[0] = 1;
        ret = pke_sub(i, flex_num, tmp, pWordLen);
        if(PKE_SUCCESS != ret)
        {
            goto END;
        }
        else
        {}

        // u = gama * (i - 1) mod p
        ret = pke_modmul_internal(gama, tmp, u, pWordLen);
        if(PKE_SUCCESS != ret)
        {
            goto END;
        }
        else
        {}

        // x = a * u mod p
        ret = pke_modmul_internal(a, u, x, pWordLen);
        if(PKE_SUCCESS != ret)
        {
            goto END;
        }
        else
        {}

        // tmp = x ^ 2 mod p
        ret = pke_modmul_internal(x, x, tmp, pWordLen);
        if(PKE_SUCCESS != ret)
        {
            goto END;
        }
        else
        {}

        if(0 == uint32_BigNumCmp(tmp, pWordLen, a, pWordLen))
        {
            ret = PKE_SUCCESS;
        }
        else
        {
            ret = PKE_ERROR;
        }
    }
    else if((p[0] & 0x00000003U) == 0x00000001U)// p = 1 mod 4
    {
        //set i = 2 * u + 1, here p = 4 * u + 1
        uint32_copy(i, p, pWordLen);
        (void)Big_Div2n(i, pWordLen, 1);
        i[0] |= 1;

        //set x = 4 * a mod p
#if (defined(PKE_LP) || defined(PKE_SECURE))
        pke_set_exe_cfg(PKE_EXE_CFG_ALL_NON_MONT);
#endif
        flex_num[0] = 4;
        ret = pke_modmul_internal(flex_num, a, x, pWordLen);
        if(PKE_SUCCESS != ret)
        {
            goto END;
        }
        else
        {}

        //flex_num = 2^(-1) mod p
        flex_num[0] = 2;
        ret = pke_modinv(p, flex_num, flex_num, pWordLen, 1);
        if(PKE_SUCCESS != ret)
        {
            goto END;
        }
        else
        {}

        while(1)
        {
            //generate random num in [0, p - 1]
            ret = get_rand((uint8_t *)tmp, GET_BYTE_LEN(pBitLen));
            if(TRNG_SUCCESS != ret)
            {
                goto END;
            }
            else
            {}

            if(tmp[pWordLen - 1] >= p[pWordLen - 1])
            {
               tmp[pWordLen - 1] = p[pWordLen - 1] - 1;
            }
            else
            {}

            // u --> U gama--> V
            ret = lucas_sequences(p, tmp, a, i, pBitLen, u, gama);
            if(PKE_SUCCESS != ret)
            {
                goto END;
            }
            else
            {}

            // tmp = gama ^ 2 mod p
            ret = pke_modmul_internal(gama, gama, tmp, pWordLen);
            if(PKE_SUCCESS != ret)
            {
                goto END;
            }
            else
            {}

            // gama ^ 2 = 4 * a mod p ?
            if(0 == uint32_BigNumCmp(tmp, pWordLen, x, pWordLen))
            {
                //x = V/2 mod p
                ret = pke_modmul_internal(gama, flex_num, x, pWordLen);
                if(PKE_SUCCESS != ret)
                {
                    goto END;
                }
                else
                {}

                ret = PKE_SUCCESS;
                break;
            }
            else if((Bigint_Check_1(u, pWordLen) != 1u) && (Bigint_Check_p_1(u, p, pWordLen) != 1u))
            {
                ret = PKE_ERROR;
                break;
            }
            else
            {
                //nothing to do, just for static analysis.
            }
        }
    }
    else
    {
        ret = PKE_ERROR;
    }

END:

    uint32_clear(flex_num, ECCP_MAX_WORD_LEN);
    uint32_clear(tmp, ECCP_MAX_WORD_LEN);
    uint32_clear(u, ECCP_MAX_WORD_LEN);
    uint32_clear(gama, ECCP_MAX_WORD_LEN);
    uint32_clear(i, ECCP_MAX_WORD_LEN);

    return ret;
}
#endif

/* function: An elliptic curve point P = (x , y) that is not the point at infinity shall
 *     be represented as an octet string in one of the following two forms:
 *          1. uncompressed form.
 *          2. compressed form.
 * parameters:
 *     curve ---------------------- input, ecc curve struct pointer, please make sure it is valid.
 *     x -------------------------- input, curve point x, big-endian.
 *     y -------------------------- input, curve point y, big-endian.
 *     point_form ----------------- input, curve point format.
 *     result --------------------- output, curve point representation.
 *     r_bytes -------------------- output, byte length of result
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *    1. the result space needs 1+2*pByteLen bytes while point_form is POINT_UNCOMPRESSED,
 *       or 1+pByteLen bytes while point_form is POINT_COMPRESSED, here pByteLen is byte
 *       length of curve parameter p
 */
FLAG_STATIC uint32_t point_to_octet_string_conversion(const eccp_curve_st *curve, const uint8_t *x, const uint8_t *y,
        EC_POINT_FORM point_form, uint8_t *result, uint32_t *r_bytes)
{
    uint32_t pByteLen;
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS   //just for static analysis.
    if((NULL == curve) || (NULL == r_bytes))
    {
        ret = ECIES_POINTER_NULL;
    }
    else
    {
#endif
        pByteLen = GET_BYTE_LEN(curve->eccp_p_bitLen);
        memcpy_(&result[1], x, pByteLen);

        switch(point_form)
        {
#ifdef ECIES_SUPPORT_EC_POINT_COMPRESSED
        case POINT_COMPRESSED:
            result[0] = POINT_COMPRESSED | (y[pByteLen-1] & 0x00000001);
            *r_bytes = 1u + pByteLen;
            ret = PKE_SUCCESS;
            break;
#endif

        case POINT_UNCOMPRESSED:
            result[0] = POINT_UNCOMPRESSED;
            memcpy_(&result[1u+pByteLen], y, pByteLen);
            *r_bytes = 1u + (pByteLen<<1);
            ret = PKE_SUCCESS;
            break;

        default:
            ret = PKE_ERROR;
            break;
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: The representation value of the curve point (x, y) obtained through the
 *     point_to_octet_string_conversion function is inversely released from the
 *     x_p coordinate and y_p coordinate
 * parameters:
 *     curve ---------------------- input, ecc curve struct pointer, please make sure it is valid.
 *     encode --------------------- input, curve point representation.
 *     x -------------------------- output, x coordinate of curve point, U32 little-endian.
 *     y -------------------------- output, y coordinate of curve point, U32 little-endian.
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 */
FLAG_STATIC uint32_t octet_string_to_point_conversion(const eccp_curve_st *curve, const uint8_t* encode, uint32_t *x, uint32_t *y)
 {
    uint32_t z[ECCP_MAX_WORD_LEN];
    uint32_t pWordLen = GET_WORD_LEN(curve->eccp_p_bitLen);
    uint32_t pByteLen = GET_BYTE_LEN(curve->eccp_p_bitLen);
    uint32_t ret = PKE_ERROR;

    //little endian -> big endian
    reverse_byte_array(&encode[1], (uint8_t *)x, pByteLen);

#ifdef ECIES_SUPPORT_EC_POINT_COMPRESSED
    if((POINT_COMPRESSED == encode[0]) || ((POINT_COMPRESSED+1) == encode[0]))
    {
        // z = x^3 + ax + b = x(x^2+a) + b mod p
        ret = pke_modmul(curve->eccp_p, x, x, z, pWordLen);
        if(PKE_SUCCESS == ret)
        {
            ret = pke_modadd(curve->eccp_p, curve->eccp_a, z, z, pWordLen);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            ret = pke_modmul_internal(x, z, z, pWordLen);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            ret = pke_modadd(curve->eccp_p, curve->eccp_b, z, z, pWordLen);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            ret = quadratic_residue(curve->eccp_p, z, curve->eccp_p_bitLen, y);
        }
        else
        {}
        
        if(PKE_SUCCESS == ret)
        {
            if((y[0] & 0x00000001) != (encode[0] - POINT_COMPRESSED))
            {
                // y = p - y
                ret = pke_sub(curve->eccp_p, y, y, pWordLen);
            }
            else
            {}
        }
        else
        {}
    }
    else
    {}
#else
    if(POINT_UNCOMPRESSED == encode[0])
    {
        reverse_byte_array(&encode[1u + pByteLen], (uint8_t *)y, pByteLen);
        ret = PKE_SUCCESS;
    }
    else
    {}
#endif

    uint32_clear(z, pWordLen);
    if(PKE_SUCCESS != ret)
    {
        ret = PKE_ERROR;
    }
    else
    {}

    return ret;
}


 /* function: get the 1st part(a point) byte length in ECIES ciphertext.
  * parameters:
  *     curve --------------------- input, ecc curve struct pointer, please make sure it is valid.
  *     cipher -------------------- input, ECIES ciphertext.
  *     cipher_bytes -------------- input, byre length of cipher.
  *     point_bytes --------------- output, the 1st part(a point) byte length in cipher.
  * return: PKE_SUCCESS(success), other(error)
  * caution:
  *     1.
  */
FLAG_STATIC uint32_t ansi_x963_get_point_byte_len_from_ciphertext(const eccp_curve_st *curve, const uint8_t *cipher,
        uint32_t *point_bytes)
 {
    uint32_t pByteLen = GET_BYTE_LEN(curve->eccp_p_bitLen);
    uint32_t ret;

    switch(cipher[0])
    {
#ifdef ECIES_SUPPORT_EC_POINT_COMPRESSED
    case POINT_COMPRESSED:
    case (POINT_COMPRESSED+1):
        *point_bytes = pByteLen + 1u;
        ret = PKE_SUCCESS;
        break;
#endif

    case POINT_UNCOMPRESSED:
        *point_bytes = (pByteLen<<1) + 1u;
        ret = PKE_SUCCESS;
        break;

    default:
        ret = PKE_ERROR;
        break;
    }

    return ret;
}



/*************************************************************************************
 *                       ECIES structure Implementation
 *************************************************************************************/

void ecies_ansi_x963_ctx_init(ECIES_STD *ctx, ecies_enc_mac_key_order_e enc_mac_key_order)
{
    ctx->type_flag = ANSI_X963;
    ctx->enc_mac_key_order = enc_mac_key_order;
    ctx->get_point_len_from_ciphertext = ansi_x963_get_point_byte_len_from_ciphertext;
    ctx->point_decompress = octet_string_to_point_conversion;
    ctx->point_compress = point_to_octet_string_conversion;
}

#endif

