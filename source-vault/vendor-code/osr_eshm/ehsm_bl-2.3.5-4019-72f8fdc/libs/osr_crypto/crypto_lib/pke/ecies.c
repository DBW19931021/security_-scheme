
#include "../../crypto_include/pke_config.h"


#ifdef SUPPORT_ECIES

#include "../../crypto_include/pke/ecies.h"
#include "../../crypto_include/crypto_common/utility.h"
#include "../../crypto_include/trng/trng.h"



#if 0
#define DEBUG_ECIES
#endif


FLAG_STATIC uint32_t ecies_encrypt_C2_C3(const eccp_curve_st *curve,const ECIES_STD *ecies_ctx, E_KDF_BASE *kdf_ctx, E_MAC_BASE *mac_ctx, 
    E_ENC_BASE *enc_ctx, uint8_t *Qx, uint8_t *cipher_C2_C3)
{
#if 0
    uint8_t enc_key[ECIES_BLOCK_ENC_K_E_Y_MAX_BYTE_LEN];
#endif
    uint32_t ret;

    //set enc key and output buffer, this must be done before KDF action.
    enc_ctx->output = cipher_C2_C3;
    if(XOR_ENC == enc_ctx->enc_type)
    {
        enc_ctx->key = enc_ctx->output;
    }
    else
    {
#if 0
        //TODO
        enc_ctx->key = enc_key;
#endif
    }

    //3. get k_enc and k_mac from KDF.
    kdf_ctx->input       =  (uint8_t *)Qx;
    kdf_ctx->input_bytes = GET_BYTE_LEN(curve->eccp_p_bitLen);//pByteLen;
    if(ENC_MAC_ORDER == ecies_ctx->enc_mac_key_order)
    {
        kdf_ctx->out1       = enc_ctx->key;
        kdf_ctx->out1_bytes = enc_ctx->key_bytes;
        kdf_ctx->out2       = mac_ctx->key;
        kdf_ctx->out2_bytes = mac_ctx->key_bytes;
    }
    else
    {
        kdf_ctx->out1       = mac_ctx->key;
        kdf_ctx->out1_bytes = mac_ctx->key_bytes;
        kdf_ctx->out2       = enc_ctx->key;
        kdf_ctx->out2_bytes = enc_ctx->key_bytes;
    }

    ret = kdf_ctx->kdf_fun_imp(kdf_ctx);

#ifdef DEBUG_ECIES
        print_buf_U8(kdf_ctx->input, pByteLen, "kdf - input");
#endif

    if(ret == ECIES_SUCCESS)
    {
        ret = enc_ctx->enc_fun_imp(enc_ctx);
    }
    else
    {}

#ifdef DEBUG_ECIES
    print_buf_U8(kdf_ctx->out1,kdf_ctx->out1_bytes, "kdf-enc");
    print_buf_U8(kdf_ctx->out2,kdf_ctx->out2_bytes, "kdf-mac");
#endif

    //4. c = Enc(k_enc, msg)
#ifdef DEBUG_ECIES
    print_buf_U8( kdf_ctx->out,enc_ctx->key_len, "enc-key");
#endif

#ifdef DEBUG_ECIES
    print_buf_U8(enc_ctx->output,enc_ctx->output_bytes, "cipher");
    print_buf_U8(out,r_len, "out after enc");
#endif
    if(ret == ECIES_SUCCESS)
    {
        //5. get d = mac(k_mac, c)
        //set mac msg, msg_bytes and mac buffer, this must be done before MAC action.
        mac_ctx->msg       = enc_ctx->output;
        mac_ctx->msg_bytes = enc_ctx->output_bytes;
        mac_ctx->mac       = &cipher_C2_C3[enc_ctx->output_bytes];

        ret = mac_ctx->mac_imp(mac_ctx);
    }
    else
    {}

#ifdef DEBUG_ECIES
    print_buf_U8(mac_ctx->key,mac_ctx->key_bytes, "mac_key");
    print_buf_U8(mac_ctx->msg,mac_ctx->msg_bytes, "mac input msg");
    print_buf_U8(mac_ctx->appendix,mac_ctx->appendix_bytes, "mac input appendix");
#endif

#ifdef DEBUG_ECIES
    print_buf_U8(mac_ctx->mac, mac_ctx->mac_bytes, "mac value");
    print_buf_U8(out,r_len, "out after mac");
#endif

    return ret;
}


