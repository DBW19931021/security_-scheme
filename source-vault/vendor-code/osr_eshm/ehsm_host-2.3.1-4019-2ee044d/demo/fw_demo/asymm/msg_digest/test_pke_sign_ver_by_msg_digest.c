#include <string.h>

#include "common/mem.h"

#include "ehsmdrv/basic/api.h"
#include "common/utils.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"
#include "fw_demo/asymm/msg_digest/test_pke_sign_ver_by_msg_digest.h"

// #include "ehsmdrv/basic/api.h"
// #include "ehsmdrv/basic/port/ehsm_host_port.h"

struct rsacipher_testvec {
    const uint8_t *n;
    const uint8_t *e;
    const uint8_t *d;
    const uint8_t *p;
    const uint8_t *q;
    const uint8_t *dp;
    const uint8_t *dq;
    const uint8_t *u;
    uint32_t n_byte_size;
    uint32_t e_byte_size;
    uint32_t d_byte_size;
    uint32_t p_byte_size;
    uint32_t q_byte_size;
    uint32_t dp_byte_size;
    uint32_t dq_byte_size;
    uint32_t u_byte_size;
    const uint8_t *m;
    uint32_t m_size;
    const uint8_t *dgst;
    uint32_t dgst_size;
    uint32_t is_crt_mode;
    uint8_t public_key_vec;
    uint8_t siggen_sigver_test;
    uint8_t hash_alg;

    /* PKCS#1 RSASSA-PSS */
    const uint8_t *salt;
    uint32_t salt_byte_size;
};

static volatile bool_t s_work_done = false;

static void ehsm_demo_callback(ehsm_ctx_st *ctx)
{
    (void)ctx;
    s_work_done = true;
}

static uint32_t wait_work_done(bool_t async, bool_t callback, ehsm_ctx_st *ctx, uint32_t ret)
{
    (void)async;

    // 反复poll状态，看是否完成
    while (ret == EHSM_ERR_NEED_POLL) {
        ret = ehsm_ctx_poll(ctx);
    }

    // 只要设置callback并返回成功，则callback一定会被调用
    if (callback && ret == EHSM_OK) {
        if (!s_work_done) {
            ehsm_port_printf("warn: callback is not called!\n");
        }
    }
    return ret;
}

static const uint8_t osr_rsa_sig_verify_tv_template_1024_m[] = "test";

static const uint8_t osr_rsa_sig_verify_tv_template_1024_c_sha1[]
    = "\xA9\x4A\x8F\xE5\xCC\xB1\x9B\xA6\x1C\x4C\x08\x73\xD3\x91\xE9\x87\x98\x2F\xBB\xD3";

static const uint8_t osr_rsa_sig_verify_tv_template_1024_e[] = "\x01\x00\x01";

static const uint8_t osr_rsa_sig_verify_tv_template_1024_d[]
    = "\x87\xA7\x88\x7A\x44\x93\x09\x39\xCD\xD0\x4C\x0A\x41\xD4\x00\x3B"
      "\x4D\x93\x84\xEF\x4A\xFD\xB2\x70\x89\xC4\x05\xCF\xC2\xCB\x77\x4B"
      "\xF4\x21\xB0\x1D\x87\x2D\xDD\xBF\xEE\xE3\x1B\xEA\xA3\x7C\x68\x33"
      "\x3E\xE0\x44\x4F\x0E\x71\x48\xDA\x3C\x62\x4A\x0D\xAB\xB1\x0F\x19"
      "\x99\xF6\x7B\x0F\xE1\x84\xCA\xEA\xED\x33\x9C\xCD\x04\x3D\x62\x6B"
      "\xD8\x7D\xF1\x57\x3B\x85\x60\xC0\x88\xDE\x8D\x57\xE9\x57\x9E\x1F"
      "\x2F\x1E\x5F\x5E\x2F\x94\x3A\x38\xDA\x78\xE5\x3B\xA4\x0F\xAE\xDB"
      "\xBC\xF7\x3E\x2E\xE4\x8F\x84\x78\xAB\x07\x6E\xDF\xFA\x42\x55\x91";

static const uint8_t osr_rsa_sig_verify_tv_template_1024_n[]
    = "\xB8\x07\x1F\x69\x7D\x5A\x20\x4E\xCD\xA3\x46\x80\x96\x66\x94\x9A"
      "\xC3\xC9\x07\x9D\x23\xB0\x7F\xEE\xAF\x00\x32\xF9\xEE\x30\xDE\x6E"
      "\xAF\x4D\xA2\x1D\xE7\x89\x3D\xC3\xEE\xC3\x20\xCA\x20\x46\xF1\x5C"
      "\xAD\x31\x8A\xD7\xFC\x2F\xBB\x0B\xA1\xFF\x1F\x66\xFD\xFD\x7E\x10"
      "\xBC\xEA\x78\x24\x29\x72\x66\x2D\x90\xCB\xE5\xBF\x95\x64\x16\x9F"
      "\x7D\xC9\xC8\x85\x31\xBF\x52\xD2\x5C\x08\x9A\xCD\x80\x8E\xF8\xBF"
      "\xA7\x1A\xF0\xC8\x7C\xC0\x28\x6A\x36\x9E\x18\x0A\x63\x14\x92\xB5"
      "\x23\x29\x4E\xBE\x74\x98\xF4\xAD\xFE\xB6\x00\xB4\xAF\x44\x8E\x87";

static const struct rsacipher_testvec osr_rsa_sig_verify_tv_template[] = {
    // 1024
    {
        .hash_alg = EHSM_HASH_ALGO_SHA1,
        .is_crt_mode = 1,
        .public_key_vec = 0,
        .siggen_sigver_test = 1,
        .m = osr_rsa_sig_verify_tv_template_1024_m,
        .m_size = 4,
        .dgst = osr_rsa_sig_verify_tv_template_1024_c_sha1,
        .dgst_size = 20,
        .e = osr_rsa_sig_verify_tv_template_1024_e,
        .e_byte_size = 3,
        .d = osr_rsa_sig_verify_tv_template_1024_d,
        .d_byte_size = 128,
        .n = osr_rsa_sig_verify_tv_template_1024_n,
        .n_byte_size = 128,
    },
};

