
#include "../../crypto_include/hash_hmac/pbkdf2_kmu.h"
#ifdef CONFIG_EHSM_CUSTOM_ID
#include "driver/kmu_driver.h"
#include "driver/sysreg.h"
#endif

#if (defined(SUPPORT_PBKDF2) && defined(SUPPORT_PBKDF2_KMU))

#define MOVE_TO_KMU_ONCE_WORD (8U)

static uint32_t cpu_read_hash_dout_flag = (~0U);

static void set_cpu_not_read_hash_dout(void)
{
    cpu_read_hash_dout_flag = 0U;
}

static void set_cpu_read_hash_dout(void)
{
    cpu_read_hash_dout_flag = (~0U);
}

#ifdef PBKDF2_HIGH_SPEED
/* function: pbkdf2 backup hmac ctx
 * parameters:
 *     ctx_bak -------------------- output, hmac ctx
 *     ctx ------------------------ input, hmac ctx to be backup
 *     iterator ------------------- output, if is not NULL,
 *                                  means for SHA3, iterator of K0 XOR IPAD
 * return:
 * caution:
 *     1. if hash algorithm is SHA3, it needs iterator, otherwise not. since hmac-sha3 is not
 *        supported by hardware.
 */
static void pbkdf2_hmac_backup(hmac_st *ctx_bak, const hmac_st *ctx, uint32_t *iterator)
{
    memcpy_(ctx_bak, ctx, sizeof(hmac_st));

    if (NULL != iterator) {
        if ((ctx->hash_ctx->alg >= HASH_SHA3_224) && (ctx->hash_ctx->alg <= HASH_SHA3_512)) {
            hash_get_iterator((uint8_t *)iterator, ctx->hash_ctx->iterator_word_len);
        } else {
        }
    } else {
    }
}
#endif

/* function: pbkdf2 recover hmac ctx
 * parameters:
 *     ctx ------------------------ output, hmac ctx to be recover
 *     ctx_bak -------------------- input, hmac ctx
 *     iterator ------------------- output, if is not NULL,
 *                                  means for SHA3, iterator of K0 XOR IPAD
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. if hash algorithm is SHA3, it needs iterator, otherwise not. since hmac-sha3 is not
 *        supported by hardware.
 */
static uint32_t pbkdf2_hmac_recover(hmac_st *ctx, const hmac_st *ctx_bak, const uint32_t *iterator)
{
    uint32_t ret = HASH_SUCCESS;

    memcpy_(ctx, ctx_bak, sizeof(hmac_st));

    if ((ctx->hash_ctx->alg >= HASH_SHA3_224) && (ctx->hash_ctx->alg <= HASH_SHA3_512)) {
        hash_set_cpu_mode();
        hash_set_hash_mode();
        hash_set_endian_uint32();
        hash_disable_cpu_interruption();
        hash_set_last_block(0); // set not the last block
        hash_set_alg(ctx->hash_ctx->alg);
        hash_update_config();

        hash_set_iterator(iterator, ctx->hash_ctx->iterator_word_len);
    } else {
        ret = hmac_key_state_recover(ctx->hash_ctx->alg, ctx->key_len_flag, ctx->K0,
            (uint32_t)(ctx->hash_ctx->block_byte_len), (uint32_t)(ctx->hash_ctx->iterator_word_len), ctx->is_sp_key,
            ctx->sp_key_idx, ctx->sp_key_bytes);
    }

    return ret;
}

/* function: pbkdf2 internal function(U = hmac(Password, Salt||counter) or U = hmac(Password, hmac))
 * parameters:
 *     ctx ------------------------ input, hmac_st context pointer
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the input message
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the input message
 *     mac ------------------------ output, hmac
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1.use for pbkdf2, get U1,U2 ...
 */