/* function:  Elliptic Curve Integrated Encryption Scheme (ECIES) core interface
 * parameters:
 *     ecies_ctx ------------------ input, types of ecies structure
 *     curve ---------------------- input, ecc curve struct pointer, please make sure it is valid
 *     msg ------------------------ input, original message, plaintext.
 *     msg_bytes ------------------ input, byte length of msg.
 *     sender_tmp_pri_key --------- input, sender's ephemeral private key, big-endian.
 *     receiver_pub_key ----------- input, reveiver's public key, big-endian.
 *     conversion_form ------------ input, curve point representation.
 *     kdf_ctx -------------------- input, key derivation function structure.
 *     mac_ctx -------------------- input, message authentication code structure.
 *     enc_ctx -------------------- input, symmetric encryption scheme structure.
 *     cipher --------------------- output, encryption result, ciphertext.
 *     cipher_bytes --------------- output, byte length of cipher.
 * return:
 *     ECIES_SUCCESS(success); other(error)
 * caution:
 *     1. the result ciphertext consists of three parts. the 1st part is a point, the 2nd part is
 *        internal ciphertext, the 3rd part is mac.
 */
FLAG_STATIC uint32_t ecies_encrypt(const ECIES_STD *ecies_ctx, const eccp_curve_st *curve,
        const uint8_t *sender_tmp_pri_key, const uint8_t *receiver_pub_key, EC_POINT_FORM point_form,
        E_KDF_BASE *kdf_ctx, E_MAC_BASE *mac_ctx, E_ENC_BASE *enc_ctx, uint8_t *cipher,
        uint32_t *cipher_bytes)
{
    uint32_t k[ECCP_MAX_WORD_LEN];
    uint32_t tmp[ECCP_MAX_WORD_LEN<<1];
    uint32_t pWordLen;
    uint32_t pByteLen;
    uint32_t nByteLen;
    uint32_t nWordLen;
    uint32_t r_bytes;
    uint32_t ret = ECIES_SUCCESS;

    pWordLen = GET_WORD_LEN(curve->eccp_p_bitLen);
    pByteLen = GET_BYTE_LEN(curve->eccp_p_bitLen);
    nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);
    nByteLen = GET_BYTE_LEN(curve->eccp_n_bitLen);

    //1. get r, 1st part of out
    k[nWordLen - 1u] = 0u;
    if(NULL != sender_tmp_pri_key)
    {
        //transfer to U32 big number.
        reverse_byte_array(sender_tmp_pri_key, (uint8_t *)k, nByteLen);

        //make sure k in [1, n-1]
        ret = uint32_integer_check(k, curve->eccp_n, nWordLen, ECIES_ZERO_ALL, ECIES_INTEGER_TOO_BIG,
                ECIES_SUCCESS);
        if(ECIES_SUCCESS == ret)
        {
            ret = eccp_get_pubkey_from_prikey(curve, sender_tmp_pri_key, (uint8_t *)tmp);
        }
        else
        {}
    }
    else
    {
        ret = eccp_getkey(curve, (uint8_t *)k, (uint8_t *)tmp);
        if(PKE_SUCCESS == ret)
        {
            //transfer to U32 big number.
            reverse_byte_array((uint8_t *)k, (uint8_t *)k, nByteLen);
        }
        else
        {}
    }

    if(PKE_SUCCESS == ret)
    {
        ret = ecies_ctx->point_compress(curve, (uint8_t *)tmp, &(((uint8_t *)tmp)[pByteLen]), point_form, 
                cipher, &r_bytes);
    }
    else
    {}

#ifdef DEBUG_ECIES
    print_buf_U8(out,r_len, "point-compress-big-endian");
#endif
    if(PKE_SUCCESS == ret)
    {
        //2. get ([k]pub_key).x
        tmp[pWordLen - 1u] = 0u;
        tmp[pWordLen + pWordLen - 1u] = 0u;
        reverse_byte_array(receiver_pub_key, (uint8_t *)tmp, pByteLen);
        reverse_byte_array(&receiver_pub_key[pByteLen], (uint8_t *)(&tmp[pWordLen]), pByteLen);

        ret = eccp_check_point(curve, tmp, &tmp[pWordLen]);
    }

    if(PKE_SUCCESS == ret)
    {
        ret = eccp_pointMul(curve, k, tmp, &tmp[pWordLen], k, NULL);
        reverse_byte_array((uint8_t *)k, (uint8_t *)k, pByteLen);
    }
    else
    {}