static uint32_t local_import_rsa_key(ehsm_ctx_st *ctx, const struct rsacipher_testvec *vect, uint32_t *handle)
{
    uint32_t ret;
    uint8_t *key_space = ehsm_demo_get_buffer(5);
    ehsm_key_format_st *key = (ehsm_key_format_st *)key_space;
    memset(key, 0x00, 1024);
    key->key_type = EHSM_KEY_TYPE_RSA_1024;
    key->privilege = EHSM_KEY_PRIV_SIGN | EHSM_KEY_PRIV_REMOVE | EHSM_KEY_PRIV_VERIFY;
    uint32_t key_data_size = 0;

    // 私钥签名，公钥验签； 公钥加密，私钥解密 全部传入
    key->part_info = EHSM_KEY_PART_KEY_PAIR;
    key->pub_key_size = (uint16_t)((vect->e_byte_size + 3) / 4) * 4; // e 4字节对齐
    memcpy(&key->key_value[key->pub_key_size - vect->e_byte_size], vect->e, vect->e_byte_size);
    memcpy(&key->key_value[key->pub_key_size], vect->n, vect->n_byte_size);
    uint32_t pub_size = sizeof(ehsm_key_format_st) + key->pub_key_size + vect->n_byte_size;

    key->priv_key_size = (uint16_t)vect->d_byte_size;
    memcpy(&key->key_value[key->pub_key_size + vect->n_byte_size], vect->d, vect->d_byte_size);
    key_data_size = pub_size + vect->d_byte_size;

    ret = ehsm_km_import_key(ctx, 0xFFFFFFFF, 0xFFFFFFFF, key, key_data_size, NULL, 0, handle);
    if (ret != EHSM_OK) {
        ehsm_port_printf("ret =(%dU)\n", ret);
        return ret;
    }

    return ret;
}

static uint32_t ehsm_demo_test_rsa_sign_vrf(bool_t async, bool_t callback, bool_t *verify_result)
{
    uint32_t ret;
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx();
    uint32_t key_handle = 0xFFFFFFFFU;
    uint8_t *msg = ehsm_demo_get_buffer(0);
    uint8_t *digest = ehsm_demo_get_buffer(1);
    uint8_t *sig = ehsm_demo_get_buffer(2);
    uint32_t sig_size = 1024;
    const struct rsacipher_testvec *vect = osr_rsa_sig_verify_tv_template;

    ehsm_port_printf("ehsm_demo_test_rsa, async: %d, callback: %d\n", (int)async, (int)callback);

    ehsm_ctx_init(ctx, 0, /*async=*/async, callback ? ehsm_demo_callback : NULL);

    ret = local_import_rsa_key(ctx, vect, &key_handle);
    ret = wait_work_done(async, callback, ctx, ret);
    if (ret != EHSM_OK) {
        // error
        ehsm_port_printf("local_import_rsa_key failed, code: 0x%08x\n", ret);
        return ret;
    }

    memcpy(msg, vect->m, vect->m_size);
    ret = ehsm_rsa_sign_onepass_gen(
        ctx, (ehsm_hash_algo_e)vect->hash_alg, key_handle, EHSM_RSA_PADDING_NONE, msg, vect->m_size, sig, &sig_size, 0);
    ret = wait_work_done(async, callback, ctx, ret);

    if (ret != EHSM_OK) {
        // error
        ehsm_port_printf("ehsm_hash_init failed, code: 0x%08x\n", ret);
        ehsm_km_remove_key(ctx, key_handle);
        return ret;
    }

    *verify_result = false;
    memcpy(msg, vect->m, vect->m_size);
    ret = ehsm_rsa_sign_onepass_verify(ctx, (ehsm_hash_algo_e)vect->hash_alg, key_handle, EHSM_RSA_PADDING_NONE, msg,
        vect->m_size, sig, sig_size, 0, verify_result);
    ret = wait_work_done(async, callback, ctx, ret);
    if (ret != EHSM_OK) {
        ehsm_port_printf("ehsm_rsa_sign_onepass_verify failed, code: 0x%08x\n", ret);
        return ret;
    }

    if (*verify_result != true) {
        ehsm_port_printf("ehsm_rsa_sign_onepass_verify  verify failed, code: 0x%08x\n", ret);
        return ret;
    }

    memcpy(digest, vect->dgst, vect->dgst_size);
    ret = ehsm_rsa_sign_onepass_gen_with_digest(ctx, (ehsm_hash_algo_e)vect->hash_alg, key_handle,
        EHSM_RSA_PADDING_NONE, digest, vect->dgst_size, sig, &sig_size, 0);
    ret = wait_work_done(async, callback, ctx, ret);
    if (ret != EHSM_OK) {
        // error
        ehsm_port_printf("ehsm_hash_init failed, code: 0x%08x\n", ret);
        ehsm_km_remove_key(ctx, key_handle);
        return ret;
    }

    *verify_result = false;
    memcpy(digest, vect->dgst, vect->dgst_size);
    ret = ehsm_rsa_sign_onepass_verify_with_digest(ctx, (ehsm_hash_algo_e)vect->hash_alg, key_handle,
        EHSM_RSA_PADDING_NONE, digest, vect->dgst_size, sig, sig_size, 0, verify_result);
    ret = wait_work_done(async, callback, ctx, ret);
    if (ret != EHSM_OK) {
        ehsm_port_printf("ehsm_hash_update failed, code: 0x%08x\n", ret);
        ehsm_km_remove_key(ctx, key_handle);
        return ret;
    }

    if (*verify_result != true) {
        ehsm_port_printf("ehsm_rsa_sign_onepass_verify  verify failed, code: 0x%08x\n", ret);
        return ret;
    }

    memcpy(digest, vect->dgst, vect->dgst_size);
    ret = ehsm_rsa_sign_onepass_gen_with_digest(ctx, (ehsm_hash_algo_e)vect->hash_alg, key_handle, EHSM_RSA_PADDING_PSS,
        digest, vect->dgst_size, sig, &sig_size, 64);
    ret = wait_work_done(async, callback, ctx, ret);
    if (ret != EHSM_OK) {
        // error
        ehsm_port_printf("ehsm_hash_init failed, code: 0x%08x\n", ret);
        ehsm_km_remove_key(ctx, key_handle);
        return ret;
    }

    *verify_result = false;
    memcpy(digest, vect->dgst, vect->dgst_size);
    ret = ehsm_rsa_sign_onepass_verify_with_digest(ctx, (ehsm_hash_algo_e)vect->hash_alg, key_handle,
        EHSM_RSA_PADDING_PSS, digest, vect->dgst_size, sig, sig_size, 64, verify_result);
    ret = wait_work_done(async, callback, ctx, ret);
    if (ret != EHSM_OK) {
        ehsm_port_printf("ehsm_hash_update failed, code: 0x%08x\n", ret);
        ehsm_km_remove_key(ctx, key_handle);
        return ret;
    }

    if (*verify_result != true) {
        ehsm_port_printf("ehsm_rsa_sign_onepass_verify  verify failed, code: 0x%08x\n", ret);
        return ret;
    }

    return ret;
}