static uint32_t pbkdf2_hmac_update_get_mac(
    hmac_st *ctx, const uint8_t *msg1, uint32_t msg1_bytes, const uint8_t *msg2, uint32_t msg2_bytes, uint8_t *mac)
{
    uint32_t ret;
    uint8_t tmp;

    ret = hash_update(ctx->hash_ctx, msg1, msg1_bytes);

    if (HASH_SUCCESS == ret) {
        ret = hash_update(ctx->hash_ctx, msg2, msg2_bytes);
    } else {
    }

    if (HASH_SUCCESS == ret) {
        // for hmac-sha3
#if (defined(SUPPORT_HASH_SHA3_224) || defined(SUPPORT_HASH_SHA3_256) || defined(SUPPORT_HASH_SHA3_384) \
    || defined(SUPPORT_HASH_SHA3_512))
        if ((ctx->hash_ctx->alg >= HASH_SHA3_224) && (ctx->hash_ctx->alg <= HASH_SHA3_512)) {
#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
            // IP not diretlly support hmac_sha3
            hash_hmac_disable_secure_port();
#endif
            ret = HASH_INPUT_INVALID;
        } else {
#endif

            // for hmac-non-sha3
#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
            ret = hmac_key_state_recover(ctx->hash_ctx->alg, ctx->key_len_flag, ctx->K0, ctx->hash_ctx->block_byte_len,
                ctx->hash_ctx->iterator_word_len, ctx->is_sp_key, ctx->sp_key_idx, ctx->sp_key_bytes);
            if (HASH_SUCCESS == ret) {
#endif
                ctx->hash_ctx->finish_flag = (uint8_t)1; // the last block calc

                // get the byte length of the remainder msg(less than one block)
                tmp = (uint8_t)(ctx->hash_ctx->total[0] % (ctx->hash_ctx->block_byte_len));

                // set total msg bit length
                hash_total_bytelen_2_bitlen(ctx->hash_ctx->total, CAST2UINT32(ctx->hash_ctx->block_byte_len) / 32U);
                hash_set_msg_total_bit_len(ctx->hash_ctx->total, ctx->hash_ctx->block_byte_len);

                // input the remainder msg(less than one block)
                ret = hash_calc_rand_len_msg(ctx->hash_ctx, ctx->hash_ctx->hash_buffer, tmp);
                if (HASH_SUCCESS == ret) {
                    if (0U != cpu_read_hash_dout_flag) {
                        // get the hash result
                        hash_get_iterator(mac, CAST2UINT32(ctx->hash_ctx->digest_byte_len) >> 2);
                    }
                } else {
                }
#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
            } else {
            }
#endif
#if (defined(SUPPORT_HASH_SHA3_224) || defined(SUPPORT_HASH_SHA3_256) || defined(SUPPORT_HASH_SHA3_384) \
    || defined(SUPPORT_HASH_SHA3_512))
        }
#endif
        // clear the context for non-sha3
        memset_((uint8_t *)ctx, 0, sizeof(hmac_st));
    } else {
    }

    return ret;
}

/* function: pbkdf2 internal function(out = U1 xor U2 xor ... Uc)
 * parameters:
 *     ctx ------------------------ input, hmac_st context pointer
 *     hash_alg ------------------- input, specific hash algorithm
 *     iterator ------------------- output, if is not NULL,
 *                                  means for SHA3, iterator of K0 XOR IPAD
 *     counter  ------------------- input, counter of 4 bytes
 *     salt ----------------------- input, salt
 *     salt_bytes ----------------- input, byte length of salt, it could be 0
 *     iter ----------------------- input, iteration times
 *     out ------------------------ output, derived key
 *     out_bytes ------------------ input, byte length of derived key
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1.if open the macro PBKDF2_HIGH_SPEED, the ctx means already init,
 */
#ifdef PBKDF2_HIGH_SPEED
static uint32_t pbkdf2_hmac_calc_f_kmu(const hmac_st *ctx, hash_alg_e hash_alg, const uint32_t *iterator,
    const uint8_t counter[4], const uint8_t *salt, uint32_t salt_bytes, uint32_t iter)