#ifdef DEBUG_ECIES
    print_buf_U8((uint8_t *)k, pByteLen,  "kP.x");
#endif

    ret = ecies_encrypt_C2_C3(curve, ecies_ctx, kdf_ctx, mac_ctx, enc_ctx, (uint8_t *)k, &cipher[r_bytes]);
    if(ret == ECIES_SUCCESS)
    {
        //6. out = r || c || d
        *cipher_bytes = r_bytes + enc_ctx->output_bytes + mac_ctx->mac_bytes;
    }
    else
    {}

    return ret;
}


FLAG_STATIC uint32_t ecies_decrypt_C2_C3(const eccp_curve_st *curve,const ECIES_STD *ecies_ctx, E_KDF_BASE *kdf_ctx, const E_MAC_BASE *mac_ctx, 
    E_ENC_BASE *dec_ctx, uint8_t *Qx, const uint8_t *cipher_C3)
{
    uint32_t ret;

    //2. get k_enc and k_mac from KDF.
    kdf_ctx->input       =  (uint8_t *)Qx;
    kdf_ctx->input_bytes = GET_BYTE_LEN(curve->eccp_p_bitLen);

    if(ENC_MAC_ORDER == ecies_ctx->enc_mac_key_order)
    {
        kdf_ctx->out1       = dec_ctx->key;
        kdf_ctx->out1_bytes = dec_ctx->key_bytes;
        kdf_ctx->out2       = mac_ctx->key;
        kdf_ctx->out2_bytes = mac_ctx->key_bytes;
    }
    else
    {
        kdf_ctx->out1       = mac_ctx->key;
        kdf_ctx->out1_bytes = mac_ctx->key_bytes;
        kdf_ctx->out2       = dec_ctx->key;
        kdf_ctx->out2_bytes = dec_ctx->key_bytes;
    }

    ret = kdf_ctx->kdf_fun_imp(kdf_ctx);

#ifdef DEBUG_ECIES
    printf("kdf-outlen: %d\n\r", kdf_ctx->out_bytes);
    print_buf_U8(kdf_ctx->out,kdf_ctx->out_bytes, "kdf-dec");
#endif

    if(ret == ECIES_SUCCESS)
    {
        //3. d ?= mac(k_mac, c)
        ret = mac_ctx->mac_imp(mac_ctx);
    }
    else
    {}

#ifdef DEBUG_ECIES
    print_buf_U8(mac_ctx->out,mac_ctx->out_len, "mac-dec -value");
    print_buf_U8(cipher + cipher_len - mac_ctx->out_len,mac_ctx->out_len, "mac-dec-compare");
#endif

    if(ret == ECIES_SUCCESS)
    {
        ret = memcmp_(cipher_C3, mac_ctx->mac, mac_ctx->mac_bytes);
        if(0U == ret)
        {
            //4. msg = dec(k_enc,c)
#ifdef DEBUG_ECIES
            print_buf_U8(dec_ctx->key,dec_ctx->key_len, "dec-key");
            print_buf_U8(dec_ctx->msg,dec_ctx->msg_len, "dec-cip");
#endif
            ret = dec_ctx->dec_fun_imp(dec_ctx);
        }
        else
        {
            ret = ECIES_ERROR;
        }
    }
    else
    {}
    
    return ret;
}


/* function:  Elliptic Curve Integrated Encryption Scheme (ECIES) Decrypt core interface
 * parameters:
 *     ecies_ctx ------------------ input, types of ecies structure
 *     curve ---------------------- input, ecc curve struct pointer, please make sure it is valid
 *     cipher --------------------- input, ciphertext
 *     cipher_bytes --------------- input, byte length of ciphertext
 *     receiver_pri_key ----------- input, receiver's private key, big-endian.
 *     kdf_ctx -------------------- input, key derivation function structure.
 *     mac_ctx -------------------- input, message authentication code structure.
 *     dec_ctx -------------------- input, symmetric encryption scheme structure.
 *     msg ------------------------ output, decryption result, plaintext.
 *     msg_bytes ------------------ output, byte length of msg.
 * return:
 *     ECIES_SUCCESS(success); other(error)
 * caution:
 *     1. the input ciphertext consists of three parts. the 1st part is a point, the 2nd part is
 *        internal ciphertext, the 3rd part is mac.
 */
