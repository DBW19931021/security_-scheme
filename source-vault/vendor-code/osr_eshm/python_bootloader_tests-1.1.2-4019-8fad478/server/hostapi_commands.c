#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "test_types.h"

#include "ehsmdrv/basic/api.h"
#include "ehsmdrv/basic/bl_api.h"
#include "ehsmdrv/basic/test_api.h"

typedef struct {
    uint32_t ctx_addr;
    uint32_t src_data;
    uint32_t ehsm_dest_addr;
    uint32_t size;
} write_otp_cmd_st;

typedef struct {
    uint32_t ret;
} write_otp_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t type;
    uint32_t soc_dbg_bitmap;
} close_debug_cmd_st;

typedef struct {
    uint32_t ret;
} close_debug_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t challenge_type;
    uint32_t algo;
    uint32_t sig;
    uint32_t sig_size;
    uint32_t pub_key;
    uint32_t pub_key_size;
    uint32_t soc_dbg_bitmap;
} debug_auth_cmd_st;

typedef struct {
    uint32_t ret;
} debug_auth_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t buf;
    uint32_t ehsm_src_addr;
    uint32_t size;
} read_otp_cmd_st;

typedef struct {
    uint32_t ret;
} read_otp_rsp_st;

typedef struct {
    uint32_t ctx;
    uint32_t hash_algo;
    uint32_t msg_addr;
    uint32_t msg_size;
    uint32_t digest_addr;
    uint32_t digest_size;
} hash_cmd_st;

typedef struct {
    uint32_t ret;
    uint32_t digest_size;
} hash_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t buf;
    uint32_t ehsm_src_addr;
    uint32_t size;
} read_reg_cmd_st;

typedef struct {
    uint32_t ret;
} read_reg_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t src_data;
    uint32_t ehsm_dest_addr;
    uint32_t size;
} write_reg_cmd_st;

typedef struct {
    uint32_t ret;
} write_reg_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t key_level;
    uint32_t input_data;
    uint32_t size;
    uint32_t key_out;
} encrypt_key_cmd_st;

typedef struct {
    uint32_t ret;
} encrypt_key_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t type;
    uint32_t arg;
    uint32_t auth_data;
    uint32_t out_addr;
} fw_auth_cmd_st;

typedef struct {
    uint32_t ret;
} fw_auth_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t key_level;
    uint32_t key_type;
    uint32_t key_out;
} random_key_cmd_st;

typedef struct {
    uint32_t ret;
} random_key_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t challenge_type;
    uint32_t output;
} challenge_cmd_st;

typedef struct {
    uint32_t ret;
} challenge_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t algo;
    uint32_t mode;
    uint32_t key_handle;
    uint32_t nonce;
    uint32_t nonce_size;
    uint32_t aad;
    uint32_t aad_size;
    uint32_t input;
    uint32_t input_size;
    uint32_t output;
    uint32_t tag;
    uint32_t tag_size;
} aead_onepass_enc_cmd_st;

typedef struct {
    uint32_t ret;
} aead_onepass_enc_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t algo;
    uint32_t mode;
    uint32_t key_handle;
    uint32_t nonce;
    uint32_t nonce_size;
    uint32_t aad;
    uint32_t aad_size;
    uint32_t input;
    uint32_t input_size;
    uint32_t output;
    uint32_t tag;
    uint32_t tag_size;
} aead_onepass_dec_cmd_st;

typedef struct {
    uint32_t ret;
    uint32_t verify_result;
} aead_onepass_dec_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t algo;
    uint32_t mode;
    uint32_t key_handle;
    uint32_t enc;
    uint32_t nonce;
    uint32_t nonce_size;
    uint32_t aad;
    uint32_t aad_size;
    uint32_t data_size;
    uint32_t tag_size;
    uint32_t session;
} aead_init_cmd_st;

typedef struct {
    uint32_t ret;
} aead_init_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t input;
    uint32_t input_size;
    uint32_t output;
} aead_update_cmd_st;

typedef struct {
    uint32_t ret;
} aead_update_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t input;
    uint32_t input_size;
    uint32_t output;
    uint32_t tag;
} aead_finish_enc_cmd_st;

typedef struct {
    uint32_t ret;
} aead_finish_enc_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t input;
    uint32_t input_size;
    uint32_t output;
    uint32_t tag;
} aead_finish_dec_cmd_st;

typedef struct {
    uint32_t ret;
    uint32_t verify_result;
} aead_finish_dec_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t algo;
    uint32_t mode;
    uint32_t key_addr;
    uint32_t key_size;
    uint32_t nonce_addr;
    uint32_t nonce_size;
    uint32_t aad_addr;
    uint32_t aad_size;
    uint32_t input_addr;
    uint32_t input_size;
    uint32_t output_addr;
    uint32_t tag_addr;
    uint32_t tag_size;
} aead_onepass_enc_with_plain_key_cmd_st;

typedef struct {
    uint32_t ret;
} aead_onepass_enc_with_plain_key_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t algo;
    uint32_t mode;
    uint32_t key_addr;
    uint32_t key_size;
    uint32_t nonce_addr;
    uint32_t nonce_size;
    uint32_t aad_addr;
    uint32_t aad_size;
    uint32_t input_addr;
    uint32_t input_size;
    uint32_t output_addr;
    uint32_t tag_addr;
    uint32_t tag_size;
} aead_onepass_dec_with_plain_key_cmd_st;

typedef struct {
    uint32_t ret;
    uint32_t verify_result;
} aead_onepass_dec_with_plain_key_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t algo;
    uint32_t mode;
    uint32_t key_addr;
    uint32_t key_size;
    uint32_t enc;
    uint32_t nonce_addr;
    uint32_t nonce_size;
    uint32_t aad_addr;
    uint32_t aad_size;
    uint32_t data_size;
    uint32_t tag_size;
    uint32_t session;
} aead_init_with_plain_key_cmd_st;

typedef struct {
    uint32_t ret;
} aead_init_with_plain_key_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t key_addr;
    uint32_t key_size;
    uint32_t nonce_addr;
    uint32_t nonce_size;
    uint32_t aad_addr;
    uint32_t aad_size;
    uint32_t constant;
    uint32_t input_addr;
    uint32_t input_size;
    uint32_t output_addr;
    uint32_t tag_addr;
    uint32_t tag_size;
} chacha_onepass_enc_with_plain_key_cmd_st;

typedef struct {
    uint32_t ret;
} chacha_onepass_enc_with_plain_key_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t key_addr;
    uint32_t key_size;
    uint32_t nonce_addr;
    uint32_t nonce_size;
    uint32_t aad_addr;
    uint32_t aad_size;
    uint32_t constant;
    uint32_t input_addr;
    uint32_t input_size;
    uint32_t output_addr;
    uint32_t tag_addr;
    uint32_t tag_size;
} chacha_onepass_dec_with_plain_key_cmd_st;

typedef struct {
    uint32_t ret;
    uint32_t verify_result;
} chacha_onepass_dec_with_plain_key_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t key_addr;
    uint32_t key_size;
    uint32_t enc;
    uint32_t nonce_addr;
    uint32_t nonce_size;
    uint32_t aad_addr;
    uint32_t aad_size;
    uint32_t constant;
    uint32_t session;
} chacha_init_with_plain_key_cmd_st;

typedef struct {
    uint32_t ret;
} chacha_init_with_plain_key_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t key_handle;
    uint32_t nonce;
    uint32_t nonce_size;
    uint32_t aad;
    uint32_t aad_size;
    uint32_t constant;
    uint32_t input;
    uint32_t input_size;
    uint32_t output;
    uint32_t tag;
    uint32_t tag_size;
} chacha_onepass_enc_cmd_st;

typedef struct {
    uint32_t ret;
} chacha_onepass_enc_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t key_handle;
    uint32_t nonce;
    uint32_t nonce_size;
    uint32_t aad;
    uint32_t aad_size;
    uint32_t constant;
    uint32_t input;
    uint32_t input_size;
    uint32_t output;
    uint32_t tag;
    uint32_t tag_size;
    uint32_t verify_result;
} chacha_onepass_dec_cmd_st;

typedef struct {
    uint32_t ret;
    uint32_t verify_result;
} chacha_onepass_dec_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t key_handle;
    uint32_t enc;
    uint32_t nonce;
    uint32_t nonce_size;
    uint32_t aad;
    uint32_t aad_size;
    uint32_t constant;
    uint32_t session;
} chacha_init_cmd_st;

typedef struct {
    uint32_t ret;
} chacha_init_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t input;
    uint32_t input_size;
    uint32_t output;
} chacha_update_cmd_st;

typedef struct {
    uint32_t ret;
} chacha_update_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t input;
    uint32_t input_size;
    uint32_t output;
    uint32_t tag;
    uint32_t tag_size;
} chacha_finish_enc_cmd_st;

typedef struct {
    uint32_t ret;
} chacha_finish_enc_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t input;
    uint32_t input_size;
    uint32_t output;
    uint32_t tag;
    uint32_t tag_size;
    uint32_t verify_result;
} chacha_finish_dec_cmd_st;

typedef struct {
    uint32_t ret;
    uint32_t verify_result;
} chacha_finish_dec_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t field;
    uint32_t values;
} control_field_cmd_st;

typedef struct {
    uint32_t ret;
} control_field_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t counter_id;
    uint32_t counter_value;
} counter_cmd_st;

typedef struct {
    uint32_t ret;
    uint32_t counter_id;
    uint32_t counter_value;
} counter_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t mb_ch;
    uint32_t async;
    uint32_t rsp_callback;
} ctx_cmd_st;

typedef struct {
    uint32_t ret;
} ctx_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t key_handle;
    uint32_t msg_addr;
    uint32_t msg_size;
    uint32_t sig_addr;
    uint32_t sig_size;
} sm2_sign_cmd_st;

typedef struct {
    uint32_t ret;
    uint32_t sign_size;
    uint32_t verification;
} sm2_sign_rsp_st;

typedef struct {
    uint32_t ctx;
    uint32_t key_handle;
    uint32_t gen_sig;
    uint32_t session;
} sm2_sign_init_cmd_st;

typedef struct {
    uint32_t ctx;
    uint32_t msg;
    uint32_t msg_size;
} sm2_sign_update_cmd_st;

typedef struct {
    uint32_t ctx;
    uint32_t sig;
    uint32_t sig_size;
} sm2_sign_finish_gen_cmd_st;

typedef struct {
    uint32_t ctx;
    uint32_t sig;
    uint32_t sig_size;
} sm2_sign_finish_verify_cmd_st;

#define DATA_SIZE 2048

uint16_t test_cmd_hostapi_init_lib(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != 1) {
        return RSP_ERR_DATA_LENGTH;
    }
    uint32_t ret = ehsm_driver_init_library((ehsm_drv_mode_e)cmd->val[0]);
    rsp->len = 4;
    memcpy(rsp->val, &ret, 4);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_init_ctx(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != 6) {
        return RSP_ERR_DATA_LENGTH;
    }
    (void)rsp;
    uint32_t ctx_addr = *((uint32_t *)&cmd->val[0]);

    ehsm_ctx_init((ehsm_ctx_st *)ctx_addr, cmd->val[4], cmd->val[5], NULL);
    return RSP_OK;
}

typedef struct {
    uint32_t ctx_addr;
    uint32_t digest;
    uint32_t digest_size;
} hash_finish_cmd_st;

typedef struct {
    uint32_t ret;
    uint32_t digest_size;
} hash_finish_rsp_st;

typedef struct {
    uint32_t ctx;
    uint32_t hash_algo;
    uint32_t session;
} hash_init_cmd_st;

typedef struct {
    uint32_t ret;
} hash_init_rsp_st;

typedef struct {
    uint32_t ctx;
    uint32_t msg;
    uint32_t msg_size;
} hash_update_cmd_st;

typedef struct {
    uint32_t ret;
    uint32_t digest_size;
} hash_onepass_rsp_st;

typedef struct {
    uint32_t ret;
} hash_update_rsp_st;

typedef struct {
    uint32_t ctx;
    uint32_t key_handle;
} remove_key_cmd_st;

typedef struct {
    uint32_t ret;
} remove_key_rsp_st;

typedef struct {
    uint32_t ctx_addr;   // 对应原参数 ctx
    uint32_t algo;       // 对应原参数 algo，mac算法
    uint32_t mode;       // 对应原参数 mode，mac模式
    uint32_t key_handle; // 对应原参数 key_handle，密钥句柄
    uint32_t iv_addr;    // 对应原参数 iv，初始化向量指针地址
    uint32_t iv_size;    // 对应原参数 iv_size，初始化向量长度
    uint32_t msg_addr;   // 对应原参数 msg，消息数据指针地址
    uint32_t msg_size;   // 对应原参数 msg_size，消息长度
    uint32_t mac_addr;   // 对应原参数 mac，mac数据指针地址
    uint32_t mac_size;   // 对应原参数 mac_size，mac长度
} mac_onepass_gen_cmd_st;

/* mac_onepass_gen 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
} mac_onepass_gen_rsp_st;

typedef struct {
    uint32_t ctx_addr;   // 对应原参数 ctx
    uint32_t algo;       // 对应原参数 algo，mac算法
    uint32_t mode;       // 对应原参数 mode，mac模式
    uint32_t key_handle; // 对应原参数 key_handle，密钥句柄
    uint32_t iv_addr;    // 对应原参数 iv，初始化向量指针地址
    uint32_t iv_size;    // 对应原参数 iv_size，初始化向量长度
    uint32_t msg_addr;   // 对应原参数 msg，消息数据指针地址
    uint32_t msg_size;   // 对应原参数 msg_size，消息长度
    uint32_t mac_addr;   // 对应原参数 mac，mac数据指针地址
    uint32_t mac_size;   // 对应原参数 mac_size，mac长度
} mac_onepass_verify_cmd_st;

/* mac_onepass_verify 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
    uint32_t verify_result;
} mac_onepass_verify_rsp_st;

typedef struct {
    uint32_t ctx_addr;     // 对应原参数 ctx
    uint32_t algo;         // 对应原参数 algo，mac算法
    uint32_t mode;         // 对应原参数 mode，mac模式
    uint32_t key_handle;   // 对应原参数 key_handle，密钥句柄
    uint32_t gen_mac;      // 对应原参数 gen_mac，生成mac或验证mac
    uint32_t iv_addr;      // 对应原参数 iv，初始化向量指针地址
    uint32_t iv_size;      // 对应原参数 iv_size，初始化向量长度
    uint32_t mac_size;     // 对应原参数 mac_size，mac长度
    uint32_t session_addr; // 对应原参数 session，会话结构体指针地址
} mac_init_cmd_st;

/* mac_init 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
} mac_init_rsp_st;

typedef struct {
    uint32_t ctx_addr;   // 对应原参数 ctx
    uint32_t msg_addr;   // 对应原参数 msg，消息数据指针地址
    uint32_t msg_size;   // 对应原参数 msg_size，消息长度
} mac_update_cmd_st;

/* mac_update 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
} mac_update_rsp_st;

typedef struct {
    uint32_t ctx;
    uint32_t msg;
    uint32_t msg_size;
    uint32_t mac;
} mac_finish_gen_cmd_st;

/* mac_finish_gen 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
} mac_finish_gen_rsp_st;

typedef struct {
    uint32_t ctx_addr;   // 对应原参数 ctx
    uint32_t msg_addr;   // 对应原参数 msg，消息数据指针地址
    uint32_t msg_size;   // 对应原参数 msg_size，消息长度
    uint32_t mac_addr;   // 对应原参数 mac，mac数据指针地址
} mac_finish_verify_cmd_st;

/* mac_finish_verify 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
    uint32_t verify_result;
} mac_finish_verify_rsp_st;

/* mac_onepass_gen_with_plain_key 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;    // 对应原参数 ctx
    uint32_t algo;        // 对应原参数 algo，对称算法类型
    uint32_t mode;        // 对应原参数 mode，MAC模式
    uint32_t key_addr;    // 对应原参数 key，明文密钥地址
    uint32_t key_size;    // 对应原参数 key_size，密钥长度
    uint32_t iv_addr;     // 对应原参数 iv，IV地址 (GMAC需要)
    uint32_t iv_size;     // 对应原参数 iv_size，IV长度
    uint32_t msg_addr;    // 对应原参数 msg，消息数据地址
    uint32_t msg_size;    // 对应原参数 msg_size，消息长度
    uint32_t mac_addr;    // 对应原参数 mac，MAC输出buffer地址
    uint32_t mac_size;    // 对应原参数 mac_size，期望的MAC长度
} mac_onepass_gen_with_plain_key_cmd_st;

typedef struct {
    uint32_t ret;
} mac_onepass_gen_with_plain_key_rsp_st;

/* mac_onepass_verify_with_plain_key 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;            // 对应原参数 ctx
    uint32_t algo;                // 对应原参数 algo，对称算法类型
    uint32_t mode;                // 对应原参数 mode，MAC模式
    uint32_t key_addr;            // 对应原参数 key，明文密钥地址
    uint32_t key_size;            // 对应原参数 key_size，密钥长度
    uint32_t iv_addr;             // 对应原参数 iv，IV地址 (GMAC需要)
    uint32_t iv_size;             // 对应原参数 iv_size，IV长度
    uint32_t msg_addr;            // 对应原参数 msg，消息数据地址
    uint32_t msg_size;            // 对应原参数 msg_size，消息长度
    uint32_t mac_addr;            // 对应原参数 mac，待验证的MAC值地址
    uint32_t mac_size;            // 对应原参数 mac_size，MAC值大小
    uint32_t verify_result_addr;  // 对应原参数 verify_result，验证结果地址
} mac_onepass_verify_with_plain_key_cmd_st;

typedef struct {
    uint32_t ret;
} mac_onepass_verify_with_plain_key_rsp_st;

/* mac_init_with_plain_key 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;     // 对应原参数 ctx
    uint32_t algo;         // 对应原参数 algo，对称算法类型
    uint32_t mode;         // 对应原参数 mode，MAC模式
    uint32_t key_addr;     // 对应原参数 key，明文密钥地址
    uint32_t key_size;     // 对应原参数 key_size，密钥长度
    uint32_t gen_mac;      // 对应原参数 gen_mac，是否生成MAC
    uint32_t iv_addr;      // 对应原参数 iv，IV地址 (GMAC需要)
    uint32_t iv_size;      // 对应原参数 iv_size，IV长度
    uint32_t mac_size;     // 对应原参数 mac_size，MAC长度
    uint32_t session_addr; // 对应原参数 session，会话结构体地址
} mac_init_with_plain_key_cmd_st;

typedef struct {
    uint32_t ret;
} mac_init_with_plain_key_rsp_st;

typedef struct {
    uint32_t ctx_addr;   // 对应原参数 ctx，结构体指针地址
    uint32_t counter_id; // 对应原参数 counter_id
} delete_counter_cmd_st;

/* driver_get_version 函数参数结构体（无参数） */
typedef struct {
    // 无参数
} driver_get_version_cmd_st;

/* driver_init_library 函数参数结构体 */
typedef struct {
    uint32_t drv_mode; // 对应原参数 drv_mode
} driver_init_library_cmd_st;