/**
 * @brief
 *
 * @param key Public key only or contain private key. Depending on public_key_vec, if public_key_vec is true. This is
 *            only the public key, otherwise its the public key and private key (private key is first). The keys are
 *            just in raw data. No '0x04' is put for public key. If the code need '0x04', please add it in test code.
 * @params: Random generated number for signature.
 * @m: Original message.
 * @c: Signature in (r, s) mode.
 * @key_len: Size of key in bytes.
 * @param_len: Size of @params in bytes.
 * @m_size: Size of message in bytes.
 * @c_size: Size of signature in bytes.
 * @public_key_vec: Is the key only contain public key.
 * @siggen_sigver_test: Test for signature and verification if ture.
 *                      Otherwise, test for encryption and decryption.
 * @hash_alg: Hash algorithm.
 */
struct akcipher_testvec {
    const uint8_t *key;
    const uint8_t *params;
    const uint8_t *m;
    const uint8_t *c;
    uint32_t key_len;
    uint32_t param_len;
    uint32_t m_size;
    uint32_t c_size;
    uint8_t public_key_vec;
    uint8_t siggen_sigver_test;
    uint8_t hash_alg;
};
static const struct akcipher_testvec g_sm2_sigverify_tv_template[] = {
    /*
     * Generated by OSR SM2 tool: Test for signification and verification
     * Tested on https://const.net.cn/tool/sm2/verify
     */
    {
        .key = (const uint8_t *)"\x04" /* public key */
                                "\x14\xD5\x5E\x8C\xB4\x21\x00\xAA\x35\x62\x7C\x0A\x47\x1D\x48\x4C"
                                "\x9C\x1D\x0C\x45\x5C\x2A\xBF\x46\x00\xC7\x49\x48\x9A\x93\xB5\xE6"
                                "\x50\x6D\xBC\x1C\x09\x86\x6D\x02\x08\xE0\xC7\x3B\x5F\xA8\xDF\x52"
                                "\xCA\x79\x6A\x81\x30\xCC\x9D\xC8\xED\x04\xB8\x88\x9E\xE6\x4D\x6D"
                                /* private key */
                                "\x9A\x04\xAB\x87\x83\x31\x9C\xCA\x63\xE8\x16\x74\xE1\x9D\xA2\x23"
                                "\x17\x07\x83\xFA\xEE\x2F\x98\x09\xC0\xC7\x6C\xE0\x8E\x24\x7E\xA1",
        .key_len = 65 + 32,
        .params = (const uint8_t *)"\xE5\x31\xCB\xAB\x8A\x48\x35\x3D\xCA\xF9\xA3\x4D\xFC\x6B\x8C\x6C"
                                   "\xE8\x8B\x99\xB9\x28\xD2\x7F\x62\xA0\xAB\x7F\xB4\x28\xDC\x8B\x42",
        .param_len = 32,
        .c = (const uint8_t *)"\x1F\x17\x5F\xD9\xA7\xC5\x09\x70\x34\x41\xEF\x65\x8E\x8B\x62\x3C"
                              "\x34\x91\x37\xD8\x15\x2B\xFC\xF4\xBA\x28\x9B\xE6\x7B\xAC\x43\xC7"
                              "\x14\x19\x3E\x33\x1A\xB8\x8F\x5D\x82\x04\xA0\x58\x86\x47\x4F\xE3"
                              "\xB2\xBD\x97\x0F\x6D\xEC\x2C\x61\x2B\x4D\xA5\xCA\x8C\xB1\x1C\xB7",
        .c_size = 64,
        .m = (const uint8_t *)"sample",
        .m_size = 6,
        .public_key_vec = 0,
        .siggen_sigver_test = 1,
    }
};
const uint8_t g_sm2_default_id[16]
    = { 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38 };

static uint32_t local_import_sm2_key(ehsm_ctx_st *ctx, const struct akcipher_testvec *vect, uint32_t *handle)
{
    uint32_t ret;
    uint8_t *key_space = ehsm_demo_get_buffer(5);
    ehsm_key_format_st *key = (ehsm_key_format_st *)key_space;
    key->key_type = EHSM_KEY_TYPE_SM2;
    key->privilege = EHSM_KEY_PRIV_SIGN | EHSM_KEY_PRIV_REMOVE | EHSM_KEY_PRIV_VERIFY;
    uint32_t key_data_size = 0;

    // 私钥签名，公钥验签； 公钥加密，私钥解密 全部传入
    key->part_info = EHSM_KEY_PART_KEY_PAIR;
    key->pub_key_size = 65; // sm2 算法公钥长度默认65，头部0x04 表示是x,y 双坐标
    memcpy(&key->key_value[0], vect->key, key->pub_key_size);
    key->priv_key_size = 32; // sm2 算法私钥长度默认 32；
    memcpy(&key->key_value[key->pub_key_size], &vect->key[65], key->priv_key_size);

    key_data_size = sizeof(ehsm_key_format_st) + key->pub_key_size + key->priv_key_size;

    ret = ehsm_km_import_key(ctx, 0xFFFFFFFF, 0xFFFFFFFF, key, key_data_size, NULL, 0, handle);
    if (ret != EHSM_OK) {
        ehsm_port_printf("ret =(%dU)\n", ret);
        return ret;
    }

    return ret;
}