FLAG_STATIC uint32_t ecies_decrypt(const ECIES_STD *ecies_ctx, const eccp_curve_st *curve, const uint8_t *cipher,
        uint32_t cipher_bytes, const uint8_t *receiver_pri_key, E_KDF_BASE *kdf_ctx, E_MAC_BASE *mac_ctx,
        E_ENC_BASE *dec_ctx, uint8_t *msg, uint32_t *msg_bytes)
{
#if 0
    uint8_t enc_key[ECIES_BLOCK_ENC_K_E_Y_MAX_BYTE_LEN];
#endif
    uint32_t rx[ECCP_MAX_WORD_LEN];
    uint32_t ry[ECCP_MAX_WORD_LEN];
    uint32_t k[ECCP_MAX_WORD_LEN];
    uint32_t nWordLen;
    uint32_t nByteLen;
    uint32_t pWordLen;
    uint32_t pByteLen;
    uint32_t ret = ECIES_SUCCESS;

    pWordLen = GET_WORD_LEN(curve->eccp_p_bitLen);
    pByteLen = GET_BYTE_LEN(curve->eccp_p_bitLen);
    nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);
    nByteLen = GET_BYTE_LEN(curve->eccp_n_bitLen);

#if 0
    //get msg length
    ecies_ctx->get_point_len_from_ciphertext(curve, cipher, cipher_bytes, &point_bytes);

   if(dec_ctx->enc_type == XOR_ENC)
   {
       *msg_len = dec_ctx->key_len;
   }
   else
   {
        mac_ctx->msg_len = 0;
        ret = mac_ctx->mac_imp(mac_ctx);
        if(PKE_SUCCESS != ret)
        {
            return ret;
        }
        else
        {}

        *msg_len = cipher_len - point_len - mac_ctx->out_len;
   }
#endif

    //set enc key and output buffer, this must be done before KDF action.
    dec_ctx->output = msg;
    if(XOR_ENC == dec_ctx->enc_type)
    {
        dec_ctx->key = dec_ctx->output;
    }
    else
    {
#if 0
        //TODO
        dec_ctx->key = enc_key;
#endif
    }

    //set mac ctx
    mac_ctx->msg       = dec_ctx->input;
    mac_ctx->msg_bytes = dec_ctx->input_bytes;

#ifdef DEBUG_ECIES
    printf("msg-len : %d\n\r", *msg_len);
    print_buf_U8(cipher, cipher_len, "cipher-input");
#endif

    //1. get [pri_key]r.x
    rx[pWordLen - 1u] = 0u;
    ry[pWordLen - 1u] = 0u;
    ret = ecies_ctx->point_decompress(curve, cipher, rx, ry);
    if(PKE_SUCCESS == ret)
    {
        //check point r
        ret = eccp_check_point(curve, rx, ry);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        k[nWordLen - 1u] = 0u;
        reverse_byte_array(receiver_pri_key, (uint8_t *)k, nByteLen);

        //make sure pri_key in [1, n-1]
        ret = uint32_integer_check(k, curve->eccp_n, nWordLen, ECIES_ZERO_ALL, ECIES_INTEGER_TOO_BIG,
                ECIES_SUCCESS);
    }
    else
    {}

#ifdef DEBUG_ECIES
    print_BN_buf_U32(rx,pWordLen, "rx");
    print_BN_buf_U32(ry,pWordLen, "ry");
#endif

    if(ECIES_SUCCESS == ret)
    {
        ret = eccp_pointMul(curve, k, rx, ry, k, NULL);
    }
    else
    {}

#ifdef DEBUG_ECIES
    print_buf_U8(s, pByteLen, "s-dec");