/* ecdsa_finish_gen 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr; // 对应原参数 ctx
    uint32_t sig_addr; // 对应原参数 sig，签名数据指针地址
    uint32_t sig_size; // 对应原参数 sig_size，签名长度指针地址
} ecdsa_finish_gen_cmd_st;

/* ecdsa_finish_verify 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;      // 对应原参数 ctx
    uint32_t sig_addr;      // 对应原参数 sig，签名数据指针地址
    uint32_t sig_size;      // 对应原参数 sig_size，签名长度
    uint32_t verify_result; // 对应原参数 verify_result，验证结果指针地址
} ecdsa_finish_verify_cmd_st;

/* ecdsa_init 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;     // 对应原参数 ctx
    uint32_t algo;         // 对应原参数 algo，哈希算法
    uint32_t key_handle;   // 对应原参数 key_handle，密钥句柄
    uint32_t gen_sig;      // 对应原参数 gen_sig，是否生成签名（0/1）
    uint32_t session_addr; // 对应原参数 session，会话结构体指针地址
} ecdsa_init_cmd_st;

/* ecdsa_onepass_gen 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;   // 对应原参数 ctx
    uint32_t algo;       // 对应原参数 algo，哈希算法
    uint32_t key_handle; // 对应原参数 key_handle，密钥句柄
    uint32_t msg_addr;   // 对应原参数 msg，消息数据指针地址
    uint32_t msg_size;   // 对应原参数 msg_size，消息长度
    uint32_t sig_addr;   // 对应原参数 sig，签名数据指针地址
    uint32_t sig_size;   // 对应原参数 sig_size，签名长度指针地址
} ecdsa_onepass_gen_cmd_st;

/* ecdsa_onepass_verify 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;      // 对应原参数 ctx
    uint32_t algo;          // 对应原参数 algo，哈希算法
    uint32_t key_handle;    // 对应原参数 key_handle，密钥句柄
    uint32_t msg_addr;      // 对应原参数 msg，消息数据指针地址
    uint32_t msg_size;      // 对应原参数 msg_size，消息长度
    uint32_t sig_addr;      // 对应原参数 sig，签名数据指针地址
    uint32_t sig_size;      // 对应原参数 sig_size，签名长度
} ecdsa_onepass_verify_cmd_st;

/* ecdsa_onepass_gen_with_digest 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;   // 对应原参数 ctx
    uint32_t algo;       // 对应原参数 algo，哈希算法
    uint32_t key_handle; // 对应原参数 key_handle，密钥句柄
    uint32_t digest_addr; // 对应原参数 digest，摘要数据指针地址
    uint32_t digest_size; // 对应原参数 digest_size，摘要长度
    uint32_t sig_addr;   // 对应原参数 sig，签名数据指针地址
    uint32_t sig_size;   // 对应原参数 sig_size，签名长度指针地址
} ecdsa_onepass_gen_with_digest_cmd_st;

/* ecdsa_onepass_verify_with_digest 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;      // 对应原参数 ctx
    uint32_t algo;          // 对应原参数 algo，哈希算法
    uint32_t key_handle;    // 对应原参数 key_handle，密钥句柄
    uint32_t digest_addr;   // 对应原参数 digest，摘要数据指针地址
    uint32_t digest_size;   // 对应原参数 digest_size，摘要长度
    uint32_t sig_addr;      // 对应原参数 sig，签名数据指针地址
    uint32_t sig_size;      // 对应原参数 sig_size，签名长度
} ecdsa_onepass_verify_with_digest_cmd_st;

/* ecdsa_update 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr; // 对应原参数 ctx
    uint32_t msg_addr; // 对应原参数 msg，消息数据指针地址
    uint32_t msg_size; // 对应原参数 msg_size，消息长度
} ecdsa_update_cmd_st;

/* enter_wfi 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr; // 对应原参数 ctx
} enter_wfi_cmd_st;

/* gen_random 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;      // 对应原参数 ctx
    uint32_t algo;          // 对应原参数 algo，随机数算法
    uint32_t rand_buf_addr; // 对应原参数 rand_buf，随机数缓冲区指针地址
    uint32_t rand_size;     // 对应原参数 rand_size，随机数长度
} gen_random_cmd_st;

/* hash_onepass 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;         // 对应原参数 ctx
    uint32_t algo;             // 对应原参数 algo，哈希算法
    uint32_t msg_addr;         // 对应原参数 msg，消息数据指针地址
    uint32_t msg_size;         // 对应原参数 msg_size，消息长度
    uint32_t digest_addr;      // 对应原参数 digest，摘要数据指针地址
    uint32_t digest_size;      // 对应原参数 digest_size，摘要长度
} hash_onepass_cmd_st;

/* get_challenge 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;       // 对应原参数 ctx
    uint32_t challenge_type; // 对应原参数 challenge_type，挑战类型
    uint32_t output_addr;    // 对应原参数 output，输出缓冲区指针地址
} get_challenge_cmd_st;

/* get_emu_status 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;        // 对应原参数 ctx
    uint32_t status_buf_addr; // 对应原参数 status_buf，状态缓冲区指针地址
} get_emu_status_cmd_st;

/* get_utc_time 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;      // 对应原参数 ctx
    uint32_t utc_time_addr; // 对应原参数 utc_time，UTC时间指针地址
} get_utc_time_cmd_st;

/* get_version 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;     // 对应原参数 ctx
    uint32_t version_addr; // 对应原参数 version，版本结构体指针地址
} get_version_cmd_st;

/* hmac_finish_gen 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;  // 对应原参数 ctx
    uint32_t hmac_addr; // 对应原参数 hmac，HMAC数据指针地址
    uint32_t hmac_size; // 对应原参数 hmac_size，HMAC长度
} hmac_finish_gen_cmd_st;

/* hmac_init 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;     // 对应原参数 ctx
    uint32_t algo;         // 对应原参数 algo，哈希算法
    uint32_t key_handle;   // 对应原参数 key_handle，密钥句柄
    uint32_t gen_hmac;     // 对应原参数 gen_hmac，是否生成HMAC（0/1）
    uint32_t session_addr; // 对应原参数 session，会话结构体指针地址
} hmac_init_cmd_st;

/* hmac_onepass_gen 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;   // 对应原参数 ctx
    uint32_t algo;       // 对应原参数 algo，哈希算法
    uint32_t key_handle; // 对应原参数 key_handle，密钥句柄
    uint32_t msg_addr;   // 对应原参数 msg，消息数据指针地址
    uint32_t msg_size;   // 对应原参数 msg_size，消息长度
    uint32_t hmac_addr;  // 对应原参数 hmac，HMAC数据指针地址
    uint32_t hmac_size;  // 对应原参数 hmac_size，HMAC长度
} hmac_onepass_gen_cmd_st;

/* hmac_finish_verify 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;      // 对应原参数 ctx
    uint32_t hmac_addr;     // 对应原参数 hmac，HMAC数据指针地址
    uint32_t hmac_size;     // 对应原参数 hmac_size，HMAC长度
} hmac_finish_verify_cmd_st;

/* hmac_onepass_verify 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;      // 对应原参数 ctx
    uint32_t algo;          // 对应原参数 algo，哈希算法
    uint32_t key_handle;    // 对应原参数 key_handle，密钥句柄
    uint32_t msg_addr;      // 对应原参数 msg，消息数据指针地址
    uint32_t msg_size;      // 对应原参数 msg_size，消息长度
    uint32_t hmac_addr;     // 对应原参数 hmac，HMAC数据指针地址
    uint32_t hmac_size;     // 对应原参数 hmac_size，HMAC长度
} hmac_onepass_verify_cmd_st;

/* hmac_update 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr; // 对应原参数 ctx
    uint32_t msg_addr; // 对应原参数 msg，消息数据指针地址
    uint32_t msg_size; // 对应原参数 msg_size，消息长度
} hmac_update_cmd_st;

/* hmac_onepass_gen_with_plain_key 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;     // 对应原参数 ctx
    uint32_t algo;         // 对应原参数 algo，HASH算法类型
    uint32_t key_addr;     // 对应原参数 key，明文密钥地址
    uint32_t key_size;     // 对应原参数 key_size，密钥长度
    uint32_t msg_addr;     // 对应原参数 msg，消息数据地址
    uint32_t msg_size;     // 对应原参数 msg_size，消息长度
    uint32_t hmac_addr;    // 对应原参数 hmac，HMAC输出buffer地址
    uint32_t hmac_size;    // 对应原参数 hmac_size，期望的HMAC长度
} hmac_onepass_gen_with_plain_key_cmd_st;

typedef struct {
    uint32_t ret;
} hmac_onepass_gen_with_plain_key_rsp_st;

/* hmac_onepass_verify_with_plain_key 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;            // 对应原参数 ctx
    uint32_t algo;                // 对应原参数 algo，HASH算法类型
    uint32_t key_addr;            // 对应原参数 key，明文密钥地址
    uint32_t key_size;            // 对应原参数 key_size，密钥长度
    uint32_t msg_addr;            // 对应原参数 msg，消息数据地址
    uint32_t msg_size;            // 对应原参数 msg_size，消息长度
    uint32_t hmac_addr;           // 对应原参数 hmac，待验证的HMAC值地址
    uint32_t hmac_size;           // 对应原参数 hmac_size，HMAC值大小
    uint32_t verify_result_addr;  // 对应原参数 verify_result，验证结果地址
} hmac_onepass_verify_with_plain_key_cmd_st;

typedef struct {
    uint32_t ret;
} hmac_onepass_verify_with_plain_key_rsp_st;

/* hmac_init_with_plain_key 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;     // 对应原参数 ctx
    uint32_t algo;         // 对应原参数 algo，HASH算法类型
    uint32_t key_addr;     // 对应原参数 key，明文密钥地址
    uint32_t key_size;     // 对应原参数 key_size，密钥长度
    uint32_t gen_hmac;     // 对应原参数 gen_hmac，是否生成HMAC
    uint32_t session;      // 对应原参数 session，会话地址
} hmac_init_with_plain_key_cmd_st;

typedef struct {
    uint32_t ret;
} hmac_init_with_plain_key_rsp_st;

/* increase_counter 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;           // 对应原参数 ctx
    uint32_t counter_id;         // 对应原参数 counter_id，计数器ID
    uint32_t increase_value;     // 对应原参数 increase_value，增量值（原uint64_t截断为uint32_t）
    uint32_t current_value_addr; // 对应原参数 current_value，当前值指针地址（原uint64_t截断为uint32_t）
} increase_counter_cmd_st;

/* inject_error 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;    // 对应原参数 ctx
    uint32_t values_addr; // 对应原参数 values，错误注入结构体指针地址
} inject_error_cmd_st;

/* install_encrypted_key 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;        // 对应原参数 ctx
    uint32_t key_level;       // 对应原参数 key_level，密钥级别
    uint32_t key_type;        // 对应原参数 key_type，密钥类型
    uint32_t key_slot_id;     // 对应原参数 key_slot_id，密钥槽ID
    uint32_t last_key;        // 对应原参数 last_key，是否为最后一个密钥（0/1）
    uint32_t input_data_addr; // 对应原参数 input_data，输入数据指针地址
    uint32_t size;            // 对应原参数 size，数据长度
} install_encrypted_key_cmd_st;

/* install_random_key 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;    // 对应原参数 ctx
    uint32_t key_level;   // 对应原参数 key_level，密钥级别
    uint32_t key_type;    // 对应原参数 key_type，密钥类型
    uint32_t key_slot_id; // 对应原参数 key_slot_id，密钥槽ID
    uint32_t last_key;    // 对应原参数 last_key，是否为最后一个密钥（0/1）
} install_random_key_cmd_st;

/* km_derive_key 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;          // 对应原参数 ctx
    uint32_t hash_algo;         // 对应原参数 hash_algo，哈希算法
    uint32_t derive_algo;       // 对应原参数 derive_algo，派生算法
    uint32_t derive_type;       // 对应原参数 derive_type，派生类型
    uint32_t privilege;         // 对应原参数 privilege，权限
    uint32_t key_type;          // 对应原参数 key_type，密钥类型
    uint32_t key_size;          // 对应原参数 key_size，密钥长度
    uint32_t parent_key_handle; // 对应原参数 parent_key_handle，父密钥句柄
    uint32_t salt_addr;         // 对应原参数 salt，盐值数据指针地址
    uint32_t salt_size;         // 对应原参数 salt_size，盐值长度
    uint32_t password_addr;     // 对应原参数 password，密码数据指针地址
    uint32_t password_size;     // 对应原参数 password_size，密码长度
    uint32_t iter_times;        // 对应原参数 iter_times，迭代次数
    uint32_t key_handle_addr;   // 对应原参数 key_handle，生成密钥句柄指针地址
} km_derive_key_cmd_st;

/* km_derive_key_to_soc 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;          // 对应原参数 ctx
    uint32_t hash_algo;         // 对应原参数 hash_algo，哈希算法
    uint32_t derive_algo;       // 对应原参数 derive_algo，派生算法
    uint32_t derive_type;       // 对应原参数 derive_type，派生类型
    uint32_t parent_key_handle; // 对应原参数 parent_key_handle，父密钥句柄
    uint32_t salt_addr;         // 对应原参数 salt，盐值数据指针地址
    uint32_t salt_size;         // 对应原参数 salt_size，盐值长度
    uint32_t iter_times;        // 对应原参数 iter_times，迭代次数
    uint32_t soc_channel_id;    // 对应原参数 soc_channel_id，SOC通道ID
} km_derive_key_to_soc_cmd_st;

/* km_exchange_key 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;         // 对应原参数 ctx
    uint32_t rmt_pub_key_addr; // 对应原参数 rmt_pub_key，远程公钥指针地址
    uint32_t rmt_pub_key_size; // 对应原参数 rmt_pub_key_size，远程公钥长度
    uint32_t privilege;        // 对应原参数 privilege，权限
    uint32_t key_type;         // 对应原参数 key_type，密钥类型
    uint32_t hmac_key_size;    // 对应原参数 hmac_key_size，HMAC密钥长度
    uint32_t local_key_handle; // 对应原参数 local_key_handle，本地密钥句柄
    uint32_t dh_params_addr;   // 对应原参数 dh_params，DH参数指针地址
    uint32_t dh_params_size;   // 对应原参数 dh_params_size，DH参数长度
    uint32_t sm2_params_addr;  // 对应原参数 sm2_params，SM2参数结构体指针地址
    uint32_t key_handle_addr;  // 对应原参数 key_handle，生成密钥句柄指针地址
} km_exchange_key_cmd_st;

/* km_export_key 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;             // 对应原参数 ctx
    uint32_t target_key_handle;    // 对应原参数 target_key_handle，目标密钥句柄
    uint32_t transport_key_handle; // 对应原参数 transport_key_handle，传输密钥句柄
    uint32_t auth_key_handle;      // 对应原参数 auth_key_handle，认证密钥句柄
    uint32_t key_part;             // 对应原参数 key_part，密钥部分
    uint32_t key_data_addr;        // 对应原参数 key_data，密钥数据结构体指针地址
    uint32_t key_data_size;        // 对应原参数 key_data_size，密钥数据长度
    uint32_t mac_addr;             // 对应原参数 mac，MAC值指针地址
    uint32_t mac_size;             // 对应原参数 mac_size，MAC长度
} km_export_key_cmd_st;

/* km_gen_key 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;        // 对应原参数 ctx
    uint32_t key_type;        // 对应原参数 key_type，密钥类型
    uint32_t privilege;       // 对应原参数 privilege，权限
    uint32_t rsa_e_bit_size;  // 对应原参数 rsa_e_bit_size，RSA指数位长
    uint32_t hmac_key_size;   // 对应原参数 hmac_key_size，HMAC密钥长度
    uint32_t dh_params_addr;  // 对应原参数 dh_params，DH参数指针地址
    uint32_t dh_params_size;  // 对应原参数 dh_params_size，DH参数长度
    uint32_t key_handle_addr; // 对应原参数 key_handle，生成密钥句柄指针地址
} km_gen_key_cmd_st;

/* km_get_pub_from_priv 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;          // 对应原参数 ctx
    uint32_t key_handle;        // 对应原参数 key_handle，密钥句柄
    uint32_t dh_params;    // 对应原参数 dh_params，DH参数指针地址
    uint32_t dh_params_size;    // 对应原参数 dh_params_size，DH参数长度
    uint32_t pub_key;      // 对应原参数 pub_key，公钥数据指针地址
    uint32_t pub_key_size; // 对应原参数 pub_key_size，公钥长度指针地址
    uint32_t key_type;     // 对应原参数 key_type，密钥类型指针地址
} km_get_pub_from_priv_cmd_st;

/* km_import_key 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;             // 对应原参数 ctx
    uint32_t transport_key_handle; // 对应原参数 transport_key_handle，传输密钥句柄
    uint32_t auth_key_handle;      // 对应原参数 auth_key_handle，认证密钥句柄
    uint32_t key_data_addr;        // 对应原参数 key_data，密钥数据结构体指针地址
    uint32_t key_data_size;        // 对应原参数 key_data_size，密钥数据长度
    uint32_t mac_addr;             // 对应原参数 mac，MAC值指针地址
    uint32_t mac_size;             // 对应原参数 mac_size，MAC长度
    uint32_t key_handle;           // 对应原参数 key_handle，导入密钥句柄
} km_import_key_cmd_st;

/* km_remove_key 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;   // 对应原参数 ctx
    uint32_t key_handle; // 对应原参数 key_handle，密钥句柄
} km_remove_key_cmd_st;

/* km_sm9_exchange_key 函数参数结构体 */
typedef struct {
    uint32_t ctx_addr;             // 对应原参数 ctx
    uint32_t privilege;            // 对应原参数 privilege，权限
    uint32_t key_type;             // 对应原参数 key_type，密钥类型
    uint32_t role;                 // 对应原参数 role，角色
    uint32_t user_priv_key_handle; // 对应原参数 user_priv_key_handle，用户私钥句柄
    uint32_t user_tmp_key_handle;  // 对应原参数 user_tmp_key_handle，用户临时密钥句柄
    uint32_t hmac_key_size;        // 对应原参数 hmac_key_size，HMAC密钥长度
    uint32_t extra_params_addr;    // 对应原参数 extra_params，SM9额外参数结构体指针地址
} km_sm9_exchange_key_cmd_st;

/* delete_counter 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值，0表示成功，非0表示错误码
} delete_counter_rsp_st;

/* driver_get_version 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
} driver_get_version_rsp_st;

/* driver_init_library 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
} driver_init_library_rsp_st;

/* ecdsa_finish_gen 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
    uint32_t sig_size;
} ecdsa_finish_gen_rsp_st;

/* ecdsa_finish_verify 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
    uint32_t verify_result;
} ecdsa_finish_verify_rsp_st;

/* ecdsa_init 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
} ecdsa_init_rsp_st;

/* ecdsa_onepass_gen 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
    uint32_t sig_size;
} ecdsa_onepass_gen_rsp_st;

/* ecdsa_onepass_verify 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
    uint32_t sig_size;
    uint32_t verify_result;
} ecdsa_onepass_verify_rsp_st;

/* ecdsa_update 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
} ecdsa_update_rsp_st;

/* enter_wfi 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
} enter_wfi_rsp_st;

/* gen_random 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
} gen_random_rsp_st;

/* get_challenge 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
} get_challenge_rsp_st;

/* get_emu_status 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
} get_emu_status_rsp_st;

/* get_utc_time 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
} get_utc_time_rsp_st;

/* get_version 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
} get_version_rsp_st;

/* hmac_finish_gen 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
} hmac_finish_gen_rsp_st;

/* hmac_finish_verify 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
    uint32_t verify_result;
} hmac_finish_verify_rsp_st;

/* hmac_init 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
} hmac_init_rsp_st;

/* hmac_onepass_gen 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
} hmac_onepass_gen_rsp_st;

/* hmac_onepass_verify 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
    uint32_t verify_result;
} hmac_onepass_verify_rsp_st;

/* hmac_update 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
} hmac_update_rsp_st;

/* increase_counter 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
} increase_counter_rsp_st;

/* inject_error 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
} inject_error_rsp_st;

/* install_encrypted_key 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
} install_encrypted_key_rsp_st;

/* install_random_key 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
} install_random_key_rsp_st;

/* km_derive_key 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
    uint32_t handle;
} km_derive_key_rsp_st;

/* km_derive_key_to_soc 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
} km_derive_key_to_soc_rsp_st;

/* km_exchange_key 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
    uint32_t handle;
} km_exchange_key_rsp_st;

/* km_export_key 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
    uint32_t key_data_size;
    uint32_t mac_size;
} km_export_key_rsp_st;

/* km_gen_key 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
    uint32_t handle;
} km_gen_key_rsp_st;

/* km_get_pub_from_priv 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
    uint32_t algo_id;
    uint32_t public_key_size;
} km_get_pub_from_priv_rsp_st;

/* km_import_key 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
    uint32_t handle;
} km_import_key_rsp_st;

/* km_remove_key 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
} km_remove_key_rsp_st;

/* km_sm9_exchange_key 函数响应结构体 */
typedef struct {
    uint32_t ret; // 函数返回值
    uint32_t key_handle;
} km_sm9_exchange_key_rsp_st;

typedef struct {
    uint32_t ctx;
    uint32_t algo;
    uint32_t mode;
    uint32_t padding;
    uint32_t key_handle;
    uint32_t enc;
    uint32_t iv_addr;
    uint32_t iv_size;
    uint32_t input_addr;
    uint32_t input_size;
    uint32_t output_addr;
    uint32_t outbuf_size;
} symm_cipher_onepass_cmd_st;