void u8big_to_u32little_256bits(const uint8_t *in, uint32_t *out)
{
    out[7] = ((uint32_t)in[3]) | (((uint32_t)in[2]) << 8u) | (((uint32_t)in[1]) << 16u) | (((uint32_t)in[0]) << 24u);
    out[6] = ((uint32_t)in[7]) | (((uint32_t)in[6]) << 8u) | (((uint32_t)in[5]) << 16u) | (((uint32_t)in[4]) << 24u);
    out[5] = ((uint32_t)in[11]) | (((uint32_t)in[10]) << 8u) | (((uint32_t)in[9]) << 16u) | (((uint32_t)in[8]) << 24u);
    out[4]
        = ((uint32_t)in[15]) | (((uint32_t)in[14]) << 8u) | (((uint32_t)in[13]) << 16u) | (((uint32_t)in[12]) << 24u);
    out[3]
        = ((uint32_t)in[19]) | (((uint32_t)in[18]) << 8u) | (((uint32_t)in[17]) << 16u) | (((uint32_t)in[16]) << 24u);
    out[2]
        = ((uint32_t)in[23]) | (((uint32_t)in[22]) << 8u) | (((uint32_t)in[21]) << 16u) | (((uint32_t)in[20]) << 24u);
    out[1]
        = ((uint32_t)in[27]) | (((uint32_t)in[26]) << 8u) | (((uint32_t)in[25]) << 16u) | (((uint32_t)in[24]) << 24u);
    out[0]
        = ((uint32_t)in[31]) | (((uint32_t)in[30]) << 8u) | (((uint32_t)in[29]) << 16u) | (((uint32_t)in[28]) << 24u);
}

#define SM2_WORD_LEN (8U)
#define SM2_BYTE_LEN (32U)

static const uint32_t sm2p256v1_p[8]
    = { 0xFFFFFFFFu, 0xFFFFFFFFu, 0x00000000u, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFEu };
static const uint32_t sm2p256v1_p_h[8]
    = { 0x00000003u, 0x00000002u, 0xFFFFFFFFu, 0x00000002u, 0x00000001u, 0x00000001u, 0x00000002u, 0x00000004u };
static const uint32_t sm2p256v1_a[8]
    = { 0xFFFFFFFCu, 0xFFFFFFFFu, 0x00000000u, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFEu };
static const uint32_t sm2p256v1_b[8]
    = { 0x4D940E93u, 0xDDBCBD41u, 0x15AB8F92u, 0xF39789F5u, 0xCF6509A7u, 0x4D5A9E4Bu, 0x9D9F5E34u, 0x28E9FA9Eu };
static const uint32_t sm2p256v1_Gx[8]
    = { 0x334C74C7u, 0x715A4589u, 0xF2660BE1u, 0x8FE30BBFu, 0x6A39C994u, 0x5F990446u, 0x1F198119u, 0x32C4AE2Cu };
static const uint32_t sm2p256v1_Gy[8]
    = { 0x2139F0A0u, 0x02DF32E5u, 0xC62A4740u, 0xD0A9877Cu, 0x6B692153u, 0x59BDCEE3u, 0xF4F6779Cu, 0xBC3736A2u };
static const uint32_t sm2p256v1_n[8]
    = { 0x39D54123u, 0x53BBF409u, 0x21C6052Bu, 0x7203DF6Bu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFEu };
static const uint32_t sm2p256v1_n_h[8]
    = { 0x7C114F20u, 0x901192AFu, 0xDE6FA2FAu, 0x3464504Au, 0x3AFFE0D4u, 0x620FC84Cu, 0xA22B3D3Bu, 0x1EB5E412u };

//[2^128]G, for [k]G of high speed
static const uint32_t sm2p256v1_2_128_G_x[8]
    = { 0xD13A42EDu, 0xEAE3D9A9u, 0x484E1B38u, 0x2B2308F6u, 0x88C21F3Au, 0x3DB7B248u, 0x74D55DA9u, 0xB692E5B5u };
static const uint32_t sm2p256v1_2_128_G_y[8]
    = { 0xE295E5ABu, 0xD186469Du, 0x73438E6Du, 0xDB61AC17u, 0x544926F9u, 0x5A924F85u, 0x0F3FB613u, 0xA175051Bu };

// SM2 para (n-1), for private key checking
const uint32_t g_sm2p256v1_n_minus_1[8]
    = { 0x39D54122u, 0x53BBF409u, 0x21C6052Bu, 0x7203DF6Bu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFEu };

// eccp curve struct
typedef struct {
    uint32_t eccp_p_bitLen; // bit length of prime p
    uint32_t eccp_n_bitLen; // bit length of order n
    const uint32_t *eccp_p; // prime p
    const uint32_t *eccp_p_h;
    const uint32_t *eccp_a;
    const uint32_t *eccp_b;
    const uint32_t *eccp_Gx;
    const uint32_t *eccp_Gy;
    const uint32_t *eccp_n; // order of curve or point(Gx,Gy)
    const uint32_t *eccp_n_h;
    const uint32_t *eccp_half_Gx;
    const uint32_t *eccp_half_Gy;
} eccp_curve_st;

const eccp_curve_st sm2_curve[1] = { {
    256u,
    256u,
    sm2p256v1_p,
    sm2p256v1_p_h,
    sm2p256v1_a,
    sm2p256v1_b,
    sm2p256v1_Gx,
    sm2p256v1_Gy,
    sm2p256v1_n,
    sm2p256v1_n_h,
    sm2p256v1_2_128_G_x,
    sm2p256v1_2_128_G_y,
} };