#endif

    if(PKE_SUCCESS == ret)
    {
        reverse_byte_array((uint8_t *)k, (uint8_t *)k, pByteLen);
        ret = ecies_decrypt_C2_C3(curve, ecies_ctx, kdf_ctx, mac_ctx, dec_ctx, (uint8_t *)k, &cipher[cipher_bytes - mac_ctx->mac_bytes]);
    }
    else
    {}

    if(ret == ECIES_SUCCESS)
    {
        *msg_bytes = dec_ctx->output_bytes;// because may be padding
    }
    else
    {}

#ifdef DEBUG_ECIES
    print_buf_U8(dec_ctx->out,dec_ctx->msg_len, "dec-out");
#endif

    return ret;
}


FLAG_STATIC uint32_t ansi_x963_2001_ecies_common_param_check(const eccp_curve_st *curve,
        const uint8_t *shared_info1, uint32_t shared_info1_bytes, 
        const uint8_t *shared_info2, uint32_t shared_info2_bytes,
        hash_alg_e kdf_hash_alg, hash_alg_e mac_hash_alg,
        uint32_t mac_k_bytes)
{
    uint32_t ret = ECIES_SUCCESS;

    if(NULL == curve)
    {
        ret = ECIES_POINTER_NULL;
    }
    else if((NULL == shared_info1)&&(0u != shared_info1_bytes))
    {
        ret = ECIES_INVALID_INPUT;
    }
    else if((NULL == shared_info2)&&(0u != shared_info2_bytes))
    {
        ret = ECIES_INVALID_INPUT;
    }
    else
    {
        //handle other
    }

    if(ECIES_SUCCESS == ret)
    {
        if(HASH_SUCCESS != check_hash_alg(kdf_hash_alg))
        {
            ret = HASH_INPUT_INVALID;
        }
        else if(HASH_SUCCESS != check_hash_alg(mac_hash_alg))
        {
            ret = HASH_INPUT_INVALID;
        }
        else if((mac_k_bytes < ECIES_MAC_K_E_Y_ANSI_X963_MIN_BYTE_LEN) || (mac_k_bytes > ECIES_MAC_K_E_Y_MAX_BYTE_LEN))
        {
            ret = ECIES_INVALID_INPUT;
        }
        else
        {
            //handle other
        }
    }
    else
    {}

    return ret;
}


FLAG_STATIC uint32_t ansi_x963_2001_ecies_encrypt_param_check(const eccp_curve_st *curve, const uint8_t *msg, uint32_t msg_bytes,
        const uint8_t *shared_info1, uint32_t shared_info1_bytes, const uint8_t *shared_info2,
        uint32_t shared_info2_bytes, const uint8_t *receiver_pub_key,
        hash_alg_e kdf_hash_alg, hash_alg_e mac_hash_alg,
        uint32_t mac_k_bytes, const uint8_t *cipher, const uint32_t *cipher_bytes)
{
    uint32_t ret;

    ret = ansi_x963_2001_ecies_common_param_check(curve, shared_info1, shared_info1_bytes, shared_info2, shared_info2_bytes, 
            kdf_hash_alg, mac_hash_alg, mac_k_bytes);

    if(ECIES_SUCCESS == ret)
    {
        if((NULL == msg)||(NULL == receiver_pub_key)||(NULL == cipher)||(NULL == cipher_bytes))
        {
            ret = ECIES_POINTER_NULL;
        }
        else if((0u == msg_bytes))
        {
            ret = ECIES_INVALID_INPUT;
        }
        else
        {
            //handle other
        }
    }
    else
    {}

    return ret;
}