typedef struct {
    uint32_t ctx;
    uint32_t algo;
    uint32_t mode;
    uint32_t padding;
    uint32_t enc;
    uint32_t key_addr;
    uint32_t key_size;
    uint32_t iv_addr;
    uint32_t iv_size;
    uint32_t input_addr;
    uint32_t input_size;
    uint32_t output_addr;
    uint32_t outbuf_size;
} symm_cipher_onepass_with_plain_key_cmd_st;

typedef struct {
    uint32_t ret;
    uint32_t output_size;
} symm_cipher_onepass_rsp_st;

typedef struct {
    uint32_t ctx;
    uint32_t algo;
    uint32_t mode;
    uint32_t padding;
    uint32_t key_handle;
    uint32_t enc;
    uint32_t iv;
    uint32_t iv_size;
    uint32_t session;
} symm_cipher_init_cmd_st;

typedef struct {
    uint32_t ret;
} symm_cipher_init_rsp_st;

typedef struct {
    uint32_t ctx;
    uint32_t algo;
    uint32_t mode;
    uint32_t padding;
    uint32_t key_addr;
    uint32_t key_size;
    uint32_t enc;
    uint32_t iv;
    uint32_t iv_size;
    uint32_t session;
} symm_cipher_init_with_plain_key_cmd_st;

typedef struct {
    uint32_t ret;
} symm_cipher_init_with_plain_key_rsp_st;

typedef struct {
    uint32_t ctx;
    uint32_t input;
    uint32_t input_size;
    uint32_t output;
} symm_cipher_update_cmd_st;

typedef struct {
    uint32_t ret;
} symm_cipher_update_rsp_st;

typedef struct {
    uint32_t ctx;
    uint32_t input;
    uint32_t input_size;
    uint32_t output;
    uint32_t output_size;
} symm_cipher_finish_cmd_st;

typedef struct {
    uint32_t ret;
    uint32_t output_size;
} symm_cipher_finish_rsp_st;

typedef struct {
    uint32_t ctx;
    uint32_t image;
    uint32_t image_size;
    uint32_t image_out;
} upgrade_fw_image_cmd_st;

typedef struct {
    uint32_t ret;
} upgrade_fw_image_rsp_st;

// Three-stage upgrade firmware image structures
typedef struct {
    uint32_t ctx_addr;
    uint32_t input_addr;    // image_header, body_block, or last_block
    uint32_t input_size;    // header_size or block_size
    uint32_t output_addr;   // image_out
} upgrade_fw_image_3stage_cmd_st;

typedef struct {
    uint32_t ret;
} upgrade_fw_image_3stage_rsp_st;

typedef struct {
    uint32_t ctx;
    uint32_t image;
    uint32_t image_size;
    uint32_t check_version;
    uint32_t boot;
    uint32_t image_out;
} verify_image_cmd_st;

typedef struct {
    uint32_t ret;
} verify_image_rsp_st;