uint32_t sm2_getZ_internal(const uint8_t *id, uint32_t id_bytes, const uint8_t pubKey[65], uint8_t Z[32])
{
    uint32_t ret;
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx();
    ehsm_session_st *session = ehsm_demo_get_session(); // 为FW三段式计算提供缓存空间
    uint8_t *tmp_u8 = ehsm_demo_get_buffer(1);
    uint32_t tmp[16];
    uint8_t *tmp_buf = ehsm_demo_get_buffer(2);
    uint8_t *digest = ehsm_demo_get_buffer(3);
    uint32_t digest_size = 0;

    ehsm_port_printf("sm2_getZ_internal\r\n");

    ehsm_ctx_init(ctx, 0, false, NULL);

    ret = ehsm_hash_init(ctx, EHSM_HASH_ALGO_SM3, session);
    ret = wait_work_done(false, NULL, ctx, ret);
    if (ret != EHSM_OK) {
        // error
        ehsm_port_printf("ehsm_hash_init failed, code: 0x%08x\n", ret);
        return ret;
    }

    ehsm_port_printf("sm2_getZ_internal\r\n");

    tmp_u8[0] = (uint8_t)((id_bytes >> 5u) & 0xFFu);
    tmp_u8[1] = (uint8_t)((id_bytes << 3u) & 0xFFu);
    ret = ehsm_hash_update(ctx, (uint8_t *)tmp_u8, 2u);
    ret = wait_work_done(false, NULL, ctx, ret);
    if (ret != EHSM_OK) {
        // error
        ehsm_port_printf("ehsm_hash_update failed, code: 0x%08x\n", ret);
        return ret;
    }

    memcpy(tmp, id, id_bytes);
    ret = ehsm_hash_update(ctx, (uint8_t *)tmp, id_bytes);
    ret = wait_work_done(false, NULL, ctx, ret);
    if (ret != EHSM_OK) {
        // error
        ehsm_port_printf("ehsm_hash_update failed, code: 0x%08x\n", ret);
        return ret;
    }

    u8big_to_u32little_256bits((const uint8_t *)sm2_curve->eccp_a, tmp);
    u8big_to_u32little_256bits((const uint8_t *)sm2_curve->eccp_b, &tmp[SM2_WORD_LEN]);
    memcpy(tmp_buf, tmp, SM2_BYTE_LEN << 1);
    ret = ehsm_hash_update(ctx, (uint8_t *)tmp_buf, SM2_BYTE_LEN << 1);
    ret = wait_work_done(false, NULL, ctx, ret);
    if (ret != EHSM_OK) {
        // error
        ehsm_port_printf("ehsm_hash_update failed, code: 0x%08x\n", ret);
        return ret;
    }

    u8big_to_u32little_256bits((const uint8_t *)sm2_curve->eccp_Gx, tmp);
    u8big_to_u32little_256bits((const uint8_t *)sm2_curve->eccp_Gy, &tmp[SM2_WORD_LEN]);
    memcpy(tmp_buf, tmp, SM2_BYTE_LEN << 1);
    ret = ehsm_hash_update(ctx, (uint8_t *)tmp_buf, SM2_BYTE_LEN << 1);
    ret = wait_work_done(false, NULL, ctx, ret);
    if (ret != EHSM_OK) {
        // error
        ehsm_port_printf("ehsm_hash_update failed, code: 0x%08x\n", ret);
        return ret;
    }

    memcpy(tmp, &pubKey[1u], SM2_BYTE_LEN << 1);
    memcpy(tmp_buf, tmp, SM2_BYTE_LEN << 1);
    ret = ehsm_hash_update(ctx, (uint8_t *)tmp_buf, SM2_BYTE_LEN << 1);
    ret = wait_work_done(false, NULL, ctx, ret);
    if (ret != EHSM_OK) {
        // error
        ehsm_port_printf("ehsm_hash_update failed, code: 0x%08x\n", ret);
        return ret;
    }

    digest_size = 1024;
    ret = ehsm_hash_finish(ctx, digest, &digest_size);
    ret = wait_work_done(false, NULL, ctx, ret);
    if (ret != EHSM_OK) {
        // error
        ehsm_port_printf("ehsm_hash_final failed, code: 0x%08x\n", ret);
        return ret;
    } else {
        memcpy(Z, digest, 32);
    }

    return ret;
}

uint32_t sm2_get_E(const struct akcipher_testvec *vect, const uint8_t *msg, uint8_t msg_sz, uint8_t *E)
{
    uint32_t ret;
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx();
    ehsm_session_st *session = ehsm_demo_get_session(); // 为FW三段式计算提供缓存空间
    uint8_t *tmp = ehsm_demo_get_buffer(2);
    uint8_t *digest = ehsm_demo_get_buffer(3);
    uint8_t *sm2_z = ehsm_demo_get_buffer(3);
    uint32_t digest_size = 1024;

    ehsm_port_printf("sm2_getZ_internal\r\n");
    ret = sm2_getZ_internal(g_sm2_default_id, sizeof(g_sm2_default_id), vect->key, sm2_z);
    if (ret != EHSM_OK) {
        // error
        ehsm_port_printf("sm2_getZ_internal failed, code: 0x%08x\n", ret);
        return ret;
    }

    ehsm_ctx_init(ctx, 0, false, NULL);

    ret = ehsm_hash_init(ctx, EHSM_HASH_ALGO_SM3, session);
    ret = wait_work_done(false, NULL, ctx, ret);
    if (ret != EHSM_OK) {
        // error
        ehsm_port_printf("ehsm_hash_init failed, code: 0x%08x\n", ret);
        return ret;
    }

    memcpy(tmp, sm2_z, 32);
    ret = ehsm_hash_update(ctx, tmp, 32);
    ret = wait_work_done(false, NULL, ctx, ret);
    if (ret != EHSM_OK) {
        // error
        ehsm_port_printf("ehsm_hash_update failed, code: 0x%08x\n", ret);
        return ret;
    }

    memcpy(tmp, msg, msg_sz);
    ret = ehsm_hash_update(ctx, tmp, msg_sz);
    ret = wait_work_done(false, NULL, ctx, ret);
    if (ret != EHSM_OK) {
        // error
        ehsm_port_printf("ehsm_hash_update failed, code: 0x%08x\n", ret);
        return ret;
    }

    ret = ehsm_hash_finish(ctx, digest, &digest_size);
    ret = wait_work_done(false, NULL, ctx, ret);
    if (ret != EHSM_OK) {
        // error
        ehsm_port_printf("ehsm_hash_finish failed, code: 0x%08x\n", ret);
        return ret;
    } else {
        memcpy(E, digest, 32);
    }

    return ret;
}