#else
static uint32_t pbkdf2_hmac_calc_f_kmu(hmac_st *ctx, hash_alg_e hash_alg, const uint8_t *pwd, uint16_t sp_key_idx,
    uint32_t pwd_bytes, uint32_t *iterator, uint8_t counter[4], const uint8_t *salt, uint32_t salt_bytes, uint32_t iter)
#endif
{
    uint32_t result[HASH_DIGEST_MAX_WORD_LEN];
    uint32_t digest[HASH_DIGEST_MAX_WORD_LEN];
    uint32_t digest_words, digest_bytes;
    hmac_st *ctx_ptr;
    uint32_t ret = HASH_SUCCESS;
    uint32_t i, j;

#ifdef PBKDF2_HIGH_SPEED
    hmac_st ctx_tmp[1];
    ctx_ptr = ctx_tmp;
#else
    ctx_ptr = ctx;
#endif

    digest_words = hash_get_digest_word_len(hash_alg);
    digest_bytes = digest_words << 2;

    // get U1
#ifdef PBKDF2_HIGH_SPEED
    ret = pbkdf2_hmac_recover(ctx_ptr, ctx, iterator);
#else
    ret = hmac_init(ctx, hash_alg, pwd, sp_key_idx, pwd_bytes);
#endif

    if (1U == iter) {
        for (i = 0U; i < MOVE_TO_KMU_ONCE_WORD; i++) {
            sysreg_set_gen_reg((uint8_t)i, 0U);
        }

        set_cpu_not_read_hash_dout();
    } else {
    }

    if (HASH_SUCCESS == ret) {
        ret = pbkdf2_hmac_update_get_mac(ctx_ptr, salt, salt_bytes, counter, 4, (uint8_t *)result);
    } else {
    }

    // get U1 xor U2 xor ... xor Uc
    if (HASH_SUCCESS == ret) {
        uint32_copy(digest, result, digest_words);

        for (i = 1U; i < iter; i++) {
#ifdef PBKDF2_HIGH_SPEED
            ret = pbkdf2_hmac_recover(ctx_ptr, ctx, iterator);
#else
            ret = hmac_init(ctx, hash_alg, pwd, sp_key_idx, pwd_bytes);
#endif
            if (HASH_SUCCESS == ret) {
                if (i == (iter - 1U)) {
                    set_cpu_not_read_hash_dout();
                    ret = pbkdf2_hmac_update_get_mac(
                        ctx_ptr, (const uint8_t *)digest, digest_bytes, NULL, 0, (uint8_t *)digest);
                    set_cpu_read_hash_dout();

                    for (j = 0; j < MOVE_TO_KMU_ONCE_WORD; j++) {
                        sysreg_set_gen_reg((uint8_t)j, result[j]);
                    }
                    uint32_clear(result, digest_words);
                } else {
                    ret = pbkdf2_hmac_update_get_mac(
                        ctx_ptr, (const uint8_t *)digest, digest_bytes, NULL, 0, (uint8_t *)digest);

                    for (j = 0; j < digest_words; j++) {
                        result[j] ^= digest[j];
                    }
                }
            } else {
                break;
            }
        }
    } else {
    }

    return ret;
}

/* function: pbkdf2 generate to kmu of 32Byte(using hmac as PRF)
 * parameters:
 *     hash_alg_e ----------------- input, specific hash algorithm
 *     sp_key_idx ----------------- input, index of secure port key(password)
 *     pwd_bytes ------------------ input, byte length of password, it could be 0
 *     salt ----------------------- input, salt
 *     salt_bytes ----------------- input, byte length of salt, it could be 0
 *     iter ----------------------- input, iteration times
 *     dst_key_idx ---------------- input, index of secure dst port key(kdf)
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1.only support derive 32Byte key, not support other length of derive_key
 *     2.not support SHA3(hardeare not directly support hmac-sha3), MD5, SHA1
 */