uint16_t test_cmd_hostapi_mac_onepass_gen(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(mac_onepass_gen_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const mac_onepass_gen_cmd_st *cmd_val = (const mac_onepass_gen_cmd_st *)cmd->val;

    uint32_t ret = ehsm_mac_onepass_gen((ehsm_ctx_st *)cmd_val->ctx_addr, (ehsm_symm_algo_e)cmd_val->algo,
        (ehsm_mac_mode_e)cmd_val->mode, (uint32_t)cmd_val->key_handle, (const uint8_t *)cmd_val->iv_addr,
        (uint32_t)cmd_val->iv_size, (const uint8_t *)cmd_val->msg_addr, (uint32_t)cmd_val->msg_size,
        (uint8_t *)cmd_val->mac_addr, (uint32_t)cmd_val->mac_size);

    mac_onepass_gen_rsp_st *rsp_val = (mac_onepass_gen_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(mac_onepass_gen_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_mac_onepass_gen_with_key(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(mac_onepass_gen_with_plain_key_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }
    const mac_onepass_gen_with_plain_key_cmd_st *cmd_val =
        (const mac_onepass_gen_with_plain_key_cmd_st *)cmd->val;

    uint32_t ret = ehsm_mac_onepass_gen_with_plain_key(
        (ehsm_ctx_st *)cmd_val->ctx_addr,
        (ehsm_symm_algo_e)cmd_val->algo,
        (ehsm_mac_mode_e)cmd_val->mode,
        (const uint8_t *)cmd_val->key_addr,
        cmd_val->key_size,
        (const uint8_t *)cmd_val->iv_addr,
        cmd_val->iv_size,
        (const uint8_t *)cmd_val->msg_addr,
        cmd_val->msg_size,
        (uint8_t *)cmd_val->mac_addr,
        cmd_val->mac_size
    );

    mac_onepass_gen_with_plain_key_rsp_st *rsp_data =
        (mac_onepass_gen_with_plain_key_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(mac_onepass_gen_with_plain_key_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_mac_onepass_verify(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(mac_onepass_verify_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const mac_onepass_verify_cmd_st *cmd_val = (const mac_onepass_verify_cmd_st *)cmd->val;

    uint32_t verify_result = 0;
    uint32_t ret = ehsm_mac_onepass_verify((ehsm_ctx_st *)cmd_val->ctx_addr, (ehsm_symm_algo_e)cmd_val->algo,
        (ehsm_mac_mode_e)cmd_val->mode, (uint32_t)cmd_val->key_handle, (const uint8_t *)cmd_val->iv_addr,
        (uint32_t)cmd_val->iv_size, (const uint8_t *)cmd_val->msg_addr, (uint32_t)cmd_val->msg_size,
        (uint8_t *)cmd_val->mac_addr, (uint32_t)cmd_val->mac_size, (bool_t *)&verify_result);

    mac_onepass_verify_rsp_st *rsp_val = (mac_onepass_verify_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp_val->verify_result = verify_result;
    rsp->len = sizeof(mac_onepass_verify_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_mac_onepass_verify_with_key(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(mac_onepass_verify_with_plain_key_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }
    const mac_onepass_verify_with_plain_key_cmd_st *cmd_val =
        (const mac_onepass_verify_with_plain_key_cmd_st *)cmd->val;

    uint32_t ret = ehsm_mac_onepass_verify_with_plain_key(
        (ehsm_ctx_st *)cmd_val->ctx_addr,
        (ehsm_symm_algo_e)cmd_val->algo,
        (ehsm_mac_mode_e)cmd_val->mode,
        (const uint8_t *)cmd_val->key_addr,
        cmd_val->key_size,
        (const uint8_t *)cmd_val->iv_addr,
        cmd_val->iv_size,
        (const uint8_t *)cmd_val->msg_addr,
        cmd_val->msg_size,
        (const uint8_t *)cmd_val->mac_addr,
        cmd_val->mac_size,
        (bool_t *)cmd_val->verify_result_addr
    );

    mac_onepass_verify_with_plain_key_rsp_st *rsp_data =
        (mac_onepass_verify_with_plain_key_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(mac_onepass_verify_with_plain_key_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_mac_init(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(mac_init_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const mac_init_cmd_st *cmd_val = (const mac_init_cmd_st *)cmd->val;

    uint32_t ret = ehsm_mac_init((ehsm_ctx_st *)cmd_val->ctx_addr, (ehsm_symm_algo_e)cmd_val->algo,
        (ehsm_mac_mode_e)cmd_val->mode, (uint32_t)cmd_val->key_handle, (bool_t)cmd_val->gen_mac,
        (const uint8_t *)cmd_val->iv_addr, (uint32_t)cmd_val->iv_size, (uint32_t)cmd_val->mac_size,
        (ehsm_session_st *)cmd_val->session_addr);

    mac_init_rsp_st *rsp_val = (mac_init_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(mac_init_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_mac_init_with_key(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(mac_init_with_plain_key_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }
    const mac_init_with_plain_key_cmd_st *cmd_val =
        (const mac_init_with_plain_key_cmd_st *)cmd->val;

    uint32_t ret = ehsm_mac_init_with_plain_key(
        (ehsm_ctx_st *)cmd_val->ctx_addr,
        (ehsm_symm_algo_e)cmd_val->algo,
        (ehsm_mac_mode_e)cmd_val->mode,
        (const uint8_t *)cmd_val->key_addr,
        cmd_val->key_size,
        (bool_t)cmd_val->gen_mac,
        (const uint8_t *)cmd_val->iv_addr,
        cmd_val->iv_size,
        cmd_val->mac_size,
        (ehsm_session_st *)cmd_val->session_addr
    );

    mac_init_with_plain_key_rsp_st *rsp_data =
        (mac_init_with_plain_key_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(mac_init_with_plain_key_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_mac_update(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(mac_update_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const mac_update_cmd_st *cmd_val = (const mac_update_cmd_st *)cmd->val;

    uint32_t ret = ehsm_mac_update((ehsm_ctx_st *)cmd_val->ctx_addr, (const uint8_t *)cmd_val->msg_addr,
        (uint32_t)cmd_val->msg_size);

    mac_update_rsp_st *rsp_val = (mac_update_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(mac_update_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_mac_finish_gen(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(mac_finish_gen_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const mac_finish_gen_cmd_st *cmd_val = (const mac_finish_gen_cmd_st *)cmd->val;

    uint32_t ret = ehsm_mac_finish_gen((ehsm_ctx_st *)cmd_val->ctx, (const uint8_t *)cmd_val->msg,
        (uint32_t)cmd_val->msg_size, (uint8_t *)cmd_val->mac);

    mac_finish_gen_rsp_st *rsp_val = (mac_finish_gen_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(mac_finish_gen_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_mac_finish_verify(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(mac_finish_verify_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const mac_finish_verify_cmd_st *cmd_val = (const mac_finish_verify_cmd_st *)cmd->val;

    uint32_t verify_result = 0;
    uint32_t ret = ehsm_mac_finish_verify((ehsm_ctx_st *)cmd_val->ctx_addr, (const uint8_t *)cmd_val->msg_addr,
        (uint32_t)cmd_val->msg_size, (const uint8_t *)cmd_val->mac_addr, (bool_t *)&verify_result);

    mac_finish_verify_rsp_st *rsp_val = (mac_finish_verify_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp_val->verify_result = verify_result;
    rsp->len = sizeof(mac_finish_verify_rsp_st);
    return RSP_OK;
}

typedef struct {
    uint32_t ctx;
    uint32_t counter_id;
    uint32_t counter_value;
} read_counter_cmd_st;

typedef struct {
    uint32_t ret;
    uint32_t low_value;
    uint32_t high_value;
} read_counter_rsp_st;

uint16_t test_cmd_hostapi_read_counter(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(read_counter_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const read_counter_cmd_st *cmd_val = (const read_counter_cmd_st *)cmd->val;
    uint32_t ret = ehsm_read_counter(
        (ehsm_ctx_st *)cmd_val->ctx, (uint32_t)cmd_val->counter_id, (uint64_t *)cmd_val->counter_value);

    read_counter_rsp_st *rsp_val = (read_counter_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(read_counter_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_read_otp(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(read_otp_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const read_otp_cmd_st *cmd_val = (const read_otp_cmd_st *)cmd->val;
    uint32_t ret = ehsm_read_otp((ehsm_ctx_st *)cmd_val->ctx_addr, (uint8_t *)cmd_val->buf,
        (uint32_t)cmd_val->ehsm_src_addr, (uint32_t)cmd_val->size);

    read_otp_rsp_st *rsp_val = (read_otp_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(read_otp_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_read_reg(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(read_reg_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const read_reg_cmd_st *cmd_val = (const read_reg_cmd_st *)cmd->val;
    uint32_t ret = ehsm_read_reg((ehsm_ctx_st *)cmd_val->ctx_addr, (uint8_t *)cmd_val->buf,
        (uint32_t)cmd_val->ehsm_src_addr, (uint32_t)cmd_val->size);

    read_reg_rsp_st *rsp_val = (read_reg_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(read_reg_rsp_st);
    return RSP_OK;
}

typedef struct {
    uint32_t ctx;
    uint32_t baud_div;
} uart_baudrate_cmd_st;

typedef struct {
    uint32_t ret;
} uart_baudrate_rsp_st;

uint16_t test_cmd_hostapi_set_uart_baudrate(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(uart_baudrate_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const uart_baudrate_cmd_st *cmd_val = (const uart_baudrate_cmd_st *)cmd->val;
    uint32_t ret = ehsm_set_uart_baudrate((ehsm_ctx_st *)cmd_val->ctx, (uint32_t)cmd_val->baud_div);

    uart_baudrate_rsp_st *rsp_val = (uart_baudrate_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(uart_baudrate_rsp_st);
    return RSP_OK;
}

typedef struct {
    uint32_t ctx;
    uint32_t utc_time;
} utc_time_cmd_st;

typedef struct {
    uint32_t ret;
} utc_time_rsp_st;

uint16_t test_cmd_hostapi_set_utc_time(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(utc_time_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const utc_time_cmd_st *cmd_val = (const utc_time_cmd_st *)cmd->val;
    uint32_t ret = ehsm_set_utc_time((ehsm_ctx_st *)cmd_val->ctx, (uint32_t)cmd_val->utc_time);

    utc_time_rsp_st *rsp_val = (utc_time_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(utc_time_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_sm2_sign_onepass_gen(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(sm2_sign_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }
    const sm2_sign_cmd_st *cmd_val = (const sm2_sign_cmd_st *)cmd->val;
    uint32_t ret = ehsm_sm2_sign_onepass_gen((ehsm_ctx_st *)cmd_val->ctx_addr, cmd_val->key_handle,
        (const uint8_t *)cmd_val->msg_addr, cmd_val->msg_size, (uint8_t *)cmd_val->sig_addr, cmd_val->sig_size);
    sm2_sign_rsp_st *rsp_data = (sm2_sign_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->sign_size = 64;
    rsp->len = sizeof(sm2_sign_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_sm2_sign_onepass_gen_with_digest(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(sm2_sign_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }
    const sm2_sign_cmd_st *cmd_val = (const sm2_sign_cmd_st *)cmd->val;
    uint32_t ret = ehsm_sm2_sign_onepass_gen_with_digest((ehsm_ctx_st *)cmd_val->ctx_addr, cmd_val->key_handle,
        (const uint8_t *)cmd_val->msg_addr, cmd_val->msg_size, (uint8_t *)cmd_val->sig_addr, cmd_val->sig_size);
    sm2_sign_rsp_st *rsp_data = (sm2_sign_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->sign_size = 64;
    rsp->len = sizeof(sm2_sign_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_sm2_sign_onepass_verify(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(sm2_sign_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }
    uint32_t verify_result = 0;
    const sm2_sign_cmd_st *cmd_val = (const sm2_sign_cmd_st *)cmd->val;
    uint32_t ret = ehsm_sm2_sign_onepass_verify((ehsm_ctx_st *)cmd_val->ctx_addr, cmd_val->key_handle,
        (const uint8_t *)cmd_val->msg_addr, cmd_val->msg_size, (uint8_t *)cmd_val->sig_addr, cmd_val->sig_size,
        (bool_t *)&verify_result);

    sm2_sign_rsp_st *rsp_data = (sm2_sign_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->verification = verify_result;
    rsp->len = sizeof(sm2_sign_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_sm2_sign_onepass_verify_with_digest(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(sm2_sign_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }
    uint32_t verify_result = 0;
    const sm2_sign_cmd_st *cmd_val = (const sm2_sign_cmd_st *)cmd->val;
    uint32_t ret = ehsm_sm2_sign_onepass_verify_with_digest((ehsm_ctx_st *)cmd_val->ctx_addr, cmd_val->key_handle,
        (const uint8_t *)cmd_val->msg_addr, cmd_val->msg_size, (uint8_t *)cmd_val->sig_addr, cmd_val->sig_size,
        (bool_t *)&verify_result);

    sm2_sign_rsp_st *rsp_data = (sm2_sign_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->verification = verify_result;
    rsp->len = sizeof(sm2_sign_rsp_st);
    return RSP_OK;
}

typedef struct {
    uint32_t ctx;
    uint32_t use_plain_key;
    uint32_t key_handle;
    uint32_t key;
    uint32_t gen_sig;
    uint32_t is_digest;
    uint32_t input_addr;
    uint32_t input_size;
    uint32_t sig_addr;
    uint32_t sig_size;
    uint32_t verify_result_addr;
} sm2_sign_onepass_ex_cmd_st;

uint16_t test_cmd_hostapi_sm2_sign_onepass_ex(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(sm2_sign_onepass_ex_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const sm2_sign_onepass_ex_cmd_st *cmd_val = (const sm2_sign_onepass_ex_cmd_st *)cmd->val;
    bool_t verify_result = false;

    uint32_t ret = ehsm_sm2_sign_onepass_ex(
        (ehsm_ctx_st *)cmd_val->ctx,
        (bool_t)cmd_val->use_plain_key,
        cmd_val->key_handle,
        (const uint8_t *)cmd_val->key,
        (bool_t)cmd_val->gen_sig,
        (bool_t)cmd_val->is_digest,
        (const uint8_t *)cmd_val->input_addr,
        cmd_val->input_size,
        (uintptr_t)cmd_val->sig_addr,
        cmd_val->sig_size,
        &verify_result
    );

    // Write verify_result to shared memory
    if (!cmd_val->gen_sig && cmd_val->verify_result_addr != 0) {
        *(uint32_t *)cmd_val->verify_result_addr = (uint32_t)verify_result;
    }

    sm2_sign_rsp_st *rsp_data = (sm2_sign_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->sign_size = 64;
    rsp_data->verification = verify_result;
    rsp->len = sizeof(sm2_sign_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_sm2_sign_init(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(sm2_sign_init_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const sm2_sign_init_cmd_st *cmd_val = (const sm2_sign_init_cmd_st *)cmd->val;
    uint32_t ret = ehsm_sm2_sign_init((ehsm_ctx_st *)cmd_val->ctx, (uint32_t)cmd_val->key_handle,
        (bool_t)cmd_val->gen_sig, (ehsm_session_st *)cmd_val->session);

    sm2_sign_rsp_st *rsp_val = (sm2_sign_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(sm2_sign_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_sm2_sign_update(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(sm2_sign_update_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const sm2_sign_update_cmd_st *cmd_val = (const sm2_sign_update_cmd_st *)cmd->val;
    uint32_t ret
        = ehsm_sm2_sign_update((ehsm_ctx_st *)cmd_val->ctx, (const uint8_t *)cmd_val->msg, (uint32_t)cmd_val->msg_size);

    sm2_sign_rsp_st *rsp_val = (sm2_sign_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(sm2_sign_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_sm2_sign_finish_gen(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(sm2_sign_finish_gen_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const sm2_sign_finish_gen_cmd_st *cmd_val = (const sm2_sign_finish_gen_cmd_st *)cmd->val;
    uint32_t ret
        = ehsm_sm2_sign_finish_gen((ehsm_ctx_st *)cmd_val->ctx, (uint8_t *)cmd_val->sig, (uint32_t)cmd_val->sig_size);

    sm2_sign_rsp_st *rsp_val = (sm2_sign_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp_val->sign_size = 64;
    rsp->len = sizeof(sm2_sign_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_sm2_sign_finish_verify(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(sm2_sign_finish_verify_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }
    uint32_t verify_result = 0;
    const sm2_sign_finish_verify_cmd_st *cmd_val = (const sm2_sign_finish_verify_cmd_st *)cmd->val;
    uint32_t ret = ehsm_sm2_sign_finish_verify((ehsm_ctx_st *)cmd_val->ctx, (const uint8_t *)cmd_val->sig,
        (uint32_t)cmd_val->sig_size, (bool_t *)&verify_result);

    sm2_sign_rsp_st *rsp_val = (sm2_sign_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp_val->verification = verify_result;
    rsp->len = sizeof(sm2_sign_rsp_st);
    return RSP_OK;
}

typedef struct {
    uint32_t ctx_addr;
    uint32_t key_handle;
    uint32_t enc;
    uint32_t enc_type;
    uint32_t padding;
    uint32_t key2_size;
    uint32_t hid;
    uint32_t kgc_pub_key;
    uint32_t input;
    uint32_t input_size;
    uint32_t output;
    uint32_t output_size;
    uint32_t id;
    uint32_t id_size;
    uint32_t fp12g;
} sm9_cipher_cmd_st;

typedef struct {
    uint32_t ret;
    uint32_t output_size;
} sm9_cipher_rsp_st;

uint16_t test_cmd_hostapi_sm9_cipher(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(sm9_cipher_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const sm9_cipher_cmd_st *cmd_val = (const sm9_cipher_cmd_st *)cmd->val;
    uint32_t ret = ehsm_sm9_cipher((ehsm_ctx_st *)cmd_val->ctx_addr, (uint32_t)cmd_val->key_handle,
        (bool_t)cmd_val->enc, (ehsm_sm9_enc_type_e)cmd_val->enc_type, (ehsm_sm9_padding_mode_e)cmd_val->padding,
        (uint8_t)cmd_val->key2_size, (uint8_t)cmd_val->hid, (const uint8_t *)cmd_val->kgc_pub_key,
        (const uint8_t *)cmd_val->input, (uint32_t)cmd_val->input_size, (uint8_t *)cmd_val->output,
        (uint32_t *)&cmd_val->output_size, (const uint8_t *)cmd_val->id, (uint32_t)cmd_val->id_size,
        (const uint8_t *)cmd_val->fp12g);

    sm9_cipher_rsp_st *rsp_val = (sm9_cipher_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp_val->output_size = cmd_val->output_size;
    rsp->len = sizeof(sm9_cipher_rsp_st);
    return RSP_OK;
}

typedef struct {
    uint32_t ctx;
    uint32_t key_handle;
    uint32_t msg;
    uint32_t msg_size;
    uint32_t sig;
    uint32_t sig_size;
    uint32_t kgc_pub_key;
    uint32_t fp12g;
} sm9_sign_onepass_gen_cmd_st;

typedef struct {
    uint32_t ret;
    uint32_t verify_result;
} sm9_sign_rsp_st;

uint16_t test_cmd_hostapi_sm9_sign_onepass_gen(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(sm9_sign_onepass_gen_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const sm9_sign_onepass_gen_cmd_st *cmd_val = (const sm9_sign_onepass_gen_cmd_st *)cmd->val;
    uint32_t ret = ehsm_sm9_sign_onepass_gen((ehsm_ctx_st *)cmd_val->ctx, (uint32_t)cmd_val->key_handle,
        (const uint8_t *)cmd_val->msg, (uint32_t)cmd_val->msg_size, (uint8_t *)cmd_val->sig,
        (uint32_t)cmd_val->sig_size, (const uint8_t *)cmd_val->kgc_pub_key, (const uint8_t *)cmd_val->fp12g);

    sm9_sign_rsp_st *rsp_val = (sm9_sign_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(sm9_sign_rsp_st);
    return RSP_OK;
}

typedef struct {
    uint32_t ctx;
    uint32_t msg;
    uint32_t msg_size;
    uint32_t id;
    uint32_t id_size;
    uint32_t hid;
    uint32_t kgc_pub_key;
    uint32_t fp12g;
    uint32_t sig;
    uint32_t sig_size;
} sm9_sign_onepass_verify_cmd_st;

uint16_t test_cmd_hostapi_sm9_sign_onepass_verify(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(sm9_sign_onepass_verify_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    uint32_t verify_result = 0;
    const sm9_sign_onepass_verify_cmd_st *cmd_val = (const sm9_sign_onepass_verify_cmd_st *)cmd->val;

    uint32_t ret = ehsm_sm9_sign_onepass_verify((ehsm_ctx_st *)cmd_val->ctx, (const uint8_t *)cmd_val->msg,
        (uint32_t)cmd_val->msg_size, (const uint8_t *)cmd_val->id, (uint32_t)cmd_val->id_size, (uint8_t)cmd_val->hid,
        (const uint8_t *)cmd_val->kgc_pub_key, (const uint8_t *)cmd_val->fp12g, (const uint8_t *)cmd_val->sig,
        (uint32_t)cmd_val->sig_size, (bool_t *)&verify_result);

    sm9_sign_rsp_st *rsp_val = (sm9_sign_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp_val->verify_result = verify_result;
    rsp->len = sizeof(sm9_sign_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_symm_cipher_onepass(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(symm_cipher_onepass_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const symm_cipher_onepass_cmd_st *cmd_val = (const symm_cipher_onepass_cmd_st *)cmd->val;

    uint32_t ret = ehsm_symm_cipher_onepass((ehsm_ctx_st *)cmd_val->ctx, (ehsm_symm_algo_e)cmd_val->algo,
        (ehsm_cipher_mode_e)cmd_val->mode, (ehsm_padding_mode_e)cmd_val->padding, cmd_val->key_handle,
        (bool_t)cmd_val->enc, (const uint8_t *)cmd_val->iv_addr, cmd_val->iv_size, (const uint8_t *)cmd_val->input_addr,
        cmd_val->input_size, (uint8_t *)cmd_val->output_addr, (uint32_t *)&cmd_val->outbuf_size);

    symm_cipher_onepass_rsp_st *rsp_val = (symm_cipher_onepass_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp_val->output_size = cmd_val->outbuf_size;
    rsp->len = sizeof(symm_cipher_onepass_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_symm_cipher_onepass_with_plain_key(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(symm_cipher_onepass_with_plain_key_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const symm_cipher_onepass_with_plain_key_cmd_st *cmd_val = (const symm_cipher_onepass_with_plain_key_cmd_st *)cmd->val;

    uint32_t ret = ehsm_symm_cipher_onepass_with_plain_key((ehsm_ctx_st *)cmd_val->ctx, (ehsm_symm_algo_e)cmd_val->algo,
        (ehsm_cipher_mode_e)cmd_val->mode, (ehsm_padding_mode_e)cmd_val->padding, (uint8_t *)cmd_val->key_addr, cmd_val->key_size,
        (bool_t)cmd_val->enc, (const uint8_t *)cmd_val->iv_addr, cmd_val->iv_size, (const uint8_t *)cmd_val->input_addr,
        cmd_val->input_size, (uint8_t *)cmd_val->output_addr, (uint32_t *)&cmd_val->outbuf_size);

    symm_cipher_onepass_rsp_st *rsp_val = (symm_cipher_onepass_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp_val->output_size = cmd_val->outbuf_size;
    rsp->len = sizeof(symm_cipher_onepass_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_symm_cipher_init(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(symm_cipher_init_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const symm_cipher_init_cmd_st *cmd_val = (const symm_cipher_init_cmd_st *)cmd->val;

    uint32_t ret = ehsm_symm_cipher_init((ehsm_ctx_st *)cmd_val->ctx, (ehsm_symm_algo_e)cmd_val->algo,
        (ehsm_cipher_mode_e)cmd_val->mode, (ehsm_padding_mode_e)cmd_val->padding, (uint32_t)cmd_val->key_handle,
        (bool_t)cmd_val->enc, (const uint8_t *)cmd_val->iv, (uint32_t)cmd_val->iv_size,
        (ehsm_session_st *)cmd_val->session);

    symm_cipher_init_rsp_st *rsp_val = (symm_cipher_init_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(symm_cipher_init_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_symm_cipher_init_with_plain_key(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(symm_cipher_init_with_plain_key_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const symm_cipher_init_with_plain_key_cmd_st *cmd_val = (const symm_cipher_init_with_plain_key_cmd_st *)cmd->val;

    uint32_t ret = ehsm_symm_cipher_init_with_plain_key((ehsm_ctx_st *)cmd_val->ctx, (ehsm_symm_algo_e)cmd_val->algo,
        (ehsm_cipher_mode_e)cmd_val->mode, (ehsm_padding_mode_e)cmd_val->padding, (const uint8_t *)cmd_val->key_addr,
        cmd_val->key_size, (bool_t)cmd_val->enc, (const uint8_t *)cmd_val->iv, cmd_val->iv_size,
        (ehsm_session_st *)cmd_val->session);

    symm_cipher_init_with_plain_key_rsp_st *rsp_val = (symm_cipher_init_with_plain_key_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(symm_cipher_init_with_plain_key_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_symm_cipher_update(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(symm_cipher_update_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const symm_cipher_update_cmd_st *cmd_val = (const symm_cipher_update_cmd_st *)cmd->val;

    uint32_t ret = ehsm_symm_cipher_update((ehsm_ctx_st *)cmd_val->ctx, (const uint8_t *)cmd_val->input,
        (uint32_t)cmd_val->input_size, (uint8_t *)cmd_val->output);

    symm_cipher_update_rsp_st *rsp_val = (symm_cipher_update_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(symm_cipher_update_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_symm_cipher_finish(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(symm_cipher_finish_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const symm_cipher_finish_cmd_st *cmd_val = (const symm_cipher_finish_cmd_st *)cmd->val;

    uint32_t ret = ehsm_symm_cipher_finish((ehsm_ctx_st *)cmd_val->ctx, (const uint8_t *)cmd_val->input,
        (uint32_t)cmd_val->input_size, (uint8_t *)cmd_val->output, (uint32_t *)&cmd_val->output_size);

    symm_cipher_finish_rsp_st *rsp_val = (symm_cipher_finish_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp_val->output_size = cmd_val->output_size;
    rsp->len = sizeof(symm_cipher_finish_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_upgrade_fw_image(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(upgrade_fw_image_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const upgrade_fw_image_cmd_st *cmd_val = (const upgrade_fw_image_cmd_st *)cmd->val;
    uint32_t ret = ehsm_upgrade_fw_image((ehsm_ctx_st *)cmd_val->ctx, (const uint8_t *)cmd_val->image,
        (uint32_t)cmd_val->image_size, (uint8_t *)cmd_val->image_out);

    upgrade_fw_image_rsp_st *rsp_val = (upgrade_fw_image_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(upgrade_fw_image_rsp_st);
    return RSP_OK;
}

// Three-stage upgrade firmware image functions
uint16_t test_cmd_hostapi_upgrade_fw_image_init(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(upgrade_fw_image_3stage_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const upgrade_fw_image_3stage_cmd_st *cmd_val = (const upgrade_fw_image_3stage_cmd_st *)cmd->val;
    uint32_t ret = ehsm_upgrade_fw_image_init(
        (ehsm_ctx_st *)cmd_val->ctx_addr,
        (const uint8_t *)cmd_val->input_addr,
        cmd_val->input_size,
        (uint8_t *)cmd_val->output_addr
    );

    upgrade_fw_image_3stage_rsp_st *rsp_val = (upgrade_fw_image_3stage_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(upgrade_fw_image_3stage_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_upgrade_fw_image_update(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(upgrade_fw_image_3stage_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const upgrade_fw_image_3stage_cmd_st *cmd_val = (const upgrade_fw_image_3stage_cmd_st *)cmd->val;
    uint32_t ret = ehsm_upgrade_fw_image_update(
        (ehsm_ctx_st *)cmd_val->ctx_addr,
        (const uint8_t *)cmd_val->input_addr,
        cmd_val->input_size,
        (uint8_t *)cmd_val->output_addr
    );

    upgrade_fw_image_3stage_rsp_st *rsp_val = (upgrade_fw_image_3stage_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(upgrade_fw_image_3stage_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_upgrade_fw_image_finish(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(upgrade_fw_image_3stage_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const upgrade_fw_image_3stage_cmd_st *cmd_val = (const upgrade_fw_image_3stage_cmd_st *)cmd->val;
    uint32_t ret = ehsm_upgrade_fw_image_finish(
        (ehsm_ctx_st *)cmd_val->ctx_addr,
        (const uint8_t *)cmd_val->input_addr,
        cmd_val->input_size,
        (uint8_t *)cmd_val->output_addr
    );

    upgrade_fw_image_3stage_rsp_st *rsp_val = (upgrade_fw_image_3stage_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(upgrade_fw_image_3stage_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_verify_image(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(verify_image_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const verify_image_cmd_st *cmd_val = (const verify_image_cmd_st *)cmd->val;
    uint32_t ret
        = ehsm_verify_image((ehsm_ctx_st *)cmd_val->ctx, (const uint8_t *)cmd_val->image, (uint32_t)cmd_val->image_size,
            (bool_t)cmd_val->check_version, (bool_t)cmd_val->boot, (uint8_t *)cmd_val->image_out);

    verify_image_rsp_st *rsp_val = (verify_image_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(verify_image_rsp_st);
    return RSP_OK;
}

typedef struct {
    uint32_t ctx;
    uint32_t img_size;
    uint32_t img_data;
    uint32_t trusted_fw_nv_ctr_in_otp;
    uint32_t non_trusted_fw_nv_ctr_in_otp;
    uint32_t trusted_fw_nv_ctr_in_cert;
    uint32_t non_trusted_fw_nv_ctr_in_otp_cert;
} verify_tbbr_img_cmd_st;

typedef struct {
    uint32_t ret;
    uint32_t trusted_fw_nv_ctr;
    uint32_t non_trusted_fw_nv_ctr;
} verify_tbbr_img_rsp_st;

uint16_t test_cmd_hostapi_verify_tbbr_img(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(verify_tbbr_img_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const verify_tbbr_img_cmd_st *cmd_val = (const verify_tbbr_img_cmd_st *)cmd->val;
    uint32_t ret
        = ehsm_verify_tbbr_img((ehsm_ctx_st *)cmd_val->ctx, (uint32_t)cmd_val->img_size, (uint8_t *)cmd_val->img_data,
            (uint32_t)cmd_val->trusted_fw_nv_ctr_in_otp, (uint32_t)cmd_val->non_trusted_fw_nv_ctr_in_otp,
            (uint32_t *)cmd_val->trusted_fw_nv_ctr_in_cert, (uint32_t *)cmd_val->non_trusted_fw_nv_ctr_in_otp_cert);

    verify_tbbr_img_rsp_st *rsp_val = (verify_tbbr_img_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(verify_tbbr_img_rsp_st);
    return RSP_OK;
}

typedef struct {
    uint32_t ctx;
    uint32_t src_data;
    uint32_t ehsm_dest_addr;
    uint32_t size;
} otp_cmd_st;

typedef struct {
    uint32_t ret;
} otp_rsp_st;

uint16_t test_cmd_hostapi_write_otp(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(otp_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const otp_cmd_st *cmd_val = (const otp_cmd_st *)cmd->val;
    uint32_t ret = ehsm_write_otp((ehsm_ctx_st *)cmd_val->ctx, (const uint8_t *)cmd_val->src_data,
        (uint32_t)cmd_val->ehsm_dest_addr, (uint32_t)cmd_val->size);

    otp_rsp_st *rsp_val = (otp_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(otp_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_write_reg(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(otp_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const otp_cmd_st *cmd_val = (const otp_cmd_st *)cmd->val;
    uint32_t ret = ehsm_write_reg((ehsm_ctx_st *)cmd_val->ctx, (const uint8_t *)cmd_val->src_data,
        (uint32_t)cmd_val->ehsm_dest_addr, (uint32_t)cmd_val->size);

    otp_rsp_st *rsp_val = (otp_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(otp_rsp_st);
    return RSP_OK;
}

typedef struct {
    uint32_t ctx;
    uint32_t key_type;
    uint32_t privilege;
    uint32_t rsa_e_bit_size;
    uint32_t hmac_key_size;
    uint32_t dh_params_addr;
    uint32_t dh_params_size;
    uint32_t key_handle;
} gen_key_cmd_st;

typedef struct {
    uint32_t ret;
    uint32_t key_handle;
} gen_key_rsp_st;

typedef struct {
    uint32_t ctx;
    uint32_t trans_key_handle;
    uint32_t auth_key_handle;
    uint32_t key_data_addr;
    uint32_t key_data_size;
    uint32_t mac_addr;
    uint32_t mac_size;
    uint32_t key_handle;
} import_key_cmd_st;

typedef struct {
    uint32_t ret;
    uint32_t key_handle;
} import_key_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t hash_algo;
    uint32_t key_handle;
    uint32_t padding;
    uint32_t msg_addr;
    uint32_t msg_size;
    uint32_t sig_addr;
    uint32_t sig_size;
    uint32_t salt_size;
} rsa_sign_onepass_cmd_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t hash_algo;
    uint32_t key_handle;
    uint32_t padding;
    uint32_t digest_addr;
    uint32_t digest_size;
    uint32_t sig_addr;
    uint32_t sig_size;
    uint32_t salt_size;
} rsa_sign_onepass_with_digest_cmd_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t algo;
    uint32_t key_handle;
    uint32_t gen_sig;
    uint32_t padding;
    uint32_t session;
} rsa_sign_init_cmd_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t msg;
    uint32_t msg_size;
} rsa_sign_update_cmd_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t sig;
    uint32_t sig_size;
    uint32_t salt_size;
} rsa_sign_finish_gen_cmd_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t sig;
    uint32_t sig_size;
    uint32_t salt_size;
} rsa_sign_finish_verify_cmd_st;

typedef struct {
    uint32_t ret;
    uint32_t sign_size;
    uint32_t verification;
} rsa_sign_rsp_st;

uint16_t test_cmd_hostapi_rsa_sign_onepass_gen(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(rsa_sign_onepass_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const rsa_sign_onepass_cmd_st *cmd_val = (const rsa_sign_onepass_cmd_st *)cmd->val;
    uint32_t sig_size = cmd_val->sig_size;

    uint32_t ret = ehsm_rsa_sign_onepass_gen((ehsm_ctx_st *)cmd_val->ctx_addr, (ehsm_hash_algo_e)cmd_val->hash_algo,
        cmd_val->key_handle, (ehsm_rsa_padding_mode_e)cmd_val->padding, (const uint8_t *)cmd_val->msg_addr,
        cmd_val->msg_size, (uint8_t *)cmd_val->sig_addr, &sig_size, cmd_val->salt_size);

    rsa_sign_rsp_st *rsp_data = (rsa_sign_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->sign_size = sig_size;
    rsp->len = sizeof(rsa_sign_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_rsa_sign_onepass_verify(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(rsa_sign_onepass_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const rsa_sign_onepass_cmd_st *cmd_val = (const rsa_sign_onepass_cmd_st *)cmd->val;
    uint32_t sig_size = cmd_val->sig_size;

    uint32_t verify_result = 0;
    uint32_t ret = ehsm_rsa_sign_onepass_verify((ehsm_ctx_st *)cmd_val->ctx_addr, (ehsm_hash_algo_e)cmd_val->hash_algo,
        cmd_val->key_handle, (ehsm_rsa_padding_mode_e)cmd_val->padding, (const uint8_t *)cmd_val->msg_addr,
        cmd_val->msg_size, (uint8_t *)cmd_val->sig_addr, sig_size, cmd_val->salt_size,
        (bool_t *)&verify_result);

    rsa_sign_rsp_st *rsp_data = (rsa_sign_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->verification = verify_result;
    rsp->len = sizeof(rsa_sign_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_rsa_sign_onepass_gen_with_digest(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(rsa_sign_onepass_with_digest_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const rsa_sign_onepass_with_digest_cmd_st *cmd_val = (const rsa_sign_onepass_with_digest_cmd_st *)cmd->val;
    uint32_t sig_size = cmd_val->sig_size;

    uint32_t ret = ehsm_rsa_sign_onepass_gen_with_digest((ehsm_ctx_st *)cmd_val->ctx_addr,(ehsm_hash_algo_e)cmd_val->hash_algo,
        cmd_val->key_handle, (ehsm_rsa_padding_mode_e)cmd_val->padding, (const uint8_t *)cmd_val->digest_addr,
        cmd_val->digest_size, (uint8_t *)cmd_val->sig_addr, &sig_size, cmd_val->salt_size);

    rsa_sign_rsp_st *rsp_data = (rsa_sign_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->sign_size = sig_size;
    rsp->len = sizeof(rsa_sign_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_rsa_sign_onepass_verify_with_digest(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(rsa_sign_onepass_with_digest_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    uint32_t verify_result = 0;
    const rsa_sign_onepass_with_digest_cmd_st *cmd_val = (const rsa_sign_onepass_with_digest_cmd_st *)cmd->val;

    uint32_t ret = ehsm_rsa_sign_onepass_verify_with_digest((ehsm_ctx_st *)cmd_val->ctx_addr,(ehsm_hash_algo_e)cmd_val->hash_algo,
        cmd_val->key_handle, (ehsm_rsa_padding_mode_e)cmd_val->padding, (const uint8_t *)cmd_val->digest_addr,
        cmd_val->digest_size, (uint8_t *)cmd_val->sig_addr, cmd_val->sig_size, cmd_val->salt_size,
        (bool_t *)&verify_result);

    rsa_sign_rsp_st *rsp_data = (rsa_sign_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->verification = verify_result;
    rsp->len = sizeof(rsa_sign_rsp_st);
    return RSP_OK;
}


typedef struct {
    uint32_t ctx;
    uint32_t algo;
    uint32_t use_plain_key;
    uint32_t key_handle;
    uint32_t key;
    uint32_t padding;
    uint32_t gen_sig;
    uint32_t is_digest;
    uint32_t input_addr;
    uint32_t input_size;
    uint32_t sig_addr;
    uint32_t sig_size;
    uint32_t salt_size;
    uint32_t verify_result_addr;
} rsa_sign_onepass_ex_cmd_st;

uint16_t test_cmd_hostapi_rsa_sign_onepass_ex(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(rsa_sign_onepass_ex_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const rsa_sign_onepass_ex_cmd_st *cmd_val = (const rsa_sign_onepass_ex_cmd_st *)cmd->val;
    uint32_t sig_size = cmd_val->sig_size;
    bool_t verify_result = false;

    uint32_t ret = ehsm_rsa_sign_onepass_ex(
        (ehsm_ctx_st *)cmd_val->ctx,
        (ehsm_hash_algo_e)cmd_val->algo,
        (bool_t)cmd_val->use_plain_key,
        cmd_val->key_handle,
        (const uint8_t *)cmd_val->key,
        (ehsm_rsa_padding_mode_e)cmd_val->padding,
        (bool_t)cmd_val->gen_sig,
        (bool_t)cmd_val->is_digest,
        (const uint8_t *)cmd_val->input_addr,
        cmd_val->input_size,
        (uintptr_t)cmd_val->sig_addr,
        &sig_size,
        cmd_val->salt_size,
        &verify_result
    );

    // Write verify_result to shared memory
    if (!cmd_val->gen_sig && cmd_val->verify_result_addr != 0) {
        *(uint32_t *)cmd_val->verify_result_addr = (uint32_t)verify_result;
    }

    rsa_sign_rsp_st *rsp_data = (rsa_sign_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->sign_size = sig_size;
    rsp_data->verification = verify_result;
    rsp->len = sizeof(rsa_sign_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_rsa_sign_init(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(rsa_sign_init_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const rsa_sign_init_cmd_st *cmd_val = (const rsa_sign_init_cmd_st *)cmd->val;
    uint32_t ret = ehsm_rsa_sign_init((ehsm_ctx_st *)cmd_val->ctx_addr, (ehsm_hash_algo_e)cmd_val->algo,
        (uint32_t)cmd_val->key_handle, (bool_t)cmd_val->gen_sig, (ehsm_rsa_padding_mode_e)cmd_val->padding,
        (ehsm_session_st *)cmd_val->session);

    rsa_sign_rsp_st *rsp_val = (rsa_sign_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(rsa_sign_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_rsa_sign_update(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(rsa_sign_update_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const rsa_sign_update_cmd_st *cmd_val = (const rsa_sign_update_cmd_st *)cmd->val;
    uint32_t ret = ehsm_rsa_sign_update(
        (ehsm_ctx_st *)cmd_val->ctx_addr, (const uint8_t *)cmd_val->msg, (uint32_t)cmd_val->msg_size);

    rsa_sign_rsp_st *rsp_val = (rsa_sign_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(rsa_sign_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_rsa_sign_finish_gen(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(rsa_sign_finish_gen_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const rsa_sign_finish_gen_cmd_st *cmd_val = (const rsa_sign_finish_gen_cmd_st *)cmd->val;
    uint32_t sig_size = cmd_val->sig_size;
    uint32_t ret = ehsm_rsa_sign_finish_gen((ehsm_ctx_st *)cmd_val->ctx_addr, (uint8_t *)cmd_val->sig,
        &sig_size, (uint32_t)cmd_val->salt_size);

    rsa_sign_rsp_st *rsp_val = (rsa_sign_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(rsa_sign_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_rsa_sign_finish_verify(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(rsa_sign_finish_verify_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const rsa_sign_finish_verify_cmd_st *cmd_val = (const rsa_sign_finish_verify_cmd_st *)cmd->val;

    uint32_t verify_result = 0;
    uint32_t ret = ehsm_rsa_sign_finish_verify((ehsm_ctx_st *)cmd_val->ctx_addr, (const uint8_t *)cmd_val->sig,
        (uint32_t)cmd_val->sig_size, (uint32_t)cmd_val->salt_size, (bool_t *)&verify_result);

    rsa_sign_rsp_st *rsp_val = (rsa_sign_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp_val->verification = verify_result;
    rsp->len = sizeof(rsa_sign_rsp_st);
    return RSP_OK;
}


typedef struct {
    uint32_t ctx_addr;
    uint32_t hash_algo;
    uint32_t key_handle;
    uint32_t msg_addr;
    uint32_t msg_size;
    uint32_t sig_addr;
    uint32_t sig_size;
} ecdsa_cmd_st;

typedef struct {
    uint32_t ret;
    uint32_t sig_size;
    uint32_t verify_result;
} ecdsa_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t image_addr;
    uint32_t image_size;
    uint32_t check_version;
    uint32_t boot_after_verify;
    uint32_t image_out_addr;
} bl_verify_image_st;

typedef struct {
    uint32_t ret;
} bl_verify_image_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t image_addr;
    uint32_t image_size;
    uint32_t code_addr;
    uint32_t only_copy_code;
    uint32_t check_version;
    uint32_t boot_after_verify;
    uint32_t image_out_addr;
} bl_verify_image_discrete_st;

typedef struct {
    uint32_t ret;
} bl_verify_image_discrete_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t image_addr;
    uint32_t image_size;
    uint32_t image_out_addr;
} bl_fw_upgrade_st;

typedef struct {
    uint32_t ret;
} bl_fw_upgrade_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t key_handle;
    uint32_t enc;
    uint32_t input_addr;
    uint32_t input_size;
    uint32_t output_addr;
    uint32_t output_size;
} sm2_cipher_st;

typedef struct {
    uint32_t ret;
    uint32_t ouput_size;
} sm2_cipher_rsp_st;

uint16_t test_cmd_hostapi_sm2_cipher(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(sm2_cipher_st)) {
        return RSP_ERR_DATA_LENGTH;
    }
    sm2_cipher_st *req = (sm2_cipher_st *)cmd->val;
    uint32_t ret = ehsm_sm2_cipher((ehsm_ctx_st *)req->ctx_addr, req->key_handle, (bool_t)req->enc,
        (const uint8_t *)req->input_addr, req->input_size, (uint8_t *)req->output_addr, (uint32_t *)&req->output_size);
    sm2_cipher_rsp_st *rsp_data = (sm2_cipher_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->ouput_size = req->output_size;
    rsp->len = sizeof(sm2_cipher_rsp_st);
    return RSP_OK;
}

typedef struct {
    uint32_t ctx_addr;
    uint32_t key_handle;
    uint32_t enc;
    uint32_t input_addr;
    uint32_t input_size;
    uint32_t output_addr;
    uint32_t output_size;
} rsa_cipher_st;

typedef struct {
    uint32_t ret;
    uint32_t ouput_size;
} rsa_cipher_rsp_st;

uint16_t test_cmd_hostapi_rsa_cipher(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(rsa_cipher_st)) {
        return RSP_ERR_DATA_LENGTH;
    }
    rsa_cipher_st *req = (rsa_cipher_st *)cmd->val;
    uint32_t output_size = req->output_size;
    uint32_t ret = ehsm_rsa_cipher((ehsm_ctx_st *)req->ctx_addr, req->key_handle, (bool_t)req->enc,
        (const uint8_t *)req->input_addr, req->input_size, (uint8_t *)req->output_addr, &output_size);
    rsa_cipher_rsp_st *rsp_data = (rsa_cipher_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->ouput_size = output_size;
    rsp->len = sizeof(rsa_cipher_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_bl_close_debug(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(close_debug_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }
    const close_debug_cmd_st *val = (const close_debug_cmd_st *)cmd->val;
    uint32_t ret = ehsm_bl_close_debug((ehsm_ctx_st *)val->ctx_addr, (ehsm_challenge_type_e)val->type,
        (const ehsm_soc_dbg_bitmap_st *)val->soc_dbg_bitmap);
    close_debug_rsp_st *rsp_val = (close_debug_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(close_debug_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_bl_debug_auth(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(debug_auth_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const debug_auth_cmd_st *val = (const debug_auth_cmd_st *)cmd->val;
    uint32_t ret = ehsm_bl_debug_auth((ehsm_ctx_st *)val->ctx_addr, (ehsm_challenge_type_e)val->challenge_type,
        (ehsm_auth_algo_e)val->algo, (const uint8_t *)val->sig, val->sig_size, (uint8_t *)val->pub_key,
        val->pub_key_size, (const ehsm_soc_dbg_bitmap_st *)val->soc_dbg_bitmap);

    debug_auth_rsp_st *rsp_val = (debug_auth_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(debug_auth_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_bl_get_challenge(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(challenge_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const challenge_cmd_st *val = (const challenge_cmd_st *)cmd->val;
    uint32_t ret = ehsm_bl_get_challenge(
        (ehsm_ctx_st *)val->ctx_addr, (ehsm_challenge_type_e)val->challenge_type, (uint8_t *)val->output);

    challenge_rsp_st *rsp_val = (challenge_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(challenge_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_bl_get_version(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != 8) {
        return RSP_ERR_DATA_LENGTH;
    }
    uint32_t ctx_addr = *((uint32_t *)&cmd->val[0]);
    uint32_t version = *((uint32_t *)&cmd->val[4]);

    uint32_t ret = ehsm_get_version((ehsm_ctx_st *)ctx_addr, (ehsm_version_st *)version);
    rsp->len = 4;
    memcpy(rsp->val, &ret, 4);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_bl_inject_error(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != 8) {
        return RSP_ERR_DATA_LENGTH;
    }
    uint32_t ctx_addr = *((uint32_t *)&cmd->val[0]);
    uint32_t values = *((uint32_t *)&cmd->val[4]);

    uint32_t ret = ehsm_inject_error((ehsm_ctx_st *)ctx_addr, (ehsm_inject_error_st *)values);
    rsp->len = 4;
    memcpy(rsp->val, &ret, 4);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_bl_read_otp(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(read_otp_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }
    read_otp_cmd_st *req = (read_otp_cmd_st *)cmd->val;
    uint8_t *buf = (uint8_t *)req->buf;
    uint32_t ret = ehsm_bl_read_otp((ehsm_ctx_st *)req->ctx_addr, buf, req->ehsm_src_addr, req->size);
    read_otp_rsp_st *rsp_data = (read_otp_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(read_otp_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_bl_read_reg(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(read_reg_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }
    read_reg_cmd_st *req = (read_reg_cmd_st *)cmd->val;
    uint8_t *buf = (uint8_t *)req->buf;
    uint32_t ret = ehsm_bl_read_reg((ehsm_ctx_st *)req->ctx_addr, buf, req->ehsm_src_addr, req->size);
    read_reg_rsp_st *rsp_data = (read_reg_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(read_otp_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_bl_set_uart_baudrate(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != 8) {
        return RSP_ERR_DATA_LENGTH;
    }
    uint32_t ctx_addr = *((uint32_t *)&cmd->val[0]);
    uint32_t baud_div = *((uint32_t *)&cmd->val[4]);

    uint32_t ret = ehsm_set_uart_baudrate((ehsm_ctx_st *)ctx_addr, baud_div);
    read_otp_rsp_st *rsp_data = (read_otp_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(read_otp_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_bl_set_hsm_freq(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != 8) {
        return RSP_ERR_DATA_LENGTH;
    }
    uint32_t ctx_addr = *((uint32_t *)&cmd->val[0]);
    uint32_t freq = *((uint32_t *)&cmd->val[4]);

    uint32_t ret = ehsm_bl_set_hsm_freq((ehsm_ctx_st *)ctx_addr, freq);
    read_otp_rsp_st *rsp_data = (read_otp_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(read_otp_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_bl_upgrade_fw_image(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(bl_fw_upgrade_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    bl_fw_upgrade_st *req = (bl_fw_upgrade_st *)cmd->val;
    uint32_t ret = ehsm_upgrade_fw_image((ehsm_ctx_st *)req->ctx_addr, (const uint8_t *)req->image_addr,
        req->image_size, (uint8_t *)req->image_out_addr);
    bl_fw_upgrade_rsp_st *rsp_data = (bl_fw_upgrade_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(bl_fw_upgrade_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_bl_verify_image(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(bl_verify_image_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    bl_verify_image_st *req = (bl_verify_image_st *)cmd->val;
    uint32_t ret = ehsm_verify_image((ehsm_ctx_st *)req->ctx_addr, (const uint8_t *)req->image_addr, req->image_size,
        (bool_t)req->check_version, (bool_t)req->boot_after_verify, (uint8_t *)req->image_out_addr);
    bl_verify_image_rsp_st *rsp_data = (bl_verify_image_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(bl_verify_image_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_bl_verify_image_discrete(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(bl_verify_image_discrete_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    bl_verify_image_discrete_st *req = (bl_verify_image_discrete_st *)cmd->val;
    const uint8_t *code = req->code_addr ? (const uint8_t *)req->code_addr : NULL;

    uint32_t ret = ehsm_bl_verify_image_discrete(
        (ehsm_ctx_st *)req->ctx_addr,
        (const uint8_t *)req->image_addr,
        req->image_size,
        code,
        (bool_t)req->only_copy_code,
        (bool_t)req->check_version,
        (bool_t)req->boot_after_verify,
        (uint8_t *)req->image_out_addr
    );

    bl_verify_image_discrete_rsp_st *rsp_data = (bl_verify_image_discrete_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(bl_verify_image_discrete_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_bl_write_otp(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(write_otp_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    write_otp_cmd_st *req = (write_otp_cmd_st *)cmd->val;
    uint32_t ret = ehsm_bl_write_otp(
        (ehsm_ctx_st *)req->ctx_addr, (const uint8_t *)req->src_data, req->ehsm_dest_addr, req->size);
    write_otp_rsp_st *rsp_data = (write_otp_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(write_otp_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_bl_write_reg(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(write_reg_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    write_reg_cmd_st *req = (write_reg_cmd_st *)cmd->val;
    uint32_t ret = ehsm_bl_write_reg(
        (ehsm_ctx_st *)req->ctx_addr, (const uint8_t *)req->src_data, req->ehsm_dest_addr, req->size);
    write_reg_rsp_st *rsp_data = (write_reg_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(write_reg_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_bl_encrypt_key(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(encrypt_key_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    encrypt_key_cmd_st *req = (encrypt_key_cmd_st *)cmd->val;
    uint32_t ret = ehsm_bl_encrypt_key((ehsm_ctx_st *)req->ctx_addr, (ehsm_key_level_e)req->key_level,
        (const uint8_t *)req->input_data, req->size, (uint8_t *)req->key_out);
    encrypt_key_rsp_st *rsp_data = (encrypt_key_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(encrypt_key_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_bl_fw_auth(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(fw_auth_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    fw_auth_cmd_st *req = (fw_auth_cmd_st *)cmd->val;
    uint32_t ret = ehsm_bl_fw_auth((ehsm_ctx_st *)req->ctx_addr, (uint8_t)req->type, (const uint8_t *)req->arg,
        (const uint8_t *)req->auth_data, (uint8_t *)req->out_addr);
    fw_auth_rsp_st *rsp_data = (fw_auth_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(fw_auth_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_bl_get_random_key(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(random_key_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    random_key_cmd_st *req = (random_key_cmd_st *)cmd->val;
    uint32_t ret = ehsm_bl_get_random_key((ehsm_ctx_st *)req->ctx_addr, (ehsm_key_level_e)req->key_level,
        (ehsm_bl_gen_key_type_e)req->key_type, (uint8_t *)req->key_out);
    random_key_rsp_st *rsp_data = (random_key_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(random_key_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_bl_get_self_test_result(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != 8) {
        return RSP_ERR_DATA_LENGTH;
    }
    uint32_t ctx_addr = *((uint32_t *)&cmd->val[0]);
    uint32_t result = *((uint32_t *)&cmd->val[4]);

    uint32_t ret = ehsm_bl_get_self_test_result((ehsm_ctx_st *)ctx_addr, (ehsm_self_test_result_st *)result);
    rsp->len = 4;
    memcpy(rsp->val, &ret, 4);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_bl_get_socid(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != 8) {
        return RSP_ERR_DATA_LENGTH;
    }
    uint32_t ctx_addr = *((uint32_t *)&cmd->val[0]);
    uint32_t socid = *((uint32_t *)&cmd->val[4]);

    uint32_t ret = ehsm_bl_get_socid((ehsm_ctx_st *)ctx_addr, (uint8_t *)socid);
    rsp->len = 4;
    memcpy(rsp->val, &ret, 4);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_bl_self_test(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != 4) {
        return RSP_ERR_DATA_LENGTH;
    }
    uint32_t ctx_addr = *((uint32_t *)&cmd->val[0]);

    uint32_t ret = ehsm_bl_self_test((ehsm_ctx_st *)ctx_addr);
    rsp->len = 4;
    memcpy(rsp->val, &ret, 4);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_aead_onepass_enc(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(aead_onepass_enc_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const aead_onepass_enc_cmd_st *cmd_val = (const aead_onepass_enc_cmd_st *)cmd->val;

    uint32_t ret = ehsm_aead_onepass_enc((ehsm_ctx_st *)cmd_val->ctx_addr, (ehsm_symm_algo_e)cmd_val->algo,
        (ehsm_aead_mode_e)cmd_val->mode, cmd_val->key_handle, (const uint8_t *)cmd_val->nonce, cmd_val->nonce_size,
        (const uint8_t *)cmd_val->aad, cmd_val->aad_size, (const uint8_t *)cmd_val->input, cmd_val->input_size,
        (uint8_t *)cmd_val->output, (uint8_t *)cmd_val->tag, cmd_val->tag_size);

    aead_onepass_enc_rsp_st *rsp_data = (aead_onepass_enc_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(aead_onepass_enc_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_aead_onepass_enc_with_plain_key(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(aead_onepass_enc_with_plain_key_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const aead_onepass_enc_with_plain_key_cmd_st *cmd_val = (const aead_onepass_enc_with_plain_key_cmd_st *)cmd->val;

    uint32_t ret = ehsm_aead_onepass_enc_with_plain_key((ehsm_ctx_st *)cmd_val->ctx_addr, (ehsm_symm_algo_e)cmd_val->algo,
        (ehsm_aead_mode_e)cmd_val->mode, (const uint8_t *)cmd_val->key_addr, cmd_val->key_size,
        (const uint8_t *)cmd_val->nonce_addr, cmd_val->nonce_size, (const uint8_t *)cmd_val->aad_addr, cmd_val->aad_size,
        (const uint8_t *)cmd_val->input_addr, cmd_val->input_size, (uint8_t *)cmd_val->output_addr,
        (uint8_t *)cmd_val->tag_addr, cmd_val->tag_size);

    aead_onepass_enc_with_plain_key_rsp_st *rsp_data = (aead_onepass_enc_with_plain_key_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(aead_onepass_enc_with_plain_key_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_aead_onepass_dec(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(aead_onepass_dec_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    uint32_t verify_result = 0;
    const aead_onepass_dec_cmd_st *cmd_val = (const aead_onepass_dec_cmd_st *)cmd->val;

    uint32_t ret = ehsm_aead_onepass_dec((ehsm_ctx_st *)cmd_val->ctx_addr, (ehsm_symm_algo_e)cmd_val->algo,
        (ehsm_aead_mode_e)cmd_val->mode, cmd_val->key_handle, (const uint8_t *)cmd_val->nonce, cmd_val->nonce_size,
        (const uint8_t *)cmd_val->aad, cmd_val->aad_size, (const uint8_t *)cmd_val->input, cmd_val->input_size,
        (uint8_t *)cmd_val->output, (const uint8_t *)cmd_val->tag, cmd_val->tag_size, (bool_t *)&verify_result);

    aead_onepass_dec_rsp_st *rsp_data = (aead_onepass_dec_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->verify_result = verify_result;
    rsp->len = sizeof(aead_onepass_dec_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_aead_onepass_dec_with_plain_key(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(aead_onepass_dec_with_plain_key_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    uint32_t verify_result = 0;
    const aead_onepass_dec_with_plain_key_cmd_st *cmd_val = (const aead_onepass_dec_with_plain_key_cmd_st *)cmd->val;

    uint32_t ret = ehsm_aead_onepass_dec_with_plain_key((ehsm_ctx_st *)cmd_val->ctx_addr, (ehsm_symm_algo_e)cmd_val->algo,
        (ehsm_aead_mode_e)cmd_val->mode, (const uint8_t *)cmd_val->key_addr, cmd_val->key_size,
        (const uint8_t *)cmd_val->nonce_addr, cmd_val->nonce_size, (const uint8_t *)cmd_val->aad_addr, cmd_val->aad_size,
        (const uint8_t *)cmd_val->input_addr, cmd_val->input_size, (uint8_t *)cmd_val->output_addr,
        (const uint8_t *)cmd_val->tag_addr, cmd_val->tag_size, (bool_t *)&verify_result);

    aead_onepass_dec_with_plain_key_rsp_st *rsp_data = (aead_onepass_dec_with_plain_key_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->verify_result = verify_result;
    rsp->len = sizeof(aead_onepass_dec_with_plain_key_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_aead_init(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(aead_init_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const aead_init_cmd_st *cmd_val = (const aead_init_cmd_st *)cmd->val;

    uint32_t ret = ehsm_aead_init((ehsm_ctx_st *)cmd_val->ctx_addr, (ehsm_symm_algo_e)cmd_val->algo,
        (ehsm_aead_mode_e)cmd_val->mode, cmd_val->key_handle, (bool)cmd_val->enc, (const uint8_t *)cmd_val->nonce,
        cmd_val->nonce_size, (const uint8_t *)cmd_val->aad, cmd_val->aad_size, cmd_val->data_size, cmd_val->tag_size,
        (ehsm_session_st *)cmd_val->session);

    aead_init_rsp_st *rsp_data = (aead_init_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(aead_init_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_aead_init_with_plain_key(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(aead_init_with_plain_key_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const aead_init_with_plain_key_cmd_st *cmd_val = (const aead_init_with_plain_key_cmd_st *)cmd->val;

    uint32_t ret = ehsm_aead_init_with_plain_key((ehsm_ctx_st *)cmd_val->ctx_addr, (ehsm_symm_algo_e)cmd_val->algo,
        (ehsm_aead_mode_e)cmd_val->mode, (const uint8_t *)cmd_val->key_addr, cmd_val->key_size,
        (bool)cmd_val->enc, (const uint8_t *)cmd_val->nonce_addr, cmd_val->nonce_size,
        (const uint8_t *)cmd_val->aad_addr, cmd_val->aad_size, cmd_val->data_size, cmd_val->tag_size,
        (ehsm_session_st *)cmd_val->session);

    aead_init_with_plain_key_rsp_st *rsp_data = (aead_init_with_plain_key_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(aead_init_with_plain_key_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_chacha_onepass_enc_with_plain_key(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(chacha_onepass_enc_with_plain_key_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const chacha_onepass_enc_with_plain_key_cmd_st *cmd_val = (const chacha_onepass_enc_with_plain_key_cmd_st *)cmd->val;

    uint32_t ret = ehsm_chacha_onepass_enc_with_plain_key((ehsm_ctx_st *)cmd_val->ctx_addr,
        (const uint8_t *)cmd_val->key_addr, cmd_val->key_size,
        (const uint8_t *)cmd_val->nonce_addr, cmd_val->nonce_size,
        (const uint8_t *)cmd_val->aad_addr, cmd_val->aad_size,
        cmd_val->constant,
        (const uint8_t *)cmd_val->input_addr, cmd_val->input_size,
        (uint8_t *)cmd_val->output_addr,
        (uint8_t *)cmd_val->tag_addr, cmd_val->tag_size);

    chacha_onepass_enc_with_plain_key_rsp_st *rsp_data = (chacha_onepass_enc_with_plain_key_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(chacha_onepass_enc_with_plain_key_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_chacha_onepass_dec_with_plain_key(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(chacha_onepass_dec_with_plain_key_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    uint32_t verify_result = 0;
    const chacha_onepass_dec_with_plain_key_cmd_st *cmd_val = (const chacha_onepass_dec_with_plain_key_cmd_st *)cmd->val;

    uint32_t ret = ehsm_chacha_onepass_dec_with_plain_key((ehsm_ctx_st *)cmd_val->ctx_addr,
        (const uint8_t *)cmd_val->key_addr, cmd_val->key_size,
        (const uint8_t *)cmd_val->nonce_addr, cmd_val->nonce_size,
        (const uint8_t *)cmd_val->aad_addr, cmd_val->aad_size,
        cmd_val->constant,
        (const uint8_t *)cmd_val->input_addr, cmd_val->input_size,
        (uint8_t *)cmd_val->output_addr,
        (const uint8_t *)cmd_val->tag_addr, cmd_val->tag_size, (bool_t *)&verify_result);

    chacha_onepass_dec_with_plain_key_rsp_st *rsp_data = (chacha_onepass_dec_with_plain_key_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->verify_result = verify_result;
    rsp->len = sizeof(chacha_onepass_dec_with_plain_key_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_chacha_init_with_plain_key(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(chacha_init_with_plain_key_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const chacha_init_with_plain_key_cmd_st *cmd_val = (const chacha_init_with_plain_key_cmd_st *)cmd->val;

    uint32_t ret = ehsm_chacha_init_with_plain_key((ehsm_ctx_st *)cmd_val->ctx_addr,
        (const uint8_t *)cmd_val->key_addr, cmd_val->key_size,
        (bool)cmd_val->enc,
        (const uint8_t *)cmd_val->nonce_addr, cmd_val->nonce_size,
        (const uint8_t *)cmd_val->aad_addr, cmd_val->aad_size,
        cmd_val->constant,
        (ehsm_session_st *)cmd_val->session);

    chacha_init_with_plain_key_rsp_st *rsp_data = (chacha_init_with_plain_key_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(chacha_init_with_plain_key_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_aead_update(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(aead_update_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const aead_update_cmd_st *cmd_val = (const aead_update_cmd_st *)cmd->val;

    uint32_t ret = ehsm_aead_update((ehsm_ctx_st *)cmd_val->ctx_addr, (const uint8_t *)cmd_val->input,
        cmd_val->input_size, (uint8_t *)cmd_val->output);

    aead_update_rsp_st *rsp_data = (aead_update_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(aead_update_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_aead_finish_enc(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(aead_finish_enc_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const aead_finish_enc_cmd_st *cmd_val = (const aead_finish_enc_cmd_st *)cmd->val;

    uint32_t ret = ehsm_aead_finish_enc((ehsm_ctx_st *)cmd_val->ctx_addr, (const uint8_t *)cmd_val->input,
        cmd_val->input_size, (uint8_t *)cmd_val->output, (uint8_t *)cmd_val->tag);

    aead_finish_enc_rsp_st *rsp_data = (aead_finish_enc_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(aead_finish_enc_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_aead_finish_dec(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(aead_finish_dec_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    uint32_t verify_result = 0;
    const aead_finish_dec_cmd_st *cmd_val = (const aead_finish_dec_cmd_st *)cmd->val;

    uint32_t ret = ehsm_aead_finish_dec((ehsm_ctx_st *)cmd_val->ctx_addr, (const uint8_t *)cmd_val->input,
        cmd_val->input_size, (uint8_t *)cmd_val->output, (const uint8_t *)cmd_val->tag, (bool_t *)&verify_result);

    aead_finish_dec_rsp_st *rsp_data = (aead_finish_dec_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->verify_result = verify_result;
    rsp->len = sizeof(aead_finish_dec_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_chacha_onepass_enc(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(chacha_onepass_enc_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const chacha_onepass_enc_cmd_st *cmd_val = (const chacha_onepass_enc_cmd_st *)cmd->val;

    uint32_t ret = ehsm_chacha_onepass_enc((ehsm_ctx_st *)cmd_val->ctx_addr, cmd_val->key_handle,
        (uint8_t *)cmd_val->nonce, cmd_val->nonce_size, (const uint8_t *)cmd_val->aad, cmd_val->aad_size,
        cmd_val->constant, (const uint8_t *)cmd_val->input, cmd_val->input_size, (uint8_t *)cmd_val->output,
        (uint8_t *)cmd_val->tag, cmd_val->tag_size);

    chacha_onepass_enc_rsp_st *rsp_data = (chacha_onepass_enc_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(chacha_onepass_enc_rsp_st);
    return RSP_OK;
}


uint16_t test_cmd_hostapi_chacha_onepass_dec(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(chacha_onepass_dec_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const chacha_onepass_dec_cmd_st *cmd_val = (const chacha_onepass_dec_cmd_st *)cmd->val;

    uint32_t ret = ehsm_chacha_onepass_dec((ehsm_ctx_st *)cmd_val->ctx_addr, cmd_val->key_handle,
        (uint8_t *)cmd_val->nonce, cmd_val->nonce_size, (const uint8_t *)cmd_val->aad, cmd_val->aad_size,
        cmd_val->constant, (const uint8_t *)cmd_val->input, cmd_val->input_size, (uint8_t *)cmd_val->output,
        (const uint8_t *)cmd_val->tag, cmd_val->tag_size, (bool_t *)&cmd_val->verify_result);

    chacha_onepass_dec_rsp_st *rsp_data = (chacha_onepass_dec_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->verify_result = cmd_val->verify_result;
    rsp->len = sizeof(chacha_onepass_dec_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_chacha_init(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(chacha_init_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const chacha_init_cmd_st *cmd_val = (const chacha_init_cmd_st *)cmd->val;

    uint32_t ret = ehsm_chacha_init((ehsm_ctx_st *)cmd_val->ctx_addr, (uint32_t)cmd_val->key_handle,
        (bool_t)cmd_val->enc, (uint8_t *)cmd_val->nonce, cmd_val->nonce_size, (const uint8_t *)cmd_val->aad,
        cmd_val->aad_size, cmd_val->constant, (ehsm_session_st *)cmd_val->session);

    chacha_init_rsp_st *rsp_data = (chacha_init_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(chacha_init_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_chacha_update(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(chacha_update_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const chacha_update_cmd_st *cmd_val = (const chacha_update_cmd_st *)cmd->val;

    uint32_t ret = ehsm_chacha_update((ehsm_ctx_st *)cmd_val->ctx_addr, (const uint8_t *)cmd_val->input,
        cmd_val->input_size, (uint8_t *)cmd_val->output);

    chacha_update_rsp_st *rsp_data = (chacha_update_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(chacha_update_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_chacha_finish_enc(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(chacha_finish_enc_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const chacha_finish_enc_cmd_st *cmd_val = (const chacha_finish_enc_cmd_st *)cmd->val;

    uint32_t ret = ehsm_chacha_finish_enc((ehsm_ctx_st *)cmd_val->ctx_addr, (const uint8_t *)cmd_val->input,
        cmd_val->input_size, (uint8_t *)cmd_val->output, (uint8_t *)cmd_val->tag, cmd_val->tag_size);

    chacha_finish_enc_rsp_st *rsp_data = (chacha_finish_enc_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(chacha_finish_enc_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_chacha_finish_dec(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(chacha_finish_dec_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const chacha_finish_dec_cmd_st *cmd_val = (const chacha_finish_dec_cmd_st *)cmd->val;

    uint32_t ret = ehsm_chacha_finish_dec((ehsm_ctx_st *)cmd_val->ctx_addr, (const uint8_t *)cmd_val->input,
        cmd_val->input_size, (uint8_t *)cmd_val->output, (const uint8_t *)cmd_val->tag, cmd_val->tag_size,
        (bool_t *)&cmd_val->verify_result);

    chacha_finish_dec_rsp_st *rsp_data = (chacha_finish_dec_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->verify_result = cmd_val->verify_result;
    rsp->len = sizeof(chacha_finish_dec_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_change_control_field(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(control_field_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const control_field_cmd_st *cmd_val = (const control_field_cmd_st *)cmd->val;
    uint32_t ret = ehsm_change_control_field(
        (ehsm_ctx_st *)cmd_val->ctx_addr, (ehsm_ctrl_field_e)cmd_val->field, (const uint64_t *)cmd_val->values);

    control_field_rsp_st *rsp_data = (control_field_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(control_field_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_change_lifecycle(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != 8) {
        return RSP_ERR_DATA_LENGTH;
    }
    uint32_t ctx_addr = *((uint32_t *)&cmd->val[0]);
    uint32_t lifecycle = *((uint32_t *)&cmd->val[4]);

    uint32_t ret;
    ret = ehsm_change_lifecycle((ehsm_ctx_st *)ctx_addr, (ehsm_lifecycle_e)lifecycle);
    rsp->len = 4;
    memcpy(rsp->val, &ret, 4);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_close_debug(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(close_debug_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }
    const close_debug_cmd_st *val = (const close_debug_cmd_st *)cmd->val;
    uint32_t ret;
    ret = ehsm_close_debug((ehsm_ctx_st *)val->ctx_addr, (ehsm_challenge_type_e)val->type,
        (const ehsm_soc_dbg_bitmap_st *)val->soc_dbg_bitmap);
    close_debug_rsp_st *rsp_val = (close_debug_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(close_debug_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_create_counter(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(counter_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const counter_cmd_st *val = (const counter_cmd_st *)cmd->val;
    uint32_t id = val->counter_id;
    uint64_t value = (uint64_t)val->counter_value;

    uint32_t ret;
    ret = ehsm_create_counter((ehsm_ctx_st *)val->ctx_addr, &id, &value);

    counter_rsp_st *rsp_val = (counter_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp_val->counter_id = id;
    rsp_val->counter_value = id;
    rsp->len = sizeof(counter_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_ctx_init(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(ctx_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }
    const ctx_cmd_st *val = (const ctx_cmd_st *)cmd->val;
    ehsm_ctx_init((ehsm_ctx_st *)val->ctx_addr, (uint8_t)val->mb_ch, (bool)val->async, (ehsm_rsp_cb_func_t)val->rsp_callback);
    ctx_rsp_st *rsp_val = (ctx_rsp_st *)rsp->val;
    rsp_val->ret = RSP_OK;
    rsp->len = sizeof(ctx_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_ctx_poll(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != 4) {
        return RSP_ERR_DATA_LENGTH;
    }
    const ctx_cmd_st *val = (const ctx_cmd_st *)cmd->val;

    uint32_t ret = ehsm_ctx_poll((ehsm_ctx_st *)val->ctx_addr);
    ctx_rsp_st *rsp_val = (ctx_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(ctx_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_debug_auth(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(debug_auth_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }
    const debug_auth_cmd_st *val = (const debug_auth_cmd_st *)cmd->val;

    uint32_t ret = ehsm_bl_debug_auth((ehsm_ctx_st *)val->ctx_addr, (ehsm_challenge_type_e)val->challenge_type,
        (ehsm_auth_algo_e)val->algo, (const uint8_t *)val->sig, val->sig_size, (uint8_t *)val->pub_key,
        val->pub_key_size, (const ehsm_soc_dbg_bitmap_st *)val->soc_dbg_bitmap);

    debug_auth_rsp_st *rsp_val = (debug_auth_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(debug_auth_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_delete_counter(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(delete_counter_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const delete_counter_cmd_st *cmd_val = (const delete_counter_cmd_st *)cmd->val;

    uint32_t ret = ehsm_delete_counter((ehsm_ctx_st *)cmd_val->ctx_addr, cmd_val->counter_id);

    delete_counter_rsp_st *rsp_data = (delete_counter_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(delete_counter_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_driver_get_version(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(driver_get_version_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    uint32_t ret = ehsm_driver_get_version();

    driver_get_version_rsp_st *rsp_data = (driver_get_version_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(driver_get_version_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_driver_init_librany(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(driver_init_library_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const driver_init_library_cmd_st *cmd_val = (const driver_init_library_cmd_st *)cmd->val;

    uint32_t ret = ehsm_driver_init_library((ehsm_drv_mode_e)cmd_val->drv_mode);

    driver_init_library_rsp_st *rsp_data = (driver_init_library_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(driver_init_library_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_ecdsa_finish_gen(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(ecdsa_finish_gen_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const ecdsa_finish_gen_cmd_st *cmd_val = (const ecdsa_finish_gen_cmd_st *)cmd->val;
    uint32_t sig_size = *(uint32_t *)cmd_val->sig_size;

    uint32_t ret = ehsm_ecdsa_finish_gen((ehsm_ctx_st *)cmd_val->ctx_addr, (uint8_t *)cmd_val->sig_addr, &sig_size);

    ecdsa_finish_gen_rsp_st *rsp_data = (ecdsa_finish_gen_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->sig_size = sig_size;
    rsp->len = sizeof(ecdsa_finish_gen_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_ecdsa_finish_verify(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(ecdsa_finish_verify_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }
    const ecdsa_finish_verify_cmd_st *cmd_val = (const ecdsa_finish_verify_cmd_st *)cmd->val;

    uint32_t ret = ehsm_ecdsa_finish_verify((ehsm_ctx_st *)cmd_val->ctx_addr, (const uint8_t *)cmd_val->sig_addr,
        cmd_val->sig_size, (bool_t *)&cmd_val->verify_result);

    ecdsa_finish_verify_rsp_st *rsp_data = (ecdsa_finish_verify_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->verify_result = (uint32_t)cmd_val->verify_result;
    rsp->len = sizeof(ecdsa_finish_verify_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_ecdsa_init(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(ecdsa_init_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const ecdsa_init_cmd_st *cmd_val = (const ecdsa_init_cmd_st *)cmd->val;

    uint32_t ret = ehsm_ecdsa_init((ehsm_ctx_st *)cmd_val->ctx_addr, (ehsm_hash_algo_e)cmd_val->algo,
        cmd_val->key_handle, (bool_t)cmd_val->gen_sig, (ehsm_session_st *)cmd_val->session_addr);

    ecdsa_init_rsp_st *rsp_data = (ecdsa_init_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(ecdsa_init_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_ecdsa_onepass_gen(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(ecdsa_onepass_gen_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const ecdsa_onepass_gen_cmd_st *cmd_val = (const ecdsa_onepass_gen_cmd_st *)cmd->val;

    uint32_t sig_size = cmd_val->sig_size;
    uint32_t ret = ehsm_ecdsa_onepass_gen((ehsm_ctx_st *)cmd_val->ctx_addr, (ehsm_hash_algo_e)cmd_val->algo,
        cmd_val->key_handle, (const uint8_t *)cmd_val->msg_addr, cmd_val->msg_size, (uint8_t *)cmd_val->sig_addr,
        &sig_size);

    ecdsa_onepass_gen_rsp_st *rsp_data = (ecdsa_onepass_gen_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->sig_size = sig_size;
    rsp->len = sizeof(ecdsa_onepass_gen_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_ecdsa_onepass_verify(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(ecdsa_onepass_verify_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }
    bool_t verify_result = false;
    const ecdsa_onepass_verify_cmd_st *cmd_val = (const ecdsa_onepass_verify_cmd_st *)cmd->val;
    uint32_t ret = ehsm_ecdsa_onepass_verify((ehsm_ctx_st *)cmd_val->ctx_addr, (ehsm_hash_algo_e)cmd_val->algo,
        cmd_val->key_handle, (const uint8_t *)cmd_val->msg_addr, cmd_val->msg_size, (uint8_t *)cmd_val->sig_addr,
        cmd_val->sig_size, &verify_result);

    ecdsa_onepass_verify_rsp_st *rsp_data = (ecdsa_onepass_verify_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->verify_result = (uint32_t)verify_result;
    rsp->len = sizeof(ecdsa_onepass_verify_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_ecdsa_onepass_gen_with_digest(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(ecdsa_onepass_gen_with_digest_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const ecdsa_onepass_gen_with_digest_cmd_st *cmd_val = (const ecdsa_onepass_gen_with_digest_cmd_st *)cmd->val;

    uint32_t sig_size = *(uint32_t *)cmd_val->sig_size;
    uint32_t ret = ehsm_ecdsa_onepass_gen_with_digest((ehsm_ctx_st *)cmd_val->ctx_addr,(ehsm_hash_algo_e)cmd_val->algo,
        cmd_val->key_handle, (const uint8_t *)cmd_val->digest_addr, cmd_val->digest_size,
        (uint8_t *)cmd_val->sig_addr, &sig_size);

    ecdsa_onepass_gen_rsp_st *rsp_data = (ecdsa_onepass_gen_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->sig_size = sig_size;
    rsp->len = sizeof(ecdsa_onepass_gen_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_ecdsa_onepass_verify_with_digest(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(ecdsa_onepass_verify_with_digest_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }
    bool_t verify_result = false;
    const ecdsa_onepass_verify_with_digest_cmd_st *cmd_val = (const ecdsa_onepass_verify_with_digest_cmd_st *)cmd->val;
    uint32_t ret = ehsm_ecdsa_onepass_verify_with_digest((ehsm_ctx_st *)cmd_val->ctx_addr, (ehsm_hash_algo_e)cmd_val->algo,
        cmd_val->key_handle, (const uint8_t *)cmd_val->digest_addr, cmd_val->digest_size,
        (uint8_t *)cmd_val->sig_addr, cmd_val->sig_size, (bool_t *)&verify_result);

    ecdsa_onepass_verify_rsp_st *rsp_data = (ecdsa_onepass_verify_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->verify_result = (uint32_t)verify_result;
    rsp->len = sizeof(ecdsa_onepass_verify_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_ecdsa_update(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(ecdsa_update_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const ecdsa_update_cmd_st *cmd_val = (const ecdsa_update_cmd_st *)cmd->val;

    uint32_t ret
        = ehsm_ecdsa_update((ehsm_ctx_st *)cmd_val->ctx_addr, (const uint8_t *)cmd_val->msg_addr, cmd_val->msg_size);

    ecdsa_update_rsp_st *rsp_data = (ecdsa_update_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(ecdsa_update_rsp_st);
    return RSP_OK;
}

typedef struct {
    uint32_t ctx;
    uint32_t algo;
    uint32_t use_plain_key;
    uint32_t key_handle;
    uint32_t key;
    uint32_t gen_sig;
    uint32_t is_digest;
    uint32_t input_addr;
    uint32_t input_size;
    uint32_t sig_addr;
    uint32_t sig_size;
    uint32_t verify_result_addr;
} ecdsa_onepass_ex_cmd_st;

uint16_t test_cmd_hostapi_ecdsa_onepass_ex(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(ecdsa_onepass_ex_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const ecdsa_onepass_ex_cmd_st *cmd_val = (const ecdsa_onepass_ex_cmd_st *)cmd->val;
    uint32_t sig_size = cmd_val->sig_size;
    bool_t verify_result = false;

    uint32_t ret = ehsm_ecdsa_onepass_ex(
        (ehsm_ctx_st *)cmd_val->ctx,
        (ehsm_hash_algo_e)cmd_val->algo,
        (bool_t)cmd_val->use_plain_key,
        cmd_val->key_handle,
        (const uint8_t *)cmd_val->key,
        (bool_t)cmd_val->gen_sig,
        (bool_t)cmd_val->is_digest,
        (const uint8_t *)cmd_val->input_addr,
        cmd_val->input_size,
        (uintptr_t)cmd_val->sig_addr,
        &sig_size,
        &verify_result
    );

    // Write verify_result to shared memory
    if (!cmd_val->gen_sig && cmd_val->verify_result_addr != 0) {
        *(uint32_t *)cmd_val->verify_result_addr = (uint32_t)verify_result;
    }

    ecdsa_rsp_st *rsp_data = (ecdsa_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->sig_size = sig_size;
    rsp_data->verify_result = verify_result;
    rsp->len = sizeof(ecdsa_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_enter_wfi(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(enter_wfi_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const enter_wfi_cmd_st *cmd_val = (const enter_wfi_cmd_st *)cmd->val;

    uint32_t ret = ehsm_enter_wfi((ehsm_ctx_st *)cmd_val->ctx_addr);

    enter_wfi_rsp_st *rsp_data = (enter_wfi_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(enter_wfi_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_gen_random(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(gen_random_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const gen_random_cmd_st *cmd_val = (const gen_random_cmd_st *)cmd->val;

    uint32_t ret = ehsm_gen_random((ehsm_ctx_st *)cmd_val->ctx_addr, (ehsm_rng_algo_e)cmd_val->algo,
        (uint8_t *)cmd_val->rand_buf_addr, cmd_val->rand_size);

    gen_random_rsp_st *rsp_data = (gen_random_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(gen_random_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_get_challenge(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(get_challenge_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const get_challenge_cmd_st *cmd_val = (const get_challenge_cmd_st *)cmd->val;

    uint32_t ret = ehsm_get_challenge((ehsm_ctx_st *)cmd_val->ctx_addr, (ehsm_challenge_type_e)cmd_val->challenge_type,
        (uint8_t *)cmd_val->output_addr);

    get_challenge_rsp_st *rsp_data = (get_challenge_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(get_challenge_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_get_emu_status(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(get_emu_status_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const get_emu_status_cmd_st *cmd_val = (const get_emu_status_cmd_st *)cmd->val;

    uint32_t ret
        = ehsm_get_emu_status((ehsm_ctx_st *)cmd_val->ctx_addr, (ehsm_emu_status_st *)cmd_val->status_buf_addr);

    get_emu_status_rsp_st *rsp_data = (get_emu_status_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(get_emu_status_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_get_utc_time(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(get_utc_time_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const get_utc_time_cmd_st *cmd_val = (const get_utc_time_cmd_st *)cmd->val;

    uint32_t ret = ehsm_get_utc_time((ehsm_ctx_st *)cmd_val->ctx_addr, (uint32_t *)cmd_val->utc_time_addr);

    get_utc_time_rsp_st *rsp_data = (get_utc_time_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(get_utc_time_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_get_version(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(get_version_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const get_version_cmd_st *cmd_val = (const get_version_cmd_st *)cmd->val;

    uint32_t ret = ehsm_get_version((ehsm_ctx_st *)cmd_val->ctx_addr, (ehsm_version_st *)cmd_val->version_addr);

    get_version_rsp_st *rsp_data = (get_version_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(get_version_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_hash_finish(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(hash_finish_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const hash_finish_cmd_st *cmd_val = (const hash_finish_cmd_st *)cmd->val;
    uint32_t digest_size = cmd_val->digest_size;

    uint32_t ret = ehsm_hash_finish((ehsm_ctx_st *)cmd_val->ctx_addr, (uint8_t *)cmd_val->digest, &digest_size);

    hash_finish_rsp_st *rsp_data = (hash_finish_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->digest_size = digest_size;
    rsp->len = sizeof(hash_finish_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_hash_init(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(hash_init_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const hash_init_cmd_st *cmd_val = (const hash_init_cmd_st *)cmd->val;

    uint32_t ret = ehsm_hash_init(
        (ehsm_ctx_st *)cmd_val->ctx, (ehsm_hash_algo_e)cmd_val->hash_algo, (ehsm_session_st *)cmd_val->session);

    hash_init_rsp_st *rsp_data = (hash_init_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(hash_init_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_hash_onepass(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(hash_onepass_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const hash_onepass_cmd_st *cmd_val = (const hash_onepass_cmd_st *)cmd->val;

    uint32_t ret = ehsm_hash_onepass((ehsm_ctx_st *)cmd_val->ctx_addr, (ehsm_hash_algo_e)cmd_val->algo,
        (const uint8_t *)cmd_val->msg_addr, cmd_val->msg_size, (uint8_t *)cmd_val->digest_addr, (uint32_t *)&cmd_val->digest_size);

    hash_onepass_rsp_st *rsp_data = (hash_onepass_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->digest_size = cmd_val->digest_size;
    rsp->len = sizeof(hash_onepass_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_hash_update(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(hash_update_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const hash_update_cmd_st *cmd_val = (const hash_update_cmd_st *)cmd->val;

    uint32_t ret = ehsm_hash_update((ehsm_ctx_st *)cmd_val->ctx, (const uint8_t *)cmd_val->msg, cmd_val->msg_size);

    hash_update_rsp_st *rsp_data = (hash_update_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(hash_update_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_hmac_finish_gen(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(hmac_finish_gen_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const hmac_finish_gen_cmd_st *cmd_val = (const hmac_finish_gen_cmd_st *)cmd->val;

    uint32_t ret
        = ehsm_hmac_finish_gen((ehsm_ctx_st *)cmd_val->ctx_addr, (uint8_t *)cmd_val->hmac_addr, cmd_val->hmac_size);

    hmac_finish_gen_rsp_st *rsp_data = (hmac_finish_gen_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(hmac_finish_gen_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_hmac_finish_verify(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(hmac_finish_verify_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const hmac_finish_verify_cmd_st *cmd_val = (const hmac_finish_verify_cmd_st *)cmd->val;

    bool_t verify_result = false;
    uint32_t ret = ehsm_hmac_finish_verify((ehsm_ctx_st *)cmd_val->ctx_addr, (const uint8_t *)cmd_val->hmac_addr,
        cmd_val->hmac_size, &verify_result);

    hmac_finish_verify_rsp_st *rsp_data = (hmac_finish_verify_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->verify_result = verify_result;
    rsp->len = sizeof(hmac_finish_verify_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_hmac_init(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(hmac_init_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const hmac_init_cmd_st *cmd_val = (const hmac_init_cmd_st *)cmd->val;

    uint32_t ret = ehsm_hmac_init((ehsm_ctx_st *)cmd_val->ctx_addr, (ehsm_hash_algo_e)cmd_val->algo,
        cmd_val->key_handle, (bool_t)cmd_val->gen_hmac, (ehsm_session_st *)cmd_val->session_addr);

    hmac_init_rsp_st *rsp_data = (hmac_init_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(hmac_init_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_hmac_onepass_gen(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(hmac_onepass_gen_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const hmac_onepass_gen_cmd_st *cmd_val = (const hmac_onepass_gen_cmd_st *)cmd->val;

    uint32_t ret
        = ehsm_hmac_onepass_gen((ehsm_ctx_st *)cmd_val->ctx_addr, (ehsm_hash_algo_e)cmd_val->algo, cmd_val->key_handle,
            (const uint8_t *)cmd_val->msg_addr, cmd_val->msg_size, (uint8_t *)cmd_val->hmac_addr, cmd_val->hmac_size);

    hmac_onepass_gen_rsp_st *rsp_data = (hmac_onepass_gen_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(hmac_onepass_gen_rsp_st);
    return RSP_OK;
}


uint16_t test_cmd_hostapi_hmac_onepass_verify(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(hmac_onepass_verify_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const hmac_onepass_verify_cmd_st *cmd_val = (const hmac_onepass_verify_cmd_st *)cmd->val;

    bool_t verify_result = false;
    uint32_t ret = ehsm_hmac_onepass_verify((ehsm_ctx_st *)cmd_val->ctx_addr, (ehsm_hash_algo_e)cmd_val->algo,
        cmd_val->key_handle, (const uint8_t *)cmd_val->msg_addr, cmd_val->msg_size, (const uint8_t *)cmd_val->hmac_addr,
        cmd_val->hmac_size, &verify_result);

    hmac_onepass_verify_rsp_st *rsp_data = (hmac_onepass_verify_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->verify_result = verify_result;
    rsp->len = sizeof(hmac_onepass_verify_rsp_st);
    return RSP_OK;
}


uint16_t test_cmd_hostapi_hmac_update(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(hmac_update_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const hmac_update_cmd_st *cmd_val = (const hmac_update_cmd_st *)cmd->val;

    uint32_t ret
        = ehsm_hmac_update((ehsm_ctx_st *)cmd_val->ctx_addr, (const uint8_t *)cmd_val->msg_addr, cmd_val->msg_size);

    hmac_update_rsp_st *rsp_data = (hmac_update_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(hmac_update_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_hmac_onepass_gen_with_key(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(hmac_onepass_gen_with_plain_key_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const hmac_onepass_gen_with_plain_key_cmd_st *cmd_val = (const hmac_onepass_gen_with_plain_key_cmd_st *)cmd->val;

    uint32_t ret = ehsm_hmac_onepass_gen_with_plain_key(
        (ehsm_ctx_st *)cmd_val->ctx_addr,
        (ehsm_hash_algo_e)cmd_val->algo,
        (const uint8_t *)cmd_val->key_addr,
        cmd_val->key_size,
        (const uint8_t *)cmd_val->msg_addr,
        cmd_val->msg_size,
        (uint8_t *)cmd_val->hmac_addr,
        cmd_val->hmac_size
    );

    hmac_onepass_gen_with_plain_key_rsp_st *rsp_data = (hmac_onepass_gen_with_plain_key_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(hmac_onepass_gen_with_plain_key_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_hmac_onepass_verify_with_key(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(hmac_onepass_verify_with_plain_key_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const hmac_onepass_verify_with_plain_key_cmd_st *cmd_val = (const hmac_onepass_verify_with_plain_key_cmd_st *)cmd->val;

    uint32_t ret = ehsm_hmac_onepass_verify_with_plain_key(
        (ehsm_ctx_st *)cmd_val->ctx_addr,
        (ehsm_hash_algo_e)cmd_val->algo,
        (const uint8_t *)cmd_val->key_addr,
        cmd_val->key_size,
        (const uint8_t *)cmd_val->msg_addr,
        cmd_val->msg_size,
        (const uint8_t *)cmd_val->hmac_addr,
        cmd_val->hmac_size,
        (bool_t *)cmd_val->verify_result_addr
    );

    hmac_onepass_verify_with_plain_key_rsp_st *rsp_data = (hmac_onepass_verify_with_plain_key_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(hmac_onepass_verify_with_plain_key_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_hmac_init_with_key(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(hmac_init_with_plain_key_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const hmac_init_with_plain_key_cmd_st *cmd_val = (const hmac_init_with_plain_key_cmd_st *)cmd->val;

    uint32_t ret = ehsm_hmac_init_with_plain_key(
        (ehsm_ctx_st *)cmd_val->ctx_addr,
        (ehsm_hash_algo_e)cmd_val->algo,
        (const uint8_t *)cmd_val->key_addr,
        cmd_val->key_size,
        (bool_t)cmd_val->gen_hmac,
        (ehsm_session_st *)cmd_val->session
    );

    hmac_init_with_plain_key_rsp_st *rsp_data = (hmac_init_with_plain_key_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(hmac_init_with_plain_key_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_increase_counter(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(increase_counter_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const increase_counter_cmd_st *cmd_val = (const increase_counter_cmd_st *)cmd->val;

    uint32_t ret = ehsm_increase_counter((ehsm_ctx_st *)cmd_val->ctx_addr, cmd_val->counter_id, cmd_val->increase_value,
        (uint64_t *)cmd_val->current_value_addr);

    increase_counter_rsp_st *rsp_data = (increase_counter_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(increase_counter_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_inject_error(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(inject_error_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const inject_error_cmd_st *cmd_val = (const inject_error_cmd_st *)cmd->val;

    uint32_t ret
        = ehsm_inject_error((ehsm_ctx_st *)cmd_val->ctx_addr, (const ehsm_inject_error_st *)cmd_val->values_addr);

    inject_error_rsp_st *rsp_data = (inject_error_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(inject_error_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_install_encrypted_key(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(install_encrypted_key_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const install_encrypted_key_cmd_st *cmd_val = (const install_encrypted_key_cmd_st *)cmd->val;

    uint32_t ret = ehsm_install_encrypted_key((ehsm_ctx_st *)cmd_val->ctx_addr, (ehsm_key_level_e)cmd_val->key_level,
        (ehsm_install_key_type_e)cmd_val->key_type, (uint16_t)cmd_val->key_slot_id, cmd_val->last_key,
        (const uint8_t *)cmd_val->input_data_addr, cmd_val->size);

    install_encrypted_key_rsp_st *rsp_data = (install_encrypted_key_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(install_encrypted_key_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_install_random_key(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(install_random_key_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const install_random_key_cmd_st *cmd_val = (const install_random_key_cmd_st *)cmd->val;

    uint32_t ret = ehsm_install_random_key((ehsm_ctx_st *)cmd_val->ctx_addr, (ehsm_key_level_e)cmd_val->key_level,
        (ehsm_install_key_type_e)cmd_val->key_type, (uint16_t)cmd_val->key_slot_id, (uint32_t)cmd_val->last_key);

    install_random_key_rsp_st *rsp_data = (install_random_key_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(install_random_key_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_km_derive_key(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(km_derive_key_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const km_derive_key_cmd_st *cmd_val = (const km_derive_key_cmd_st *)cmd->val;
    uint32_t handle = cmd_val->key_handle_addr;
    uint32_t ret = ehsm_km_derive_key((ehsm_ctx_st *)cmd_val->ctx_addr, (ehsm_hash_algo_e)cmd_val->hash_algo,
        (ehsm_derive_algo_e)cmd_val->derive_algo, (ehsm_derive_type_e)cmd_val->derive_type, cmd_val->privilege,
        (ehsm_key_type_e)cmd_val->key_type, (uint16_t)cmd_val->key_size, cmd_val->parent_key_handle,
        (const uint8_t *)cmd_val->salt_addr, cmd_val->salt_size, (const uint8_t *)cmd_val->password_addr,
        cmd_val->password_size, cmd_val->iter_times, &handle);

    km_derive_key_rsp_st *rsp_data = (km_derive_key_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->handle = handle;
    rsp->len = sizeof(km_derive_key_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_km_derive_key_to_soc(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(km_derive_key_to_soc_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const km_derive_key_to_soc_cmd_st *cmd_val = (const km_derive_key_to_soc_cmd_st *)cmd->val;

    uint32_t ret = ehsm_km_derive_key_to_soc((ehsm_ctx_st *)cmd_val->ctx_addr, (ehsm_hash_algo_e)cmd_val->hash_algo,
        (ehsm_derive_algo_e)cmd_val->derive_algo, (ehsm_derive_type_e)cmd_val->derive_type, cmd_val->parent_key_handle,
        (const uint8_t *)cmd_val->salt_addr, cmd_val->salt_size, cmd_val->iter_times, (uint8_t)cmd_val->soc_channel_id);

    km_derive_key_to_soc_rsp_st *rsp_data = (km_derive_key_to_soc_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(km_derive_key_to_soc_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_km_exchange_key(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(km_exchange_key_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const km_exchange_key_cmd_st *cmd_val = (const km_exchange_key_cmd_st *)cmd->val;
    uint32_t handle = cmd_val->key_handle_addr;
    uint32_t ret = ehsm_km_exchange_key((ehsm_ctx_st *)cmd_val->ctx_addr, (const uint8_t *)cmd_val->rmt_pub_key_addr,
        cmd_val->rmt_pub_key_size, cmd_val->privilege, (ehsm_key_type_e)cmd_val->key_type, cmd_val->hmac_key_size,
        cmd_val->local_key_handle, (const void *)cmd_val->dh_params_addr, cmd_val->dh_params_size,
        (ehsm_sm2_params_st *)cmd_val->sm2_params_addr, &handle);

    km_exchange_key_rsp_st *rsp_data = (km_exchange_key_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->handle = handle;
    rsp->len = sizeof(km_exchange_key_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_km_export_key(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(km_export_key_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const km_export_key_cmd_st *cmd_val = (const km_export_key_cmd_st *)cmd->val;

    uint32_t ret = ehsm_km_export_key((ehsm_ctx_st *)cmd_val->ctx_addr, cmd_val->target_key_handle,
        cmd_val->transport_key_handle, cmd_val->auth_key_handle, (ehsm_key_part_e)cmd_val->key_part,
        (ehsm_key_format_st *)cmd_val->key_data_addr, (uint32_t *)&cmd_val->key_data_size,
        (uint8_t *)cmd_val->mac_addr, (uint32_t *)&cmd_val->mac_size);

    km_export_key_rsp_st *rsp_data = (km_export_key_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->key_data_size = cmd_val->key_data_size;
    rsp_data->mac_size = cmd_val->mac_size;
    rsp->len = sizeof(km_export_key_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_km_gen_key(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(km_gen_key_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const km_gen_key_cmd_st *cmd_val = (const km_gen_key_cmd_st *)cmd->val;
    uint32_t handle = cmd_val->key_handle_addr;
    uint32_t ret = ehsm_km_gen_key((ehsm_ctx_st *)cmd_val->ctx_addr, (ehsm_key_type_e)cmd_val->key_type,
        cmd_val->privilege, cmd_val->rsa_e_bit_size, cmd_val->hmac_key_size, (const void *)cmd_val->dh_params_addr,
        cmd_val->dh_params_size, &handle);

    km_gen_key_rsp_st *rsp_data = (km_gen_key_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->handle = handle;
    rsp->len = sizeof(km_gen_key_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_km_get_pub_from_priv(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(km_get_pub_from_priv_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const km_get_pub_from_priv_cmd_st *cmd_val = (const km_get_pub_from_priv_cmd_st *)cmd->val;

    uint32_t ret = ehsm_km_get_pub_from_priv((ehsm_ctx_st *)cmd_val->ctx_addr, cmd_val->key_handle,
        (const void *)cmd_val->dh_params, cmd_val->dh_params_size, (uint8_t *)cmd_val->pub_key,
        (uint32_t *)cmd_val->pub_key_size, (ehsm_key_type_e *)cmd_val->key_type);

    km_get_pub_from_priv_rsp_st *rsp_data = (km_get_pub_from_priv_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->algo_id = *((uint32_t *)cmd_val->key_type);
    rsp_data->public_key_size = *((uint32_t *)cmd_val->pub_key_size);
    rsp->len = sizeof(km_get_pub_from_priv_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_km_import_key(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(km_import_key_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const km_import_key_cmd_st *cmd_val = (const km_import_key_cmd_st *)cmd->val;

    uint32_t ret = ehsm_km_import_key((ehsm_ctx_st *)cmd_val->ctx_addr, cmd_val->transport_key_handle,
        cmd_val->auth_key_handle, (const ehsm_key_format_st *)cmd_val->key_data_addr, cmd_val->key_data_size,
        (const uint8_t *)cmd_val->mac_addr, cmd_val->mac_size, (uint32_t *)&cmd_val->key_handle);

    km_import_key_rsp_st *rsp_data = (km_import_key_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->handle = cmd_val->key_handle;
    rsp->len = sizeof(km_import_key_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_km_remove_key(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(km_remove_key_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const km_remove_key_cmd_st *cmd_val = (const km_remove_key_cmd_st *)cmd->val;

    uint32_t ret = ehsm_km_remove_key((ehsm_ctx_st *)cmd_val->ctx_addr, cmd_val->key_handle);

    km_remove_key_rsp_st *rsp_data = (km_remove_key_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(km_remove_key_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_km_sm9_exchange_key(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(km_sm9_exchange_key_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const km_sm9_exchange_key_cmd_st *cmd_val = (const km_sm9_exchange_key_cmd_st *)cmd->val;
    uint32_t key_handle;
    uint32_t ret = ehsm_km_sm9_exchange_key((ehsm_ctx_st *)cmd_val->ctx_addr, cmd_val->privilege,
        (ehsm_key_type_e)cmd_val->key_type, (uint8_t)cmd_val->role, cmd_val->user_priv_key_handle, cmd_val->user_tmp_key_handle,
        cmd_val->hmac_key_size, (ehsm_sm9_params_st *)cmd_val->extra_params_addr, &key_handle);

    km_sm9_exchange_key_rsp_st *rsp_data = (km_sm9_exchange_key_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->key_handle = key_handle;
    rsp->len = sizeof(km_sm9_exchange_key_rsp_st);
    return RSP_OK;
}

typedef struct {
    uint32_t ctx;
    uint32_t src_data;
    uint32_t ehsm_dest_addr;
    uint32_t size;
} test_memory_cmd_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t buf;
    uint32_t ehsm_src_addr;
    uint32_t size;
} test_memory_read_cmd_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t addr;
} test_jump_to_addr_cmd_st;

typedef struct {
    uint32_t ctx_addr;
} test_jump_to_loop_cmd_st;

typedef struct {
    uint32_t ret;
} test_cmd_rsp_st;

uint16_t test_cmd_hostapi_test_read_memory(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(test_memory_read_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }
    test_memory_read_cmd_st *req = (test_memory_read_cmd_st *)cmd->val;
    uint8_t *buf = (uint8_t *)req->buf;
    uint32_t ret = ehsm_test_read_memory((ehsm_ctx_st *)req->ctx_addr, buf, req->ehsm_src_addr, req->size);
    test_cmd_rsp_st *rsp_data = (test_cmd_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp->len = sizeof(test_cmd_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_test_write_memory(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(test_memory_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const test_memory_cmd_st *cmd_val = (const test_memory_cmd_st *)cmd->val;
    uint32_t ret = ehsm_test_write_memory((ehsm_ctx_st *)cmd_val->ctx, (const uint8_t *)cmd_val->src_data,
        (uint32_t)cmd_val->ehsm_dest_addr, (uint32_t)cmd_val->size);

    test_cmd_rsp_st *rsp_val = (test_cmd_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(test_cmd_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_test_jump_to_addr(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(test_jump_to_addr_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const test_jump_to_addr_cmd_st *cmd_val = (const test_jump_to_addr_cmd_st *)cmd->val;
    uint32_t ret = ehsm_test_jump_to_addr((ehsm_ctx_st *)cmd_val->ctx_addr, (uint32_t)cmd_val->addr);

    test_cmd_rsp_st *rsp_val = (test_cmd_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(test_cmd_rsp_st);
    return RSP_OK;
}
uint16_t test_cmd_hostapi_test_jump_to_loop(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(test_jump_to_loop_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const test_jump_to_loop_cmd_st *cmd_val = (const test_jump_to_loop_cmd_st *)cmd->val;
    uint32_t ret = ehsm_test_jump_to_loop((ehsm_ctx_st *)cmd_val->ctx_addr);

    test_cmd_rsp_st *rsp_val = (test_cmd_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(test_cmd_rsp_st);
    return RSP_OK;
}

// PKE plain key command structures
typedef struct {
    uint32_t ctx_addr;
    uint32_t key_addr;
    uint32_t key_size;
    uint32_t enc;
    uint32_t input_addr;
    uint32_t input_size;
    uint32_t output_addr;
    uint32_t output_buff_size;
} rsa_cipher_with_plain_key_cmd_st;

typedef struct {
    uint32_t ret;
    uint32_t output_size;
} rsa_cipher_with_plain_key_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t key_addr;
    uint32_t key_size;
    uint32_t enc;
    uint32_t input_addr;
    uint32_t input_size;
    uint32_t output_addr;
    uint32_t output_buff_size;
} sm2_cipher_with_plain_key_cmd_st;

typedef struct {
    uint32_t ret;
    uint32_t output_size;
} sm2_cipher_with_plain_key_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t algo;
    uint32_t key_addr;
    uint32_t key_size;
    uint32_t gen_sig;
    uint32_t padding;
    uint32_t session;
} rsa_sign_init_with_plain_key_cmd_st;

typedef struct {
    uint32_t ret;
} rsa_sign_init_with_plain_key_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t key_addr;
    uint32_t key_size;
    uint32_t gen_sig;
    uint32_t session;
} sm2_sign_init_with_plain_key_cmd_st;

typedef struct {
    uint32_t ret;
} sm2_sign_init_with_plain_key_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t algo;
    uint32_t key_addr;
    uint32_t gen_sig;
    uint32_t session;
} ecdsa_init_with_plain_key_cmd_st;

typedef struct {
    uint32_t ret;
} ecdsa_init_with_plain_key_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t key_addr;
    uint32_t key_size;
    uint32_t enc;
    uint32_t enc_type;
    uint32_t padding;
    uint32_t key2_size;
    uint32_t hid;
    uint32_t kgc_pub_key_addr;
    uint32_t input_addr;
    uint32_t input_size;
    uint32_t output_addr;
    uint32_t output_buff_size;
    uint32_t id_addr;
    uint32_t id_size;
    uint32_t fp12g_addr;
} sm9_cipher_with_plain_key_cmd_st;

typedef struct {
    uint32_t ret;
    uint32_t output_size;
} sm9_cipher_with_plain_key_rsp_st;

typedef struct {
    uint32_t ctx_addr;
    uint32_t key_addr;
    uint32_t msg_addr;
    uint32_t msg_size;
    uint32_t sig_addr;
    uint32_t sig_size;
    uint32_t kgc_pub_key_addr;
    uint32_t fp12g_addr;
} sm9_sign_onepass_gen_with_plain_key_cmd_st;

typedef struct {
    uint32_t ret;
} sm9_sign_onepass_gen_with_plain_key_rsp_st;

// PKE plain key command handlers
uint16_t test_cmd_hostapi_rsa_cipher_with_plain_key(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(rsa_cipher_with_plain_key_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }
    rsa_cipher_with_plain_key_cmd_st *req = (rsa_cipher_with_plain_key_cmd_st *)cmd->val;
    uint32_t output_size = req->output_buff_size;
    uint32_t ret = ehsm_rsa_cipher_with_plain_key((ehsm_ctx_st *)req->ctx_addr,
        (const uint8_t *)req->key_addr, (bool_t)req->enc,
        (const uint8_t *)req->input_addr, req->input_size,
        (uint8_t *)req->output_addr, &output_size);
    rsa_cipher_with_plain_key_rsp_st *rsp_data = (rsa_cipher_with_plain_key_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->output_size = output_size;
    rsp->len = sizeof(rsa_cipher_with_plain_key_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_sm2_cipher_with_plain_key(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(sm2_cipher_with_plain_key_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }
    sm2_cipher_with_plain_key_cmd_st *req = (sm2_cipher_with_plain_key_cmd_st *)cmd->val;
    uint32_t output_size = req->output_buff_size;
    uint32_t ret = ehsm_sm2_cipher_with_plain_key((ehsm_ctx_st *)req->ctx_addr,
        (const uint8_t *)req->key_addr, (bool_t)req->enc,
        (const uint8_t *)req->input_addr, req->input_size,
        (uint8_t *)req->output_addr, &output_size);
    sm2_cipher_with_plain_key_rsp_st *rsp_data = (sm2_cipher_with_plain_key_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->output_size = output_size;
    rsp->len = sizeof(sm2_cipher_with_plain_key_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_rsa_sign_init_with_plain_key(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(rsa_sign_init_with_plain_key_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }
    const rsa_sign_init_with_plain_key_cmd_st *cmd_val = (const rsa_sign_init_with_plain_key_cmd_st *)cmd->val;
    uint32_t ret = ehsm_rsa_sign_init_with_plain_key((ehsm_ctx_st *)cmd_val->ctx_addr,
        (ehsm_hash_algo_e)cmd_val->algo, (const uint8_t *)cmd_val->key_addr,
        (bool_t)cmd_val->gen_sig, (ehsm_rsa_padding_mode_e)cmd_val->padding,
        (ehsm_session_st *)cmd_val->session);
    rsa_sign_init_with_plain_key_rsp_st *rsp_val = (rsa_sign_init_with_plain_key_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(rsa_sign_init_with_plain_key_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_sm2_sign_init_with_plain_key(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(sm2_sign_init_with_plain_key_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }
    const sm2_sign_init_with_plain_key_cmd_st *cmd_val = (const sm2_sign_init_with_plain_key_cmd_st *)cmd->val;
    uint32_t ret = ehsm_sm2_sign_init_with_plain_key((ehsm_ctx_st *)cmd_val->ctx_addr,
        (const uint8_t *)cmd_val->key_addr, (bool_t)cmd_val->gen_sig,
        (ehsm_session_st *)cmd_val->session);
    sm2_sign_init_with_plain_key_rsp_st *rsp_val = (sm2_sign_init_with_plain_key_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(sm2_sign_init_with_plain_key_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_ecdsa_init_with_plain_key(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(ecdsa_init_with_plain_key_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }
    const ecdsa_init_with_plain_key_cmd_st *cmd_val = (const ecdsa_init_with_plain_key_cmd_st *)cmd->val;
    uint32_t ret = ehsm_ecdsa_init_with_plain_key((ehsm_ctx_st *)cmd_val->ctx_addr,
        (ehsm_hash_algo_e)cmd_val->algo, (const uint8_t *)cmd_val->key_addr,
        (bool_t)cmd_val->gen_sig, (ehsm_session_st *)cmd_val->session);
    ecdsa_init_with_plain_key_rsp_st *rsp_val = (ecdsa_init_with_plain_key_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(ecdsa_init_with_plain_key_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_sm9_cipher_with_plain_key(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(sm9_cipher_with_plain_key_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }
    sm9_cipher_with_plain_key_cmd_st *req = (sm9_cipher_with_plain_key_cmd_st *)cmd->val;
    uint32_t output_size = req->output_buff_size;
    uint32_t ret = ehsm_sm9_cipher_with_plain_key((ehsm_ctx_st *)req->ctx_addr,
        (const uint8_t *)req->key_addr, (bool_t)req->enc,
        (ehsm_sm9_enc_type_e)req->enc_type, (ehsm_sm9_padding_mode_e)req->padding,
        (uint8_t)req->key2_size, (uint8_t)req->hid, (const uint8_t *)req->kgc_pub_key_addr,
        (const uint8_t *)req->input_addr, req->input_size,
        (uint8_t *)req->output_addr, &output_size,
        (const uint8_t *)req->id_addr, req->id_size, (const uint8_t *)req->fp12g_addr);
    sm9_cipher_with_plain_key_rsp_st *rsp_data = (sm9_cipher_with_plain_key_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->output_size = output_size;
    rsp->len = sizeof(sm9_cipher_with_plain_key_rsp_st);
    return RSP_OK;
}

uint16_t test_cmd_hostapi_sm9_sign_onepass_gen_with_plain_key(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(sm9_sign_onepass_gen_with_plain_key_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }
    const sm9_sign_onepass_gen_with_plain_key_cmd_st *cmd_val = (const sm9_sign_onepass_gen_with_plain_key_cmd_st *)cmd->val;
    uint32_t ret = ehsm_sm9_sign_onepass_gen_with_plain_key((ehsm_ctx_st *)cmd_val->ctx_addr,
        (const uint8_t *)cmd_val->key_addr,
        (const uint8_t *)cmd_val->msg_addr, cmd_val->msg_size,
        (uint8_t *)cmd_val->sig_addr, cmd_val->sig_size,
        (const uint8_t *)cmd_val->kgc_pub_key_addr, (const uint8_t *)cmd_val->fp12g_addr);
    sm9_sign_onepass_gen_with_plain_key_rsp_st *rsp_val = (sm9_sign_onepass_gen_with_plain_key_rsp_st *)rsp->val;
    rsp_val->ret = ret;
    rsp->len = sizeof(sm9_sign_onepass_gen_with_plain_key_rsp_st);
    return RSP_OK;
}
// PQC DSA command structures
typedef struct {
    uint32_t ctx;
    uint32_t sign_algo;
    uint32_t sign_mode;
    uint32_t hash_algo;
    uint32_t is_det;
    uint32_t use_plain_key;
    uint32_t key_handle;
    uint32_t key_addr;
    uint32_t gen_sig;
    uint32_t input_addr;
    uint32_t input_size;
    uint32_t pqc_ctx_str_addr;
    uint32_t pqc_ctx_str_size;
    uint32_t sig_addr;
    uint32_t sig_size;
    uint32_t verify_result_addr;
} pqc_dsa_onepass_ex_cmd_st;

typedef struct {
    uint32_t ret;
    uint32_t sig_size;
    uint32_t verify_result;
} pqc_dsa_onepass_ex_rsp_st;

uint16_t test_cmd_hostapi_pqc_dsa_onepass_ex(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    // Debug: print actual vs expected size
    printf("[PQC DSA] cmd->len=%u, sizeof(pqc_dsa_onepass_ex_cmd_st)=%u\n", cmd->len, (uint32_t)sizeof(pqc_dsa_onepass_ex_cmd_st));
    if (cmd->len != sizeof(pqc_dsa_onepass_ex_cmd_st)) {
        printf("[PQC DSA] ERROR: Length mismatch! Expected %u, got %u\n", (uint32_t)sizeof(pqc_dsa_onepass_ex_cmd_st), cmd->len);
        return RSP_ERR_DATA_LENGTH;
    }

    const pqc_dsa_onepass_ex_cmd_st *cmd_val = (const pqc_dsa_onepass_ex_cmd_st *)cmd->val;
    uint32_t sig_size = cmd_val->sig_size;
    bool_t verify_result = false;

    uint32_t ret = ehsm_pqc_dsa_onepass_ex(
        (ehsm_ctx_st *)cmd_val->ctx,
        (ehsm_pqc_sign_algo_e)cmd_val->sign_algo,
        (ehsm_pqc_sign_mode_e)cmd_val->sign_mode,
        (ehsm_pqc_hash_algo_e)cmd_val->hash_algo,
        (bool_t)cmd_val->is_det,
        (bool_t)cmd_val->use_plain_key,
        cmd_val->key_handle,
        (const ehsm_pqc_key_st *)cmd_val->key_addr,
        (bool_t)cmd_val->gen_sig,
        (const uint8_t *)cmd_val->input_addr,
        cmd_val->input_size,
        (uint8_t *)cmd_val->pqc_ctx_str_addr,
        cmd_val->pqc_ctx_str_size,
        (uintptr_t)cmd_val->sig_addr,
        &sig_size,
        &verify_result
    );

    // Write verify_result to shared memory
    if (!cmd_val->gen_sig && cmd_val->verify_result_addr != 0) {
        *(uint32_t *)cmd_val->verify_result_addr = (uint32_t)verify_result;
    }

    pqc_dsa_onepass_ex_rsp_st *rsp_data = (pqc_dsa_onepass_ex_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->sig_size = sig_size;
    rsp_data->verify_result = verify_result;
    rsp->len = sizeof(pqc_dsa_onepass_ex_rsp_st);
    return RSP_OK;
}

// PQC ML-KEM command structures
typedef struct {
    uint32_t ctx;
    uint32_t is_encaps;
    uint32_t use_plain_parent_key;
    uint32_t parent_key_handle;
    uint32_t parent_key_addr;
    uint32_t cipher_key_data_addr;
    uint32_t cipher_key_data_size;
    uint32_t ss_out_type;
    uint32_t ss_key_type;
    uint32_t ss_privilege;
    uint32_t ss_key_handle_addr;
    uint32_t ss_key_addr;
    uint32_t ss_key_size;
} pqc_ml_kem_ex_cmd_st;

typedef struct {
    uint32_t ret;
    uint32_t cipher_key_data_size;
    uint32_t ss_key_handle;
} pqc_ml_kem_ex_rsp_st;

uint16_t test_cmd_hostapi_pqc_ml_kem_ex(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(pqc_ml_kem_ex_cmd_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const pqc_ml_kem_ex_cmd_st *cmd_val = (const pqc_ml_kem_ex_cmd_st *)cmd->val;
    uint32_t cipher_key_data_size = cmd_val->cipher_key_data_size;

    // Reason: 从共享内存读取输入的 ss_key_handle 值（0xFFFFFFFF表示自动分配新句柄）
    // 这样固件 API 才能知道用户想要自动分配还是使用指定句柄
    uint32_t ss_key_handle = (cmd_val->ss_key_handle_addr != 0) ?
                              *(uint32_t *)cmd_val->ss_key_handle_addr : 0;

    uint32_t ret = ehsm_pqc_ml_kem_ex(
        (ehsm_ctx_st *)cmd_val->ctx,
        (bool_t)cmd_val->is_encaps,
        (bool_t)cmd_val->use_plain_parent_key,
        cmd_val->parent_key_handle,
        (const ehsm_pqc_key_st *)cmd_val->parent_key_addr,
        (uint8_t *)cmd_val->cipher_key_data_addr,
        &cipher_key_data_size,
        (ehsm_pqc_out_type_e)cmd_val->ss_out_type,
        (ehsm_key_type_e)cmd_val->ss_key_type,
        cmd_val->ss_privilege,
        &ss_key_handle,
        (uint8_t *)cmd_val->ss_key_addr,
        cmd_val->ss_key_size
    );

    // Write ss_key_handle to shared memory if needed
    if (cmd_val->ss_key_handle_addr != 0) {
        *(uint32_t *)cmd_val->ss_key_handle_addr = ss_key_handle;
    }

    pqc_ml_kem_ex_rsp_st *rsp_data = (pqc_ml_kem_ex_rsp_st *)rsp->val;
    rsp_data->ret = ret;
    rsp_data->cipher_key_data_size = cipher_key_data_size;
    rsp_data->ss_key_handle = ss_key_handle;
    rsp->len = sizeof(pqc_ml_kem_ex_rsp_st);
    return RSP_OK;
}