static uint32_t ehsm_demo_test_sm2_sign_vrf(bool_t async, bool_t callback, bool_t *verify_result)
{
    uint32_t ret;
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx();
    uint32_t key_handle = 0xFFFFFFFFU;
    uint8_t *msg = ehsm_demo_get_buffer(0);
    uint8_t *E = ehsm_demo_get_buffer(1);
    uint8_t *sig = ehsm_demo_get_buffer(2);
    uint32_t sig_size = 64;
    const struct akcipher_testvec *vect = g_sm2_sigverify_tv_template;

    ehsm_port_printf("ehsm_demo_test_rsa signature/verify, async: %d, callback: %d\n", (int)async, (int)callback);

    ehsm_ctx_init(ctx, 0, /*async=*/async, callback ? ehsm_demo_callback : NULL);

    ret = local_import_sm2_key(ctx, vect, &key_handle);
    ret = wait_work_done(async, callback, ctx, ret);
    if (ret != EHSM_OK) {
        // error
        ehsm_port_printf("local_import_rsa_key failed, code: 0x%08x\n", ret);
        return ret;
    }

    memcpy(msg, vect->m, vect->m_size);
    ret = ehsm_sm2_sign_onepass_gen(ctx, key_handle, msg, vect->m_size, sig, sig_size);
    ret = wait_work_done(async, callback, ctx, ret);

    if (ret != EHSM_OK) {
        // error
        ehsm_port_printf("ehsm_hash_init failed, code: 0x%08x\n", ret);
        ehsm_km_remove_key(ctx, key_handle);
        return ret;
    }

    *verify_result = false;
    memcpy(msg, vect->m, vect->m_size);
    ret = ehsm_sm2_sign_onepass_verify(ctx, key_handle, msg, vect->m_size, sig, sig_size, verify_result);
    ret = wait_work_done(async, callback, ctx, ret);
    if (ret != EHSM_OK) {
        ehsm_port_printf("ehsm_hash_update failed, code: 0x%08x\n", ret);
        return ret;
    }

    if (*verify_result != true) {
        ehsm_port_printf("verify_result verify failed, code: 0x%08x\n", ret);
        return ret;
    }

    ret = sm2_get_E(vect, vect->m, (uint8_t)vect->m_size, E);
    if (ret != EHSM_OK) {
        ehsm_port_printf("sm2_get_E failed, code: 0x%08x\n", ret);
        return ret;
    }

    sig_size = 64;
    ret = ehsm_sm2_sign_onepass_gen_with_digest(ctx, key_handle, E, 32, sig, sig_size);
    ret = wait_work_done(async, callback, ctx, ret);
    if (ret != EHSM_OK) {
        // error
        ehsm_port_printf("ehsm_hash_init failed, code: 0x%08x\n", ret);
        ehsm_km_remove_key(ctx, key_handle);
        return ret;
    }

    *verify_result = false;
    ret = ehsm_sm2_sign_onepass_verify_with_digest(ctx, key_handle, E, 32, sig, sig_size, verify_result);
    ret = wait_work_done(async, callback, ctx, ret);
    if (ret != EHSM_OK) {
        ehsm_port_printf("ehsm_hash_update failed, code: 0x%08x\n", ret);
        return ret;
    }

    if (*verify_result != true) {
        ehsm_port_printf("verify_result verify failed, code: 0x%08x\n", ret);
        return ret;
    }

    return ret;
}

typedef enum {
    ECC_CURVE_BRAINPOOLP160R1,
    ECC_CURVE_BRAINPOOLP192R1,
    ECC_CURVE_BRAINPOOLP224R1,
    ECC_CURVE_BRAINPOOLP256R1,
    ECC_CURVE_BRAINPOOLP320R1,
    ECC_CURVE_BRAINPOOLP384R1,
    ECC_CURVE_BRAINPOOLP512R1,
    ECC_CURVE_SECP192R1,
    ECC_CURVE_SECP224R1,
    ECC_CURVE_SECP256R1,
    ECC_CURVE_SECP384R1,
    ECC_CURVE_SECP521R1,
    ECC_CURVE_SM2,
    ECC_CURVE_ED25519,
} ecc_curve_e;

static const struct akcipher_testvec g_ecdsa_nist_p256[] = { {
    .key = (const uint8_t *)"\xC9\xAF\xA9\xD8\x45\xBA\x75\x16"
                            "\x6B\x5C\x21\x57\x67\xB1\xD6\x93"
                            "\x4E\x50\xC3\xDB\x36\xE8\x9B\x12"
                            "\x7B\x8A\x62\x2B\x12\x0F\x67\x21"
                            "\x60\xFE\xD4\xBA\x25\x5A\x9D\x31"
                            "\xC9\x61\xEB\x74\xC6\x35\x6D\x68"
                            "\xC0\x49\xB8\x92\x3B\x61\xFA\x6C"
                            "\xE6\x69\x62\x2E\x60\xF2\x9F\xB6"
                            "\x79\x03\xFE\x10\x08\xB8\xBC\x99"
                            "\xA4\x1A\xE9\xE9\x56\x28\xBC\x64"
                            "\xF2\xF1\xB2\x0C\x2D\x7E\x9F\x51"
                            "\x77\xA3\xC2\x94\xD4\x46\x22\x99",
    .params = (const uint8_t *)"\x88\x29\x05\xF1\x22\x7F\xD6\x20"
                               "\xFB\xF2\xAB\xF2\x12\x44\xF0\xBA"
                               "\x83\xD0\xDC\x3A\x91\x03\xDB\xBE"
                               "\xE4\x3A\x1F\xB8\x58\x10\x9D\xB4",
    .c = (const uint8_t *)"\x61\x34\x0C\x88\xC3\xAA\xEB\xEB"
                          "\x4F\x6D\x66\x7F\x67\x2C\xA9\x75"
                          "\x9A\x6C\xCA\xA9\xFA\x88\x11\x31"
                          "\x30\x39\xEE\x4A\x35\x47\x1D\x32"
                          "\x6D\x7F\x14\x7D\xAC\x08\x94\x41"
                          "\xBB\x2E\x2F\xE8\xF7\xA3\xFA\x26"
                          "\x4B\x9C\x47\x50\x98\xFD\xCF\x6E"
                          "\x00\xD7\xC9\x96\xE1\xB8\xB7\xEB",
    .m = (const uint8_t *)"sample",
    .m_size = 6,
    .hash_alg = EHSM_HASH_ALGO_SHA1,
    .public_key_vec = 0,
    .siggen_sigver_test = 1,
} };