FLAG_STATIC uint32_t ansi_x963_2001_ecies_decrypt_param_check(const eccp_curve_st *curve, const uint8_t *cipher,
        const uint8_t *receiver_pri_key, const uint8_t *shared_info1, uint32_t shared_info1_bytes,
        const uint8_t *shared_info2, uint32_t shared_info2_bytes, hash_alg_e kdf_hash_alg,
        hash_alg_e mac_hash_alg, uint32_t mac_k_bytes, const uint8_t *msg, const uint32_t *msg_bytes)
{
    uint32_t ret;
    
    ret = ansi_x963_2001_ecies_common_param_check(curve, shared_info1, shared_info1_bytes, shared_info2, shared_info2_bytes, 
        kdf_hash_alg, mac_hash_alg, mac_k_bytes);

    if(ECIES_SUCCESS == ret)
    {
        if((NULL == cipher)||(NULL == receiver_pri_key)||(NULL == msg)||(NULL == msg_bytes))
        {
            ret = ECIES_POINTER_NULL;
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function:  Elliptic Curve Integrated Encryption Scheme (ECIES) ANSI-X963-2001
 * parameters:
 *     curve ---------------------- input, ecc curve struct pointer, please make sure it is valid
 *     msg ------------------------ input, original message, plaintext.
 *     msg_bytes ------------------ input, byte length of msg.
 *     shared_info1 --------------- input, optional, shared information, for KDF.
 *     shared_info1_bytes --------- input, byte length of shared_info1.
 *     shared_info2 --------------- input, optional, shared information, for MAC.
 *     shared_info2_bytes --------- input, byte length of shared_info2.
 *     sender_tmp_pri_key --------- input, sender's ephemeral private key, big-endian.
 *                                         if you do not have this, please set this parameter to be NULL,
 *                                         it will be generated inside.
 *     receiver_pub_key ----------- input, reveiver's public key, big-endian.
 *     conversion_form ------------ input, curve point representation.
 *     kdf_hash_alg --------------- input, specific hash algorithm for KDF.
 *     mac_hash_alg --------------- input, specific hash algorithm for MAC.
 *     mac_k_bytes ---------------- input, key length of the MAC.
 *     cipher --------------------- output, encryption result, ciphertext.
 *     cipher_bytes --------------- output, byte length of cipher.
 * return:
 *     ECIES_SUCCESS(success); other(error)
 * caution:
 *     1. if no shared_info1 needs to be provided, set the shared_info1 to NULL and the shared_info1_len to 0
 *     2. if no shared_info2 needs to be provided, set the shared_info2 to NULL and the shared_info2_len to 0
 *     3. if you do not have local_tmp_pri_key, please set the parameter to be NULL, it will be generated inside.
 *        this is recommended.
 *     4. the result ciphertext consists of three parts. the 1st part is a point, the 2nd part is
 *        internal ciphertext, the 3rd part is mac.
 */
uint32_t ansi_x963_2001_ecies_encrypt(const eccp_curve_st *curve, uint8_t *msg, uint32_t msg_bytes,
        uint8_t *shared_info1, uint32_t shared_info1_bytes, uint8_t *shared_info2,
        uint32_t shared_info2_bytes, const uint8_t *sender_tmp_pri_key, const uint8_t *receiver_pub_key,
        EC_POINT_FORM point_form, hash_alg_e kdf_hash_alg, hash_alg_e mac_hash_alg,
        uint32_t mac_k_bytes, uint8_t *cipher, uint32_t *cipher_bytes)
{
    uint32_t ret;
    uint8_t hmac_key[ECIES_MAC_K_E_Y_MAX_BYTE_LEN];
    e_kdf_ansi_x963_2001_ctx_st kdf_ctx;
    E_XOR_ENC_CTX xor_ctx;
    e_hmac_ctx_st hmac_ctx;
    ECIES_STD ecies_ctx;

    ret = ansi_x963_2001_ecies_encrypt_param_check(curve, msg, msg_bytes, shared_info1, shared_info1_bytes, shared_info2, 
        shared_info2_bytes, receiver_pub_key, kdf_hash_alg, mac_hash_alg, mac_k_bytes, cipher, cipher_bytes);

    if(ECIES_SUCCESS == ret)
    {
        ecies_ansi_x963_ctx_init(&ecies_ctx, ENC_MAC_ORDER);

        ansi_x963_2001_kdf_init(&kdf_ctx, shared_info1, shared_info1_bytes, kdf_hash_alg);

        e_xor_enc_init(&xor_ctx, msg, msg_bytes);

        e_hmac_init(&hmac_ctx, hmac_key, mac_k_bytes, shared_info2, shared_info2_bytes, mac_hash_alg);

        ret = ecies_encrypt(&ecies_ctx, curve, sender_tmp_pri_key, receiver_pub_key,
            point_form, &kdf_ctx.base, &hmac_ctx.base, &xor_ctx.base,
            cipher, cipher_bytes);
    }
    else
    {}

    return ret;
}


/* function:  Elliptic Curve Integrated Encryption Scheme (ECIES) Decrypt ANSI-X963-2001
 * parameters:
 *     curve ---------------------- input, ecc curve struct pointer, please make sure it is valid
 *     cipher --------------------- input, ciphertext
 *     cipher_len ----------------- input, byte length of ciphertext
 *     receiver_pri_key ----------- input, receiver's private key, big-endian.
 *     shared_info1 --------------- input, optional, shared information, for KDF.
 *     shared_info1_bytes --------- input, byte length of shared_info1.
 *     shared_info2 --------------- input, optional, shared information, for MAC.
 *     shared_info2_bytes --------- input, byte length of shared_info2.
 *     kdf_hash_alg --------------- input, specific hash algorithm for KDF.
 *     mac_hash_alg --------------- input, specific hash algorithm for KDF.
 *     mac_k_bytes ---------------- input, key length of the MAC.
 *     msg ------------------------ output, decryption result.
 *     msg_bytes ------------------ output, byte length of msg.
 * return:
 *     ECIES_SUCCESS(success); other(error)
 * caution:
 *     1. if no shared_info1 needs to be provided, set the shared_info1 to NULL and the shared_info1_len to 0
 *     2. if no shared_info2 needs to be provided, set the shared_info2 to NULL and the shared_info2_len to 0
 *     3. the input cipher consists of three parts. the 1st part is a point, the 2nd part is
 *        internal ciphertext, the 3rd part is mac.
 */
uint32_t ansi_x963_2001_ecies_decrypt(const eccp_curve_st *curve, uint8_t *cipher, uint32_t cipher_bytes,
        const uint8_t *receiver_pri_key, uint8_t *shared_info1, uint32_t shared_info1_bytes,
        uint8_t *shared_info2, uint32_t shared_info2_bytes, hash_alg_e kdf_hash_alg,
        hash_alg_e mac_hash_alg, uint32_t mac_k_bytes, uint8_t *msg, uint32_t *msg_bytes)
{
    uint32_t ret;
    uint8_t mac_buf[ECIES_MAC_MAX_BYTE_LEN];
    uint8_t hmac_key[ECIES_MAC_K_E_Y_MAX_BYTE_LEN];
    uint32_t point_bytes;
    uint32_t mac_bytes;
    e_kdf_ansi_x963_2001_ctx_st kdf_ctx;
    E_XOR_ENC_CTX xor_ctx;
    e_hmac_ctx_st hmac_ctx;
    ECIES_STD ecies_ctx;

    ret = ansi_x963_2001_ecies_decrypt_param_check(curve, cipher, receiver_pri_key, shared_info1, shared_info1_bytes,
        shared_info2,shared_info2_bytes, kdf_hash_alg, mac_hash_alg, mac_k_bytes, msg, msg_bytes);

    if(ECIES_SUCCESS == ret)
    {
        ecies_ansi_x963_ctx_init(&ecies_ctx, ENC_MAC_ORDER);

        e_hmac_init(&hmac_ctx, hmac_key, mac_k_bytes, shared_info2, shared_info2_bytes, mac_hash_alg);

        //Because XOR encryption requires the length of the message,
        //it is necessary to obtain the length of the mac
        mac_bytes = hmac_ctx.base.mac_bytes;

        ret = ecies_ctx.get_point_len_from_ciphertext(curve, cipher, &point_bytes);
        if(PKE_SUCCESS == ret)
        {
            if(cipher_bytes <= (point_bytes + mac_bytes))
            {
                ret = ECIES_INVALID_INPUT;
            }
            else
            {
                *msg_bytes = cipher_bytes - point_bytes - mac_bytes;

                e_xor_enc_init(&xor_ctx, &cipher[point_bytes], *msg_bytes);

                ansi_x963_2001_kdf_init(&kdf_ctx, shared_info1, shared_info1_bytes, kdf_hash_alg);

                ((E_MAC_BASE *)(&hmac_ctx.base))->mac       = mac_buf;

                ret = ecies_decrypt(&ecies_ctx, curve, cipher, cipher_bytes, receiver_pri_key,
                    &kdf_ctx.base, &hmac_ctx.base, &xor_ctx.base, msg, msg_bytes);
            }
        }
        else
        {}
    }

    return ret;
}

#endif