uint32_t pbkdf2_hmac_kmu(hash_alg_e hash_alg, uint16_t sp_key_idx, uint32_t pwd_bytes, const uint8_t *salt,
    uint32_t salt_bytes, uint32_t iter, uint16_t dst_key_idx)
{
    uint32_t digest_words, digest_bytes, tmp_bytes;
    uint8_t counter[4] = { 0, 0, 0, 1 };
    uint32_t actual_pwd_bytes = pwd_bytes;
    uint32_t actual_salt_bytes = salt_bytes;
    uint32_t ret = HASH_SUCCESS;
    uint32_t remainder_out_bytes = 32U;

#if ((!defined(CONFIG_HASH_SUPPORT_MUL_THREAD))                                                            \
    && (defined(SUPPORT_HASH_SHA3_224) || defined(SUPPORT_HASH_SHA3_256) || defined(SUPPORT_HASH_SHA3_384) \
        || defined(SUPPORT_HASH_SHA3_512)))
    uint32_t iterator[HASH_ITERATOR_MAX_WORD_LEN];
#endif

    hmac_st ctx[1];

#ifdef PBKDF2_HIGH_SPEED
    hmac_st ctx_bak[1];
#endif

    if (HASH_SUCCESS != check_hash_alg(hash_alg)) {
        ret = HASH_INPUT_INVALID;
    } else if (pwd_bytes > 32U) {
        ret = HASH_INPUT_INVALID;
    } else {
        // nothing to do
    }

    digest_words = hash_get_digest_word_len(hash_alg);
    digest_bytes = 4U * digest_words;

    if (digest_bytes < 32U) {
        ret = HASH_INPUT_INVALID;
    } else {
    }

    if (HASH_SUCCESS == ret) {
        if (NULL == salt) {
            actual_salt_bytes = 0;
        } else {
        }

#ifdef PBKDF2_HIGH_SPEED
        ret = hmac_init(ctx, hash_alg, NULL, sp_key_idx, actual_pwd_bytes);
        if (HASH_SUCCESS == ret) {
#if ((!defined(CONFIG_HASH_SUPPORT_MUL_THREAD))                                                            \
    && (defined(SUPPORT_HASH_SHA3_224) || defined(SUPPORT_HASH_SHA3_256) || defined(SUPPORT_HASH_SHA3_384) \
        || defined(SUPPORT_HASH_SHA3_512)))
            pbkdf2_hmac_backup(ctx_bak, ctx, iterator);
#else
            pbkdf2_hmac_backup(ctx_bak, ctx, NULL);
#endif
#endif
            while (0U != remainder_out_bytes) {
                if (remainder_out_bytes > digest_bytes) {
                    tmp_bytes = digest_bytes;
                } else {
                    tmp_bytes = remainder_out_bytes;
                }

#ifdef PBKDF2_HIGH_SPEED
#if ((!defined(CONFIG_HASH_SUPPORT_MUL_THREAD))                                                            \
    && (defined(SUPPORT_HASH_SHA3_224) || defined(SUPPORT_HASH_SHA3_256) || defined(SUPPORT_HASH_SHA3_384) \
        || defined(SUPPORT_HASH_SHA3_512)))
                ret = pbkdf2_hmac_calc_f_kmu(ctx_bak, hash_alg, iterator, counter, salt, actual_salt_bytes, iter);
#else
                ret = pbkdf2_hmac_calc_f_kmu(ctx_bak, hash_alg, NULL, counter, salt, actual_salt_bytes, iter);
#endif
#else
            ret = pbkdf2_hmac_calc_f_kmu(ctx, hash_alg, pwd, sp_key_idx, actual_pwd_bytes, NULL, counter, salt,
                actual_salt_bytes, iter, tmp_bytes);
#endif
                if (HASH_SUCCESS == ret) {
                    // add 1
                    (void)uint8_big_num_big_endian_add_little(counter, 4, 1, 1);

                    remainder_out_bytes -= tmp_bytes;
                } else {
                    break;
                }
            }
#ifdef PBKDF2_HIGH_SPEED
        }
#endif
    } else {
    }

    if (HASH_SUCCESS == ret) {
        ret = kmu_config_derive_key(dst_key_idx);
    }

    return ret;
}
#endif