static uint8_t get_curve_alg_id(ecc_curve_e curve_id)
{
    uint8_t key_alg_id = 0;
    switch (curve_id) {
    case ECC_CURVE_SM2:
        key_alg_id = EHSM_KEY_TYPE_SM2;
        break;
    case ECC_CURVE_BRAINPOOLP160R1:
        key_alg_id = EHSM_KEY_TYPE_ECC_BRAINPOOLP_160R1;
        break;
    case ECC_CURVE_BRAINPOOLP192R1:
        key_alg_id = EHSM_KEY_TYPE_ECC_BRAINPOOLP_192R1;
        break;
    case ECC_CURVE_BRAINPOOLP224R1:
        key_alg_id = EHSM_KEY_TYPE_ECC_BRAINPOOLP_224R1;
        break;
    case ECC_CURVE_BRAINPOOLP256R1:
        key_alg_id = EHSM_KEY_TYPE_ECC_BRAINPOOLP_256R1;
        break;
    case ECC_CURVE_BRAINPOOLP320R1:
        key_alg_id = EHSM_KEY_TYPE_ECC_BRAINPOOLP_320R1;
        break;
    case ECC_CURVE_BRAINPOOLP384R1:
        key_alg_id = EHSM_KEY_TYPE_ECC_BRAINPOOLP_384R1;
        break;
    case ECC_CURVE_BRAINPOOLP512R1:
        key_alg_id = EHSM_KEY_TYPE_ECC_BRAINPOOLP_512R1;
        break;
    case ECC_CURVE_SECP192R1:
        key_alg_id = EHSM_KEY_TYPE_ECC_SECP_192R1;
        break;
    case ECC_CURVE_SECP224R1:
        key_alg_id = EHSM_KEY_TYPE_ECC_SECP_224R1;
        break;
    case ECC_CURVE_SECP256R1:
        key_alg_id = EHSM_KEY_TYPE_ECC_SECP_256R1;
        break;
    case ECC_CURVE_SECP384R1:
        key_alg_id = EHSM_KEY_TYPE_ECC_SECP_384R1;
        break;
    case ECC_CURVE_SECP521R1:
        key_alg_id = EHSM_KEY_TYPE_ECC_SECP_521R1;
        break;
    case ECC_CURVE_ED25519:
        key_alg_id = EHSM_KEY_TYPE_ED25519;
        break;
    default:
        key_alg_id = 0xFFU;
        break;
    }
    return key_alg_id;
}

static uint8_t get_curve_public_key_size(ecc_curve_e curve_type)
{
    uint8_t key_size = 0;
    switch (curve_type) {
    case ECC_CURVE_SM2:
        key_size = 65;
        break;
    case ECC_CURVE_BRAINPOOLP160R1:
        key_size = 40;
        break;
    case ECC_CURVE_BRAINPOOLP192R1:
    case ECC_CURVE_SECP192R1:
        key_size = 48;
        break;
    case ECC_CURVE_BRAINPOOLP224R1:
    case ECC_CURVE_SECP224R1:
        key_size = 56;
        break;
    case ECC_CURVE_BRAINPOOLP256R1:
    case ECC_CURVE_SECP256R1:
        key_size = 64;
        break;
    case ECC_CURVE_BRAINPOOLP320R1:
        key_size = 80;
        break;
    case ECC_CURVE_BRAINPOOLP384R1:
    case ECC_CURVE_SECP384R1:
        key_size = 96;
        break;
    case ECC_CURVE_BRAINPOOLP512R1:
        key_size = 128;
        break;
    case ECC_CURVE_SECP521R1:
        key_size = 132;
        break;
    case ECC_CURVE_ED25519:
        key_size = 32;
        break;
    default:
        key_size = 0U;
        break;
    }
    return key_size;
}
static uint8_t get_curve_private_key_size(ecc_curve_e curve_type)
{
    uint8_t key_size = 0;
    switch (curve_type) {
    case ECC_CURVE_SM2:
    case ECC_CURVE_ED25519:
        key_size = 32;
        break;
    case ECC_CURVE_BRAINPOOLP160R1:
        key_size = 20;
        break;
    case ECC_CURVE_BRAINPOOLP192R1:
    case ECC_CURVE_SECP192R1:
        key_size = 24;
        break;
    case ECC_CURVE_BRAINPOOLP224R1:
    case ECC_CURVE_SECP224R1:
        key_size = 28;
        break;
    case ECC_CURVE_BRAINPOOLP256R1:
    case ECC_CURVE_SECP256R1:
        key_size = 32;
        break;
    case ECC_CURVE_BRAINPOOLP320R1:
        key_size = 40;
        break;
    case ECC_CURVE_BRAINPOOLP384R1:
    case ECC_CURVE_SECP384R1:
        key_size = 48;
        break;
    case ECC_CURVE_BRAINPOOLP512R1:
        key_size = 64;
        break;
    case ECC_CURVE_SECP521R1:
        key_size = 66;
        break;
    default:
        key_size = 0U;
        break;
    }
    return key_size;
}

static uint32_t local_import_ecc_key(
    ehsm_ctx_st *ctx, const struct akcipher_testvec *vect, ecc_curve_e curve_type, uint32_t *handle)
{
    uint32_t ret;
    uint8_t *key_space = ehsm_demo_get_buffer(5);
    ehsm_key_format_st *key = (ehsm_key_format_st *)key_space;
    key->key_type = get_curve_alg_id(curve_type);
    key->privilege = EHSM_KEY_PRIV_SIGN | EHSM_KEY_PRIV_REMOVE | EHSM_KEY_PRIV_VERIFY;
    uint32_t key_data_size = 0;

    // 私钥签名，公钥验签； 公钥加密，私钥解密 全部传入
    key->part_info = EHSM_KEY_PART_KEY_PAIR;
    // vect 前面为私钥，后面为公钥
    key->pub_key_size = get_curve_public_key_size(curve_type);
    key->priv_key_size = get_curve_private_key_size(curve_type);
    memcpy(&key->key_value[0], &vect->key[key->priv_key_size], key->pub_key_size); // ecc 前面为公钥
    memcpy(&key->key_value[key->pub_key_size], vect->key, key->priv_key_size);
    key_data_size = sizeof(ehsm_key_format_st) + key->pub_key_size + key->priv_key_size;

    ret = ehsm_km_import_key(ctx, 0xFFFFFFFF, 0xFFFFFFFF, key, key_data_size, NULL, 0, handle);
    if (ret != EHSM_OK) {
        ehsm_port_printf("ret =(%dU)\n", ret);
        return ret;
    }

    return ret;
}

uint32_t ehsm_cacl_hash(ehsm_hash_algo_e alg, uint8_t *msg, uint32_t msg_sz, uint8_t *digest, uint32_t *digest_sz)
{
    uint32_t ret;
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx();

    ehsm_ctx_init(ctx, 0, false, NULL);

    ret = ehsm_hash_onepass(ctx, alg, msg, msg_sz, digest, digest_sz);
    ret = wait_work_done(false, NULL, ctx, ret);
    if (ret != EHSM_OK) {
        ehsm_port_printf("ehsm_hash_onepass failed, code: 0x%08x\n", ret);
    }

    return ret;
}

static uint32_t ehsm_demo_test_ecdsa_sign_vrf(bool_t async, bool_t callback, bool_t *verify_result)
{
    uint32_t ret;
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx();
    uint32_t key_handle = 0xFFFFFFFFU;
    uint8_t *msg = ehsm_demo_get_buffer(0);
    uint8_t *digest = ehsm_demo_get_buffer(1);
    uint8_t *sig = ehsm_demo_get_buffer(2);
    uint32_t sig_size = 64;
    uint32_t digest_size = 0U;
    const struct akcipher_testvec *vect = g_ecdsa_nist_p256;

    ehsm_port_printf(
        "ehsm_demo_test_ecdsa_sign_vrf signature/verify, async: %d, callback: %d\n", (int)async, (int)callback);

    ehsm_ctx_init(ctx, 0, /*async=*/async, callback ? ehsm_demo_callback : NULL);

    ret = local_import_ecc_key(ctx, vect, ECC_CURVE_SECP256R1, &key_handle);
    ret = wait_work_done(async, callback, ctx, ret);
    if (ret != EHSM_OK) {
        // error
        ehsm_port_printf("local_import_ecc_key failed, code: 0x%08x\n", ret);
        return ret;
    }

    memcpy(msg, vect->m, vect->m_size);
    ret = ehsm_ecdsa_onepass_gen(ctx, (ehsm_hash_algo_e)vect->hash_alg, key_handle, msg, vect->m_size, sig, &sig_size);
    ret = wait_work_done(async, callback, ctx, ret);

    if (ret != EHSM_OK) {
        // error
        ehsm_port_printf("ehsm_hash_init failed, code: 0x%08x\n", ret);
        ehsm_km_remove_key(ctx, key_handle);
        return ret;
    }

    *verify_result = false;
    memcpy(msg, vect->m, vect->m_size);
    ret = ehsm_ecdsa_onepass_verify(
        ctx, (ehsm_hash_algo_e)vect->hash_alg, key_handle, msg, vect->m_size, sig, sig_size, verify_result);
    ret = wait_work_done(async, callback, ctx, ret);
    if (ret != EHSM_OK) {
        ehsm_port_printf("ehsm_hash_update failed, code: 0x%08x\n", ret);
        return ret;
    }

    if (*verify_result != true) {
        ehsm_port_printf("verify_result failed, code: 0x%08x\n", ret);
        return ret;
    }

    digest_size = 1024;
    ret = ehsm_cacl_hash((ehsm_hash_algo_e)vect->hash_alg, msg, vect->m_size, digest, &digest_size);
    if (ret != EHSM_OK) {
        ehsm_port_printf("ehsm_hash_update failed, code: 0x%08x\n", ret);
        return ret;
    }

    ret = ehsm_ecdsa_onepass_gen_with_digest(ctx, EHSM_HASH_ALGO_SHA256, key_handle, digest, 32, sig, &sig_size);
    ret = wait_work_done(async, callback, ctx, ret);
    if (ret != EHSM_OK) {
        // error
        ehsm_port_printf("ehsm_hash_init failed, code: 0x%08x\n", ret);
        ehsm_km_remove_key(ctx, key_handle);
        return ret;
    }

    *verify_result = 0;
    ret = ehsm_ecdsa_onepass_verify_with_digest(
        ctx, EHSM_HASH_ALGO_SHA256, key_handle, digest, 32, sig, sig_size, verify_result);
    ret = wait_work_done(async, callback, ctx, ret);
    if (ret != EHSM_OK) {
        ehsm_port_printf("ehsm_hash_update failed, code: 0x%08x\n", ret);
        return ret;
    }

    if (*verify_result != true) {
        ehsm_port_printf("verify_result failed, code: 0x%08x\n", ret);
        return ret;
    }

    return ret;
}

uint32_t ehsm_demo_pke_sign_verify_by_msg_digest_test(void)
{
    uint32_t ret = EHSM_OK;
    bool_t verify_result = false;

    ehsm_port_printf("\r\n\r\n==================== eHSM demo for RSA signature starts. ==================== "
                     "\r\n\r\n");
    ret = ehsm_demo_test_rsa_sign_vrf(false, false, &verify_result);
    ret = demo_check_val("The execution of finish API:", EHSM_OK, ret);
    if (EHSM_OK == ret) {
        /** eHSM FW 返回成功，检查签名验证结果 */
        ret = demo_check_val("Signature verification:", true, verify_result);
    }
    ehsm_port_printf("\r\n\r\n==================== eHSM demo for RSA signature end. ==================== "
                     "\r\n\r\n");

    ehsm_port_printf("\r\n\r\n==================== eHSM demo for sm2 signature starts. ==================== "
                     "\r\n\r\n");
    ret = ehsm_demo_test_sm2_sign_vrf(false, false, &verify_result);
    ret = demo_check_val("The execution of finish API:", EHSM_OK, ret);
    if (EHSM_OK == ret) {
        /** eHSM FW 返回成功，检查签名验证结果 */
        ret = demo_check_val("Signature verification:", true, verify_result);
    }
    ehsm_port_printf("\r\n\r\n==================== eHSM demo for sm2 signature end. ==================== "
                     "\r\n\r\n");

    ehsm_port_printf("\r\n\r\n==================== eHSM demo for ecdsa signature starts. ==================== "
                     "\r\n\r\n");
    ret = ehsm_demo_test_ecdsa_sign_vrf(false, false, &verify_result);
    ret = demo_check_val("The execution of finish API:", EHSM_OK, ret);
    if (EHSM_OK == ret) {
        /** eHSM FW 返回成功，检查签名验证结果 */
        ret = demo_check_val("Signature verification:", true, verify_result);
    }
    ehsm_port_printf("\r\n\r\n==================== eHSM demo for ecdsa signature end. ==================== "
                     "\r\n\r\n");

    return ret;
}
