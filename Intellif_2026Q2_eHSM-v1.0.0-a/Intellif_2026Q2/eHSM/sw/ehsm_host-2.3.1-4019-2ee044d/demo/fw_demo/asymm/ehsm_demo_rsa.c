#include <stdio.h>
#include <string.h>

#include "common/mem.h"

#include "ehsmdrv/basic/api.h"
#include "common/utils.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"
#include "ehsm_demo_rsa.h"

#define DEMO_RSA_KEY_HANDLE (0x100000U)

typedef struct {
    const char *alg_str;
    ehsm_key_type_e key_type;
    const uint8_t *std_key;
    uint16_t std_key_size;
    uint16_t std_pub_key_size;
    uint16_t std_priv_key_size;
    uint16_t std_n_size;
    const uint8_t *std_plaintext;
    uint32_t std_plaintext_size;
    const uint8_t *std_ciphertext;
    uint32_t std_ciphertext_size;
} demo_rsa_cipher_std_st;

typedef struct {
    const char *alg_str;
    ehsm_rsa_padding_mode_e padding;
    ehsm_hash_algo_e algo;
    ehsm_key_type_e key_type;
    const uint8_t *std_key;
    uint16_t std_key_size;
    uint16_t std_pub_key_size;
    uint16_t std_priv_key_size;
    uint16_t std_n_size;
    const uint8_t *std_msg;
    uint32_t std_msg_size;
    const uint8_t *std_signature;
    uint32_t std_signature_size;
    const uint8_t *std_salt;
    uint32_t std_salt_size;
} demo_rsa_sign_std_st;

// clang-format off
static const uint8_t s_rsa_1024_std_key[4 + 128 + 128] = {
    /* e: Public exponent */
    0x00,0x01,0x00,0x01,
    /* n: Modulus */
    0xED,0x98,0x3D,0xFF,0x82,0x00,0x88,0x75,0xF5,0x53,0xBC,0x53,0x04,0x55,0x69,0xF2,
    0x4F,0x65,0x2A,0x36,0x4E,0x33,0x5B,0xD9,0xDB,0xAE,0x64,0x63,0x5F,0x4F,0xF4,0x0D,
    0x24,0xC2,0x93,0x98,0x57,0xBF,0xAB,0x56,0x67,0xCC,0x00,0xB5,0xC7,0x77,0xCA,0x46,
    0xD1,0xD4,0x97,0x46,0x3E,0x15,0xBB,0x85,0x7C,0xE8,0x75,0x04,0xB5,0xB8,0x05,0xFC,
    0x52,0x1D,0xCA,0x77,0x77,0xE1,0x34,0x5B,0xD4,0x18,0x6B,0x34,0x04,0xCE,0x25,0xC1,
    0xBB,0x41,0x51,0x8B,0x53,0x6C,0x74,0x48,0x4C,0xEA,0x1B,0xF8,0x5B,0xB6,0x03,0x33,
    0x11,0x60,0xFA,0xC3,0x40,0xE3,0x80,0x5A,0x8D,0x24,0x4F,0x91,0x08,0xFA,0x1B,0x59,
    0xBE,0x13,0x92,0x96,0xB2,0x57,0xF3,0xF0,0xBA,0xDA,0x2F,0xDE,0xCC,0x2E,0x09,0xE3,
    /* d: Private exponent */
    0x9D,0x2A,0x90,0x18,0x6C,0x8E,0x9D,0xB6,0x29,0xCA,0x72,0x51,0x18,0x14,0xC8,0x38,
    0x99,0x7C,0x7A,0x27,0xE2,0x22,0xE7,0x27,0x7D,0xC2,0x26,0x75,0xF5,0x9E,0x95,0xF9,
    0xFB,0xCF,0x1A,0x83,0x33,0xC8,0x7E,0x36,0x72,0x02,0xE8,0x95,0x56,0x95,0x4F,0x20,
    0xF9,0xBC,0x7F,0x34,0xDF,0xEC,0xD0,0x56,0xC5,0x01,0xA0,0x26,0xB5,0x6E,0x3D,0xC6,
    0x1C,0xE0,0xB6,0xB9,0x62,0x51,0xEB,0xF5,0xF2,0x95,0xBB,0x40,0xDB,0x4B,0x39,0xDC,
    0x5F,0x82,0x88,0x6A,0xD8,0x3F,0xEA,0xE3,0x83,0x09,0xD8,0x99,0xB1,0xAF,0x63,0xBF,
    0x48,0xDF,0x20,0x3A,0xFE,0x05,0xA4,0xF5,0xEE,0x25,0x4A,0xF9,0xB0,0x67,0x5D,0x97,
    0x14,0x34,0x69,0x48,0xDE,0xE4,0x22,0x9C,0x27,0x88,0x21,0x87,0x13,0x13,0x2F,0x81
};

static const uint8_t s_rsa_1024_std_crt_key[4 + 128 + 64 * 5] = {
    /* e: Public exponent */
    0x00,0x01,0x00,0x01,
    /* n: Modulus */
    0xED,0x98,0x3D,0xFF,0x82,0x00,0x88,0x75,0xF5,0x53,0xBC,0x53,0x04,0x55,0x69,0xF2,
    0x4F,0x65,0x2A,0x36,0x4E,0x33,0x5B,0xD9,0xDB,0xAE,0x64,0x63,0x5F,0x4F,0xF4,0x0D,
    0x24,0xC2,0x93,0x98,0x57,0xBF,0xAB,0x56,0x67,0xCC,0x00,0xB5,0xC7,0x77,0xCA,0x46,
    0xD1,0xD4,0x97,0x46,0x3E,0x15,0xBB,0x85,0x7C,0xE8,0x75,0x04,0xB5,0xB8,0x05,0xFC,
    0x52,0x1D,0xCA,0x77,0x77,0xE1,0x34,0x5B,0xD4,0x18,0x6B,0x34,0x04,0xCE,0x25,0xC1,
    0xBB,0x41,0x51,0x8B,0x53,0x6C,0x74,0x48,0x4C,0xEA,0x1B,0xF8,0x5B,0xB6,0x03,0x33,
    0x11,0x60,0xFA,0xC3,0x40,0xE3,0x80,0x5A,0x8D,0x24,0x4F,0x91,0x08,0xFA,0x1B,0x59,
    0xBE,0x13,0x92,0x96,0xB2,0x57,0xF3,0xF0,0xBA,0xDA,0x2F,0xDE,0xCC,0x2E,0x09,0xE3,
    /* p: 1st prime */
    0xF8,0x58,0xF6,0x35,0xA8,0xAF,0x85,0x00,0x5F,0xA4,0xE8,0xC4,0x36,0x77,0x80,0x9B,
    0x64,0x85,0x9A,0xC2,0xB0,0x9D,0x03,0x97,0x0A,0x89,0xC1,0x88,0x06,0x34,0xB6,0xA0,
    0x0F,0xE5,0xFB,0xF5,0x4D,0x74,0x67,0xE3,0x56,0xF7,0x5B,0x0D,0x2C,0xD0,0x6E,0x50,
    0xF6,0x11,0xD5,0x68,0x51,0xBD,0x4B,0x19,0x57,0x0B,0xAB,0xD1,0x3F,0x7E,0x4B,0x4B,
    /* q: 2nd prime */
    0xF4,0xEA,0x75,0x86,0xAE,0x4B,0x71,0x0C,0x00,0xF8,0xD6,0x40,0x3E,0x22,0x5D,0xAD,
    0x1C,0x1D,0x08,0x01,0xBF,0x23,0x82,0xD0,0x12,0xA1,0x92,0x66,0x11,0xB5,0x23,0x4B,
    0x13,0xD1,0xC5,0x58,0xC1,0x7F,0x0B,0xA4,0x67,0x31,0x63,0xA1,0xE2,0xAD,0x04,0xAD,
    0x07,0x3D,0xFC,0x80,0x1A,0x84,0x11,0xC3,0x28,0x72,0x4B,0x76,0x71,0xEF,0x44,0xC9,
    /* dp: Private exponent modulo p-1 */
    0xC6,0xFD,0x81,0x4A,0x3D,0x7F,0x65,0xF2,0x86,0xB0,0x7C,0x51,0xBB,0xD2,0xC5,0x19,
    0xBD,0xD1,0xCF,0xCF,0x6F,0xF4,0x5F,0x8C,0x06,0xC5,0x9A,0x6C,0x83,0x8C,0x79,0x48,
    0x87,0x74,0xA0,0x0E,0x4D,0xA1,0x98,0x1C,0x70,0xF2,0x81,0xE2,0x6C,0xA6,0x15,0xBE,
    0x89,0x6C,0x80,0xB2,0xB5,0xCC,0x17,0x00,0x96,0xDE,0xA3,0x2D,0x66,0xD2,0xAB,0x41,
    /* dq: Private exponent modulo q-1 */
    0xB0,0x5E,0xE3,0x71,0xB0,0xB4,0x7D,0x5D,0x52,0x99,0xDF,0xD8,0x75,0xA6,0x53,0xE5,
    0x64,0xD8,0x77,0x3A,0xA2,0xC5,0xD4,0xDF,0x38,0x7B,0x88,0x2C,0xDE,0xD5,0x9D,0xEB,
    0xE4,0x02,0xED,0x66,0x04,0x3A,0x14,0x4D,0x39,0x4E,0xF4,0xDB,0xD0,0x9C,0x1E,0x40,
    0xE8,0x75,0x38,0x0F,0x9C,0x88,0xBD,0x2D,0xD8,0x43,0xAE,0x9A,0x46,0x64,0x33,0x19,
    /* u: Modular inverse of q modulo p */
    0xB8,0xBB,0x7C,0x09,0x78,0x88,0xE7,0x7B,0xCD,0x33,0x44,0xCC,0xC7,0xB2,0x96,0x65,
    0x3E,0xD9,0x0C,0x42,0xDF,0x7D,0x65,0x0B,0x9D,0x3A,0x0C,0xB8,0x98,0x35,0x40,0x4F,
    0x5D,0xFD,0xD3,0x31,0x6C,0xCE,0x73,0x27,0x47,0x0F,0x50,0x87,0x9E,0xAB,0x79,0xF3,
    0x4D,0x03,0xCB,0x96,0x70,0xFB,0x74,0x37,0x3E,0x9A,0x6E,0x21,0x58,0xFF,0xE4,0x7A
};

static const uint8_t s_rsa_1024_std_plaintext[128] = {
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
    0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
    0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
    0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,
    0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
    0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f
};

static const uint8_t s_rsa_1024_std_ciphertext[128] = {
    0xDD,0xDA,0x24,0xE0,0xC2,0x66,0xA7,0x23,0xA4,0x97,0x5C,0xAA,0x34,0x22,0xB0,0x21,
    0xD9,0xEA,0x9A,0xFF,0x89,0x71,0x3A,0xF2,0xEA,0x35,0x7B,0xA1,0xE9,0xDA,0x0D,0x3F,
    0xBF,0xD7,0x8E,0xE5,0xB5,0x83,0xB6,0x51,0xCF,0xEC,0x6D,0x54,0xC3,0xD2,0x24,0xFE,
    0xD5,0x34,0x02,0x12,0x6B,0xF5,0xAD,0x02,0x86,0xEE,0x68,0x8B,0x60,0x31,0xDD,0x50,
    0xE0,0x72,0x4D,0xB8,0xBC,0x44,0xF7,0x4A,0x5B,0x0E,0xFA,0x88,0x6B,0x8E,0x34,0x68,
    0x19,0xA4,0xC1,0x7F,0xCC,0x9A,0xAF,0xE5,0x1D,0x90,0x9D,0x45,0x56,0x5C,0x36,0x15,
    0xFB,0x07,0xD9,0xA8,0x91,0xC8,0x58,0xC3,0x02,0x0F,0x2C,0x04,0x19,0x7E,0xEE,0xA0,
    0xE4,0x5D,0x27,0x44,0x2D,0x33,0xC7,0x02,0xE9,0x59,0x1C,0xC2,0xC6,0xF9,0xE4,0x3E
};

static const demo_rsa_cipher_std_st s_rsa_cipher_std_data_arr[2] = {
    {
        .alg_str = "RSA 1024 cipher with normal keys",
        .key_type = EHSM_KEY_TYPE_RSA_1024,
        .std_key = s_rsa_1024_std_key,
        .std_key_size = sizeof(s_rsa_1024_std_key),
        .std_pub_key_size = 4U,
        .std_priv_key_size = 128U,
        .std_n_size = 128U,
        .std_plaintext = s_rsa_1024_std_plaintext,
        .std_plaintext_size = sizeof(s_rsa_1024_std_plaintext),
        .std_ciphertext = s_rsa_1024_std_ciphertext,
        .std_ciphertext_size = sizeof(s_rsa_1024_std_ciphertext),
    }, {
        .alg_str = "RSA 1024 cipherwith CRT keys",
        .key_type = EHSM_KEY_TYPE_RSA_1024_CRT,
        .std_key = s_rsa_1024_std_crt_key,
        .std_key_size = sizeof(s_rsa_1024_std_crt_key),
        .std_pub_key_size = 4U,
        .std_priv_key_size = 64U * 5U,
        .std_n_size = 128U,
        .std_plaintext = s_rsa_1024_std_plaintext,
        .std_plaintext_size = sizeof(s_rsa_1024_std_plaintext),
        .std_ciphertext = s_rsa_1024_std_ciphertext,
        .std_ciphertext_size = sizeof(s_rsa_1024_std_ciphertext),
    }
};

/**
 * 对标准明文 s_rsa_1024_std_plaintext 消息计算得到 SHA256 hash digest，可用于外部工具验证。
 * static const uint8_t s_rsa_1024_no_padding_sha256_std_digest[32] = {
 *     0x47,0x1F,0xB9,0x43,0xAA,0x23,0xC5,0x11,0xF6,0xF7,0x2F,0x8D,0x16,0x52,0xD9,0xC8,
 *     0x80,0xCF,0xA3,0x92,0xAD,0x80,0x50,0x31,0x20,0x54,0x77,0x03,0xE5,0x6A,0x2B,0xE5
 * };
 */
static const uint8_t s_rsa_1024_no_padding_sha256_std_signature[128] = {
    0xDE,0x76,0x6B,0x41,0xF7,0x22,0x47,0xAD,0xDA,0x4D,0x05,0xE1,0x86,0xB8,0x9D,0xDD,
    0x92,0x2C,0x86,0xD7,0xA4,0x98,0xB3,0x23,0x52,0x9A,0x63,0xA3,0xB1,0xEE,0xFB,0x06,
    0x05,0xB8,0xEB,0xF9,0x73,0xC8,0xFF,0xB3,0x88,0xA4,0xAE,0x5F,0x92,0x0B,0x9A,0xD3,
    0xED,0x4A,0x62,0x6E,0x8D,0xEA,0xE4,0xD4,0xC9,0x0D,0x2C,0x2A,0xDE,0xD6,0x5B,0xDB,
    0xBD,0xFA,0xCB,0xCF,0x1D,0x43,0x1F,0x1B,0xE2,0x40,0x3C,0x97,0xD9,0x1F,0xC0,0x90,
    0xCE,0x3B,0x44,0x16,0x35,0x88,0x42,0xCD,0xBF,0x16,0x1D,0x55,0xE5,0x06,0xF9,0xFA,
    0xC6,0xDB,0x5F,0xAC,0xD6,0xB2,0x9C,0xE5,0x3D,0x4C,0x8F,0xB3,0xFF,0x19,0x70,0x88,
    0x81,0x30,0x9A,0xB6,0xD2,0xA6,0xAF,0x72,0xD0,0x5B,0xEC,0xE2,0xB9,0xDA,0x8B,0xCF
};

/**
 * 对标准明文消息 s_rsa_1024_std_plaintext 计算得到 SHA1 hash digest，可用于外部工具验证。
 * static const uint8_t s_rsa_1024_no_padding_sha1_std_digest[20] = {
 *     0xE6,0x43,0x4B,0xC4,0x01,0xF9,0x86,0x03,0xD7,0xED,0xA5,0x04,0x79,0x0C,0x98,0xC6,
 *     0x73,0x85,0xD5,0x35
 * };
 */
static const uint8_t s_rsa_1024_no_padding_sha1_std_signature[128] = {
    0x8F,0x72,0xFF,0xBA,0xE2,0x32,0x7D,0x9F,0xDA,0xF1,0xD1,0xA8,0x55,0x38,0x88,0x5F,
    0xCF,0x53,0x9A,0x5A,0x01,0x17,0x15,0xF9,0xC5,0x14,0x2B,0x52,0x44,0x56,0x48,0x35,
    0xB7,0x69,0x4A,0xD0,0x91,0x6E,0x00,0xFC,0x0D,0x50,0xB2,0x2A,0xFC,0x7D,0x40,0xC0,
    0xC6,0x35,0x1A,0xB6,0x8A,0x8F,0x11,0xA1,0xC8,0x35,0x67,0x1D,0x66,0x38,0xFA,0x40,
    0x52,0x47,0x91,0x81,0xB0,0x73,0x8D,0xB8,0x6C,0xBE,0x3D,0x0F,0x33,0xE6,0x72,0xF2,
    0xDF,0x70,0x0E,0xB7,0xD7,0x53,0xFD,0xE5,0x00,0xBB,0xBA,0xD7,0xE9,0x60,0xC2,0xE1,
    0x8F,0x1A,0xD5,0xA6,0xA4,0xFE,0xB4,0x80,0x4F,0xAA,0xF6,0x58,0xD6,0xC6,0x0B,0x55,
    0xDB,0xFA,0xAC,0xBE,0xB8,0xCF,0xBA,0x5B,0x5C,0x1E,0x59,0xD9,0xEC,0xE8,0xDF,0xA4
};

/* 签名需要指定 salt 长度，这里采用 64 字节 salt，可用于外部工具验证。*/
static const uint8_t s_rsa_std_salt[64] = {
    0x0A,0x11,0x79,0x19,0xB7,0x18,0x95,0x6E,0xF3,0x9B,0x17,0x6C,0xA3,0xBC,0xB4,0x41,
    0x23,0xD0,0xFC,0x16,0x70,0x34,0xAA,0xED,0xD7,0x91,0x58,0x71,0x99,0x32,0x44,0xA9,
    0x8A,0x74,0xA1,0x20,0xA9,0x9E,0x12,0x98,0xE9,0x76,0xE0,0xA5,0xF5,0xCC,0x55,0xA4,
    0x4D,0x2E,0x67,0xDC,0x29,0x5F,0x9C,0x8D,0x8D,0xC4,0xA9,0x89,0xBD,0xDE,0xCB,0xE0
};

static const uint8_t s_rsa_1024_pss_sha256_std_signature[128] = {
    0xC0,0x6B,0xF3,0xBF,0x71,0xD1,0xEF,0xEE,0xCE,0xD3,0xDC,0x35,0xF2,0x27,0x87,0x37,
    0x30,0x8E,0x8A,0x16,0x8A,0x3E,0xE9,0xF6,0x7E,0x1A,0xDD,0x1B,0xCC,0x25,0x79,0x3F,
    0x97,0x20,0xCB,0xF1,0x15,0xDE,0x1F,0xA8,0x5C,0x96,0x06,0xD9,0xDE,0x13,0x8E,0x93,
    0xC5,0x6D,0xD9,0x8A,0x08,0xB4,0xA1,0xA4,0xD5,0xA2,0xDD,0x27,0x95,0x26,0xB9,0xDE,
    0x09,0x78,0xA8,0xE3,0xC8,0x31,0xAA,0x1F,0x26,0xB0,0x92,0x04,0xA1,0x36,0x2F,0x6A,
    0x59,0x34,0xBD,0x51,0xFB,0xC2,0x79,0xBC,0xF5,0x97,0xBA,0x4E,0xF4,0x34,0x10,0x06,
    0xC9,0xEC,0xFA,0xF2,0x97,0x05,0x97,0x0A,0xDC,0x25,0xBC,0x75,0x07,0x7F,0x75,0x00,
    0x42,0x38,0xE3,0x8F,0x1E,0x59,0x18,0x94,0x5F,0x68,0xCF,0x19,0xC2,0x23,0xF2,0xC7
};

static const uint8_t s_rsa_1024_pss_sha1_std_signature[128] = {
    0x1E,0x5B,0xE0,0xB5,0x51,0x43,0x27,0x69,0x2C,0x46,0xD0,0xE4,0xE9,0xA9,0x80,0x1F,
    0xF9,0x9A,0x26,0x2E,0x21,0xFB,0xF2,0x65,0xDB,0xEB,0x77,0x4C,0x61,0x91,0x6E,0x1D,
    0xF6,0x05,0xA7,0x48,0x86,0x2A,0x7F,0xCA,0x85,0xCF,0xD3,0x2B,0xCF,0xFA,0xEE,0x79,
    0x77,0xC4,0x14,0x9E,0xCF,0x99,0xEF,0x0E,0x94,0xF5,0x4A,0xA1,0x6A,0xFA,0xA9,0x40,
    0x0A,0x53,0x4E,0x3A,0xAD,0x74,0x23,0x28,0xEF,0x9D,0xCA,0x93,0xBC,0x2D,0x59,0x60,
    0x78,0x6D,0x5F,0xC8,0xCE,0xBF,0x1F,0xAD,0x85,0x30,0x6F,0x73,0x75,0x08,0x6D,0xB7,
    0xD6,0xC4,0x36,0x8B,0xEA,0x6E,0x18,0x46,0xB5,0x4E,0xC7,0xDB,0xD7,0x1F,0xD2,0x9D,
    0x7B,0x5E,0xA2,0x32,0x1D,0x3F,0xFF,0xE3,0xED,0xBE,0x3A,0xDE,0x14,0xEA,0x71,0x40
};

static const demo_rsa_sign_std_st s_rsa_sign_std_data_arr[4] = {
    {
        .alg_str = "RSA 1024 signature no padding with normal keys",
        .algo = EHSM_HASH_ALGO_SHA256,
        .padding = EHSM_RSA_PADDING_NONE,
        .key_type = EHSM_KEY_TYPE_RSA_1024,
        .std_key = s_rsa_1024_std_key,
        .std_key_size = sizeof(s_rsa_1024_std_key),
        .std_pub_key_size = 4U,
        .std_priv_key_size = 128U,
        .std_n_size = 128U,
        .std_msg = s_rsa_1024_std_plaintext,
        .std_msg_size = sizeof(s_rsa_1024_std_plaintext),
        .std_signature = s_rsa_1024_no_padding_sha256_std_signature,
        .std_signature_size = sizeof(s_rsa_1024_no_padding_sha256_std_signature),
        .std_salt = NULL,
        .std_salt_size = 0U,
    }, {
        .alg_str = "RSA 1024 signature no padding with CRT keys",
        .algo = EHSM_HASH_ALGO_SHA1,
        .padding = EHSM_RSA_PADDING_NONE,
        .key_type = EHSM_KEY_TYPE_RSA_1024_CRT,
        .std_key = s_rsa_1024_std_crt_key,
        .std_key_size = sizeof(s_rsa_1024_std_crt_key),
        .std_pub_key_size = 4U,
        .std_priv_key_size = 64U * 5U,
        .std_n_size = 128U,
        .std_msg = s_rsa_1024_std_plaintext,
        .std_msg_size = sizeof(s_rsa_1024_std_plaintext),
        .std_signature = s_rsa_1024_no_padding_sha1_std_signature,
        .std_signature_size = sizeof(s_rsa_1024_no_padding_sha1_std_signature),
        .std_salt = NULL,
        .std_salt_size = 0U,
    }, {
        .alg_str = "RSA 1024 signature PSS with normal keys",
        .algo = EHSM_HASH_ALGO_SHA256,
        .padding = EHSM_RSA_PADDING_PSS,
        .key_type = EHSM_KEY_TYPE_RSA_1024,
        .std_key = s_rsa_1024_std_key,
        .std_key_size = sizeof(s_rsa_1024_std_key),
        .std_pub_key_size = 4U,
        .std_priv_key_size = 128U,
        .std_n_size = 128U,
        .std_msg = s_rsa_1024_std_plaintext,
        .std_msg_size = sizeof(s_rsa_1024_std_plaintext),
        .std_signature = s_rsa_1024_pss_sha256_std_signature,
        .std_signature_size = sizeof(s_rsa_1024_pss_sha256_std_signature),
        .std_salt = s_rsa_std_salt,
        .std_salt_size = sizeof(s_rsa_std_salt),
    }, {
        .alg_str = "RSA 1024 signature PSS with CRT keys",
        .algo = EHSM_HASH_ALGO_SHA1,
        .padding = EHSM_RSA_PADDING_PSS,
        .key_type = EHSM_KEY_TYPE_RSA_1024_CRT,
        .std_key = s_rsa_1024_std_crt_key,
        .std_key_size = sizeof(s_rsa_1024_std_crt_key),
        .std_pub_key_size = 4U,
        .std_priv_key_size = 64U * 5U,
        .std_n_size = 128U,
        .std_msg = s_rsa_1024_std_plaintext,
        .std_msg_size = sizeof(s_rsa_1024_std_plaintext),
        .std_signature = s_rsa_1024_pss_sha1_std_signature,
        .std_signature_size = sizeof(s_rsa_1024_pss_sha1_std_signature),
        .std_salt = s_rsa_std_salt,
        .std_salt_size = sizeof(s_rsa_std_salt),
    }
};

// clang-format on

/**
 * @brief 一次性计算 RSA 加解密的示例。
 * @param[in] enc 是否为加密
 *          - true 指加密；
 *          - false 指解密；
 * @param[in] std_data 标准数据，详见 @ref demo_rsa_cipher_std_st
 *
 * @note
 * - 任意一次调用 API 返回错误后，应终止本次计算，并忽略任何中间数据，如 ctx，session 等。
 */
static void rsa_cipher(bool_t enc, const demo_rsa_cipher_std_st *std_data)
{
    uint32_t ret;
    const char *ret_str = NULL;
    ehsm_key_format_st *key_data
        = (ehsm_key_format_st *)ehsm_demo_get_buffer(0); /* 密钥数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t key_data_size;                              /* 密钥数据 buffer 的长度 */
    uint8_t *input = ehsm_demo_get_buffer(10);  /* 输入数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t input_size;                        /* 输入数据的长度 */
    uint8_t *output = ehsm_demo_get_buffer(11); /* 存储输出数据的 buff，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t output_size;                       /* 输出数据的 fuffer 的长度 */
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置信息的 buffer */
    uint32_t key_handle = DEMO_RSA_KEY_HANDLE; /* 用于指定导入密钥的 key handle */
    const uint8_t *std_input = NULL;           /* 用于指定标准输入，加密时是明文，解密时是密文 */
    uint32_t std_input_size;                   /* 用于指定标准输入的总长度 */
    const uint8_t *std_output = NULL;          /* 用于指定标准输出，加密时是密文，解密时是明文 */
    uint32_t std_output_size;                  /* 用于指定标准输出的总长度 */
    bool_t need_remove = false;                /* 用于标记密钥是否需要移除 */

    /* 初始化数据 buffer */
    memset(key_data, 0x0U, 512U);
    memset(input, 0x0U, 512U);
    memset(output, 0x0U, 512U);

    /* 指定输入输出数据 */
    if (enc) {
        /* RSA 签名生成 */
        std_input = std_data->std_plaintext;
        std_input_size = std_data->std_plaintext_size;
        std_output = std_data->std_ciphertext;
        std_output_size = std_data->std_ciphertext_size;
    } else {

        /* RSA 解密 */
        std_input = std_data->std_ciphertext;
        std_input_size = std_data->std_ciphertext_size;
        std_output = std_data->std_plaintext;
        std_output_size = std_data->std_plaintext_size;
    }

    /* 拷贝数据 */
    input_size = std_input_size;
    memcpy(input, std_input, input_size); /* 将标准输入数据拷贝至 SoC 与 eHSM 的共享内存上。 */

    /* 打印密钥、标准输入输出数据。 */
    print_hex("The std e is: \r\n    ", std_data->std_key, std_data->std_pub_key_size);
    print_hex("The std n is: \r\n    ", &(std_data->std_key[std_data->std_pub_key_size]), std_data->std_n_size);
    print_hex("The std private key(d or p||q||dp||dq||u) is: \r\n    ",
        &(std_data->std_key[std_data->std_pub_key_size + std_data->std_n_size]), std_data->std_priv_key_size);
    print_hex("The std input is: \r\n    ", std_input, std_input_size);
    print_hex("The std output is: \r\n    ", std_output, std_output_size);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步，自动忽略回调函数。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /**
     * 导入明文密钥并获取 key handle，若 eHSM RAM 已经存在密钥，则可以省略这一步直接使用对应的 key handle。
     * 这里主要演示计算过程，若想了解密钥导入的具体流程，请参考密钥导入 demo。
     */
    key_data_size = sizeof(ehsm_key_format_st) + std_data->std_key_size;
    demo_wrap_key_data((EHSM_KEY_PRIV_ENCRYPT | EHSM_KEY_PRIV_DECRYPT), std_data->key_type, EHSM_KEY_PART_KEY_PAIR,
        std_data->std_key, std_data->std_key_size, std_data->std_pub_key_size, std_data->std_priv_key_size, key_data,
        &key_data_size);
    ret = ehsm_km_import_key(
        ctx, EHSM_KEY_HANDLE_INVALID, EHSM_KEY_HANDLE_INVALID, key_data, key_data_size, NULL, 0U, &key_handle);
    ret = demo_check_val("The execution of importing key API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        need_remove = true; /* 标记导入的密钥需要移除 */

        /**
         * 2. 调用 RSA cipher API。需要输入所有消息，API 完成计算后会将输出数据返回给 SoC。
         * - output_size:
         *      - 首先作为输入，是输出 buffer 的大小：
         *          - 加密时，必须大于等于 input_size；
         *          - 解密时，必须大于等于 RSA modulus n 长度；
         *      - API 执行完成后作为输出，是输出数据的实际长度，即 RSA modulus n 长度；
         */
        output_size = std_output_size;
        ret = ehsm_rsa_cipher(ctx, key_handle, enc, input, input_size, output, &output_size);
        ret = demo_check_val("The execution of RSA cipher API:", EHSM_OK, ret);

        /* 校验数据 */
        if (EHSM_OK == ret) {
            /* eHSM FW 返回成功，可以直接用标准数据校验 */
            ret = demo_check_data("Output comparison:", std_output, std_output_size, output, output_size);
        }
    }

    if (need_remove) {
        /* 移除密钥 */
        demo_remove_key(ctx, key_handle);
    }

    ret_str = (EHSM_OK == ret) ? "SUCCESS" : "FAILURE";
    ehsm_port_printf("[Ends with %s !!!] \r\n\r\n", ret_str);
}

/**
 * @brief 一次性计算 RSA 签名生成和验证的示例。
 * @param[in] gen_sig 是否为签名生成
 *          - true 指签名生成；
 *          - false 指签名验证；
 * @param[in] std_data 标准数据，详见 @ref demo_rsa_sign_std_st
 *
 * @note
 * - 任意一次调用 API 返回错误后，应终止本次计算，并忽略任何中间数据，如 ctx，session 等。
 * - 对于签名生成，采用先调用签名生成 API 对明文消息签名，再调用签名验证 API 对产生的签名进行验证。
 * - 对于签名验证，直接用标准数据进行验证即可。
 * - 为确保 eHSM 的 API 都是正常的，应当先调用签名验证接口，对标准数据进行验证，证明 eHSM 的 API
 * 和功能是符合标准的，然后再采用先签名，后验签的方式验证签名 API 的功能。
 * - 若为 PSS 签名，需要指定 salt 长度：
 *      - 若为 PSS 签名生成，eHSM 会随机生成指定长度的 salt 用于签名生成；
 *      - 若为 PSS 签名验证，eHSM 会根据指定的 salt 长度进行验证；
 */
static void rsa_sign_ver_onepass(bool_t gen_sig, const demo_rsa_sign_std_st *std_data)
{
    uint32_t ret;
    const char *ret_str = NULL;
    ehsm_key_format_st *key_data
        = (ehsm_key_format_st *)ehsm_demo_get_buffer(0); /* 密钥数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t key_data_size;                              /* 密钥数据 buffer 的长度 */
    uint8_t *msg = ehsm_demo_get_buffer(10);       /* 输入消息的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t msg_size;                             /* 输入消息的长度 */
    uint8_t *signature = ehsm_demo_get_buffer(11); /* 存储签名的 buff，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t signature_size;                       /* 签名的 fuffer 的长度 */
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置信息的 buffer */
    uint32_t key_handle = DEMO_RSA_KEY_HANDLE; /* 用于指定导入密钥的 key handle */
    bool_t verify_result = false;              /* 用于接收签名验证的结果 */
    bool_t need_remove = false;                /* 用于标记密钥是否需要移除 */

    /* 初始化数据 buffer */
    memset(key_data, 0x0U, 512U);
    memset(msg, 0x0U, 512U);
    memset(signature, 0x0U, 512U);

    /* 拷贝数据 */
    msg_size = std_data->std_msg_size;
    memcpy(msg, std_data->std_msg, msg_size); /* 将标准输入数据拷贝至 SoC 与 eHSM 的共享内存上。 */
    if (!gen_sig) {
        /* 验签需要输入签名，将标准签名拷贝至 SoC 与 eHSM 的共享内存上。 */
        memcpy(signature, std_data->std_signature, std_data->std_signature_size);
    }

    /* 打印密钥、标准消息、签名。 */
    print_hex("The std e is: \r\n    ", std_data->std_key, std_data->std_pub_key_size);
    print_hex("The std n is: \r\n    ", &(std_data->std_key[std_data->std_pub_key_size]), std_data->std_n_size);
    print_hex("The std private key(d or p||q||dp||dq||u) is: \r\n    ",
        &(std_data->std_key[std_data->std_pub_key_size + std_data->std_n_size]), std_data->std_priv_key_size);
    print_hex("The std msg is: \r\n    ", std_data->std_msg, std_data->std_msg_size);
    print_hex("The std signature is: \r\n    ", std_data->std_signature, std_data->std_signature_size);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步，自动忽略回调函数。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /**
     * 导入明文密钥并获取 key handle，若 eHSM RAM 已经存在密钥，则可以省略这一步直接使用对应的 key handle。
     * 这里主要演示计算过程，若想了解密钥导入的具体流程，请参考密钥导入 demo。
     */
    key_data_size = sizeof(ehsm_key_format_st) + std_data->std_key_size;
    demo_wrap_key_data((EHSM_KEY_PRIV_SIGN | EHSM_KEY_PRIV_VERIFY), std_data->key_type, EHSM_KEY_PART_KEY_PAIR,
        std_data->std_key, std_data->std_key_size, std_data->std_pub_key_size, std_data->std_priv_key_size, key_data,
        &key_data_size);
    ret = ehsm_km_import_key(
        ctx, EHSM_KEY_HANDLE_INVALID, EHSM_KEY_HANDLE_INVALID, key_data, key_data_size, NULL, 0U, &key_handle);
    ret = demo_check_val("The execution of importing key API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        need_remove = true; /* 标记导入的密钥需要移除 */

        if (gen_sig) {
            /**
             * 2. 调用 RSA 签名生成 API。需要输入所有消息，API 完成计算后会将签名返回给 SoC。
             * - signature_size：指定 signature buffer 的长度，必须大于等于 modulus n 长度，签名的长度固定为 modulus n
             * 长度。
             * - salt_size:
             *      - 若无 padding，则为 0；
             *      - 若为 PSS，则为实际的 salt 长度；
             */
            signature_size = std_data->std_signature_size;
            ret = ehsm_rsa_sign_onepass_gen(ctx, std_data->algo, key_handle, std_data->padding, msg, msg_size,
                signature, &signature_size, std_data->std_salt_size);
            ret = demo_check_val("The execution of sign onepass API:", EHSM_OK, ret);
            if (EHSM_OK == ret) {
                /*由于签名生成含有随机数，因此无法直接校输出的签名，采用先签名，后验签的方式校验 */
                print_hex("The generated signature is:\r\n    ", signature, signature_size);

                /* 这里仅仅为了验证 eHSM 生成的签名，因此采用 onepass API。 */
                ret = ehsm_rsa_sign_onepass_verify(ctx, std_data->algo, key_handle, std_data->padding, msg, msg_size,
                    signature, signature_size, std_data->std_salt_size, &verify_result);
                ret = demo_check_val(
                    "The execution of verify onepass API for verifing generated signature:", EHSM_OK, ret);
            }
        } else {
            /**
             * 2. 调用 RSA 签名验证 API。需要输入所有消息，API 完成计算后会将验证结果返回给 SoC。
             * - signature_size：指定 signature的长度，必须等于 modulus n 长度。
             * - salt_size:
             *      - 若无 padding，则为 0；
             *      - 若为 PSS，则为实际的 salt 长度；
             */
            signature_size = std_data->std_signature_size;
            ret = ehsm_rsa_sign_onepass_verify(ctx, std_data->algo, key_handle, std_data->padding, msg, msg_size,
                signature, signature_size, std_data->std_salt_size, &verify_result);
            ret = demo_check_val("The execution of onepass API:", EHSM_OK, ret);
            if (EHSM_OK == ret) {
                /** eHSM FW 返回成功，检查签名验证结果 */
                ret = demo_check_val("Signature verification:", true, verify_result);
            }
        }
    }

    if (need_remove) {
        /* 移除密钥 */
        demo_remove_key(ctx, key_handle);
    }

    ret_str = (EHSM_OK == ret) ? "SUCCESS" : "FAILURE";
    ehsm_port_printf("[Ends with %s !!!] \r\n\r\n", ret_str);
}

/**
 * @brief 分段计算 RSA 签名生成和验证的示例。
 * @param[in] gen_sig 是否为签名生成
 *          - true 指签名生成；
 *          - false 指签名验证；
 * @param[in] std_data 标准数据，详见 @ref demo_rsa_sign_std_st
 *
 * @note
 * - 任意一次调用 API 返回错误后，应终止本次计算，并忽略任何中间数据，如 ctx，session 等。
 * - 对于签名生成，采用先调用签名生成 API 对明文消息签名，再调用签名验证 API 对产生的签名进行验证。
 * - 对于签名验证，直接用标准数据进行验证即可。
 * - 为确保 eHSM 的 API 都是正常的，应当先调用签名验证接口，对标准数据进行验证，证明 eHSM 的 API
 * 和功能是符合标准的，然后再采用先签名，后验签的方式验证签名 API 的功能。
 * - 对于 init API，主要是初始化 hash 算法。
 * - 对于 update API，主要是对输入的消息计算 hash digest，所有的消息必须在 update 阶段输入。
 * - 对于 finish API，主要是对 update 阶段迭代计算的 hash 值进行签名，请确保已经通过 update API 将所有的消息输入。
 * 若为 PSS 签名，需要指定 salt 长度：
 *      - 若为 PSS 签名生成，eHSM 会随机生成指定长度的 salt 用于签名生成；
 *      - 若为 PSS 签名验证，eHSM 会根据指定的 salt 长度进行验证；
 */
static void rsa_sign_ver_stepwise(bool_t gen_sig, const demo_rsa_sign_std_st *std_data)
{
    uint32_t ret;
    const char *ret_str = NULL;
    ehsm_session_st *session = ehsm_demo_get_session(); /* eHSM 分段计算的会话 buffer，用于存储 eHSM
                                                           FW 计算的中间数据，必须是 SoC 与 eHSM 的共享内存 */
    ehsm_key_format_st *key_data
        = (ehsm_key_format_st *)ehsm_demo_get_buffer(0); /* 密钥数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t key_data_size;                              /* 密钥数据 buffer 的长度 */
    uint8_t *msg = ehsm_demo_get_buffer(10);       /* 输入消息的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t msg_size;                             /* 输入消息的长度 */
    uint8_t *signature = ehsm_demo_get_buffer(11); /* 存储签名的 buff，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t signature_size;                       /* 签名的 fuffer 的长度 */
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置信息的 buffer */
    bool_t verify_result = false;           /* 用于接收签名验证的结果 */
    uint32_t already_inputed_size;          /* 多次更新时，记录已经输入的数据总长度 */
    uint32_t key_handle = DEMO_RSA_KEY_HANDLE; /* 用于指定导入密钥的 key handle */
    bool_t need_remove = false;                /* 用于标记密钥是否需要移除 */

    /* 初始化数据 buffer */
    memset(key_data, 0x0U, 512U);
    memset(msg, 0x0U, 512U);
    memset(signature, 0x0U, 512U);

    /* 拷贝数据 */
    msg_size = std_data->std_msg_size;
    memcpy(msg, std_data->std_msg, msg_size); /* 将标准输入数据拷贝至 SoC 与 eHSM 的共享内存上。 */
    if (!gen_sig) {
        /* 验签需要输入签名，将标准签名拷贝至 SoC 与 eHSM 的共享内存上。 */
        memcpy(signature, std_data->std_signature, std_data->std_signature_size);
    }

    /* 打印密钥、标准消息、签名。 */
    print_hex("The std e is: \r\n    ", std_data->std_key, std_data->std_pub_key_size);
    print_hex("The std n is: \r\n    ", &(std_data->std_key[std_data->std_pub_key_size]), std_data->std_n_size);
    print_hex("The std private key(d or p||q||dp||dq||u) is: \r\n    ",
        &(std_data->std_key[std_data->std_pub_key_size + std_data->std_n_size]), std_data->std_priv_key_size);
    print_hex("The std msg is: \r\n    ", std_data->std_msg, std_data->std_msg_size);
    print_hex("The std signature is: \r\n    ", std_data->std_signature, std_data->std_signature_size);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步，自动忽略回调函数。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /**
     * 导入明文密钥并获取 key handle，若 eHSM RAM 已经存在密钥，则可以省略这一步直接使用对应的 key handle。
     * 这里主要演示计算过程，若想了解密钥导入的具体流程，请参考密钥导入 demo。
     */
    key_data_size = sizeof(ehsm_key_format_st) + std_data->std_key_size;
    demo_wrap_key_data((EHSM_KEY_PRIV_SIGN | EHSM_KEY_PRIV_VERIFY), std_data->key_type, EHSM_KEY_PART_KEY_PAIR,
        std_data->std_key, std_data->std_key_size, std_data->std_pub_key_size, std_data->std_priv_key_size, key_data,
        &key_data_size);
    ret = ehsm_km_import_key(
        ctx, EHSM_KEY_HANDLE_INVALID, EHSM_KEY_HANDLE_INVALID, key_data, key_data_size, NULL, 0U, &key_handle);
    ret = demo_check_val("The execution of importing key API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        need_remove = true; /* 标记导入的密钥需要移除 */

        /* 2. 调用 init API。需要输入密钥 handle、生成/验证方向、分段计算的会话 buffer。*/
        ret = ehsm_rsa_sign_init(ctx, std_data->algo, key_handle, gen_sig, std_data->padding, session);
        ret = demo_check_val("The execution of init API:", EHSM_OK, ret);
    }

    if (EHSM_OK == ret) {
        /* 3.1 第一次调用 update API 更新，假设本次更新 16 字节的消息。*/
        msg_size = 16U;
        ret = ehsm_rsa_sign_update(ctx, msg, msg_size);
        ret = demo_check_val("The 1st execution of update API:", EHSM_OK, ret);
        already_inputed_size = msg_size;

        /* 3.1 第二次调用 update API 更新，假设本次更新剩余所有消息。*/
        msg_size = std_data->std_msg_size - already_inputed_size;
        ret = ehsm_rsa_sign_update(ctx, &msg[already_inputed_size], msg_size);
        ret = demo_check_val("The 2nd execution of update API:", EHSM_OK, ret);
        already_inputed_size += msg_size;
    }

    if (EHSM_OK == ret) {
        if (gen_sig) { /* 签名生成 */
            /**
             * 4. 调用 finish API。需要输入所有消息，API 完成计算后会将签名返回给 SoC。
             * - signature_size：指定 signature buffer 的长度，必须大于等于 modulus n 长度，签名的长度固定为 modulus n
             * 长度。
             * - salt_size:
             *      - 若无 padding，则为 0；
             *      - 若为 PSS，则为实际的 salt 长度；
             */
            signature_size = std_data->std_signature_size;
            ret = ehsm_rsa_sign_finish_gen(ctx, signature, &signature_size, std_data->std_salt_size);
            ret = demo_check_val("The execution of finish API:", EHSM_OK, ret);

            if (EHSM_OK == ret) {
                /* 这里采用先签名，后验签的方式校验 */
                print_hex("The generated signature is:\r\n    ", signature, signature_size);

                /* 这里仅仅为了验证 eHSM 生成的签名，因此采用 onepass API。 */
                ret = ehsm_rsa_sign_onepass_verify(ctx, std_data->algo, key_handle, std_data->padding, msg, msg_size,
                    signature, signature_size, std_data->std_salt_size, &verify_result);
                ret = demo_check_val(
                    "The execution of verify onepass API for verifing generated signature:", EHSM_OK, ret);
            }
        } else { /* 签名验证 */
            /**
             * 4. 调用 finish API。需要输入所有消息，API 完成计算后会将验证结果返回给 SoC。
             * - signature_size：指定 signature的长度，必须等于 modulus n 长度。
             * - salt_size:
             *      - 若无 padding，则为 0；
             *      - 若为 PSS，则为实际的 salt 长度；
             */
            signature_size = std_data->std_signature_size;
            ret = ehsm_rsa_sign_finish_verify(ctx, signature, signature_size, std_data->std_salt_size, &verify_result);
            ret = demo_check_val("The execution of finish API:", EHSM_OK, ret);
            if (EHSM_OK == ret) {
                /** eHSM FW 返回成功，检查签名验证结果 */
                ret = demo_check_val("Signature verification:", true, verify_result);
            }
        }
    }

    if (need_remove) {
        /* 移除密钥 */
        demo_remove_key(ctx, key_handle);
    }

    ret_str = (EHSM_OK == ret) ? "SUCCESS" : "FAILURE";
    ehsm_port_printf("[Ends with %s !!!] \r\n\r\n", ret_str);
}

/**
 * @brief 一次性计算 RSA 加解密的示例，使用明文密钥。
 * @param[in] enc 是否为加密
 *          - true 指加密；
 *          - false 指解密；
 * @param[in] std_data 标准数据，详见 @ref demo_rsa_cipher_std_st
 *
 * @note
 * - 任意一次调用 API 返回错误后，应终止本次计算，并忽略任何中间数据，如 ctx，session 等。
 * - 对于加密，由于加密过程中引入随机数，eHSM 产生的密文和标准密文不一致，无法直接通过比对数据校验，因此先调用
 * 加密 API 对明文消息加密，再调用解密 API 对产生的密文进行解密，最后比对 eHSM 产生的明文和标准数据。
 * - 对于解密，直接用标准数据进行验证即可。
 * - 为确保 eHSM 的 API 都是正常的，应当先调用解密接口，对标准数据进行验证，证明 eHSM 的 API
 * 和功能是符合标准的，然后再采用先加密，后解密的方式验证加密 API 的功能。
 */
static void rsa_cipher_with_plain_key(bool_t enc, const demo_rsa_cipher_std_st *std_data)
{
    uint32_t ret;
    const char *ret_str = NULL;
    uint8_t *key = ehsm_demo_get_buffer(0);    /* 存储密钥值的buffer */
    uint8_t *input = ehsm_demo_get_buffer(1);  /* 输入数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t input_size;                       /* 输入数据的长度 */
    uint8_t *output = ehsm_demo_get_buffer(2); /* 存储输出数据的 buff，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t output_size;                      /* 输出数据的 fuffer 的长度 */
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置信息的 buffer */
    const uint8_t *std_input = NULL;        /* 用于指定标准输入，加密时是明文，解密时是密文 */
    uint32_t std_input_size;                /* 用于指定标准输入的总长度 */
    const uint8_t *std_output = NULL;       /* 用于指定标准输出，加密时是密文，解密时是明文 */
    uint32_t std_output_size;               /* 用于指定标准输出的总长度 */
    ehsm_rsa_key_st *rsa_key = (ehsm_rsa_key_st *)ehsm_demo_get_buffer(3); /* 存储RSA密钥结构的buffer */

    /* 初始化数据 buffer */
    memset(input, 0x0U, 512U);
    memset(output, 0x0U, 512U);

    /* 准备密钥数据，存储在rsa_key中*/
    memcpy(key, std_data->std_key, std_data->std_key_size);

    /* 设置RSA密钥结构 */
    rsa_key->crt_mode
        = (std_data->key_type == EHSM_KEY_TYPE_RSA_1024_CRT || std_data->key_type == EHSM_KEY_TYPE_RSA_2048_CRT
              || std_data->key_type == EHSM_KEY_TYPE_RSA_3072_CRT || std_data->key_type == EHSM_KEY_TYPE_RSA_4096_CRT)
        ? 1U
        : 0U;
    rsa_key->n_byte_sz = std_data->std_n_size;
    rsa_key->e_byte_sz = std_data->std_pub_key_size;
    rsa_key->e = ehsm_port_addr_to_raddr(key);
    rsa_key->n = ehsm_port_addr_to_raddr(&key[std_data->std_pub_key_size]);

    if (rsa_key->crt_mode) {
        /* CRT模式：密钥格式为 e||n||p||q||dp||dq||u */
        uint32_t offset = std_data->std_pub_key_size + std_data->std_n_size;
        uint32_t crt_param_size = std_data->std_priv_key_size / 5U; /* p, q, dp, dq, u 各占1/5 */
        rsa_key->p = ehsm_port_addr_to_raddr(&key[offset]);
        rsa_key->q = ehsm_port_addr_to_raddr(&key[offset + crt_param_size]);
        rsa_key->dp = ehsm_port_addr_to_raddr(&key[offset + 2U * crt_param_size]);
        rsa_key->dq = ehsm_port_addr_to_raddr(&key[offset + 3U * crt_param_size]);
        rsa_key->u = ehsm_port_addr_to_raddr(&key[offset + 4U * crt_param_size]);
        rsa_key->d = 0U; /* CRT模式不使用d */
    } else {
        /* 标准模式：密钥格式为 e||n||d */
        rsa_key->d = ehsm_port_addr_to_raddr(&key[std_data->std_pub_key_size + std_data->std_n_size]);
        rsa_key->p = 0U;
        rsa_key->q = 0U;
        rsa_key->dp = 0U;
        rsa_key->dq = 0U;
        rsa_key->u = 0U;
    }

    /* 指定输入输出数据 */
    if (enc) {
        /* RSA 加密 */
        std_input = std_data->std_plaintext;
        std_input_size = std_data->std_plaintext_size;
        std_output = std_data->std_ciphertext;
        std_output_size = std_data->std_ciphertext_size;
    } else {
        /* RSA 解密 */
        std_input = std_data->std_ciphertext;
        std_input_size = std_data->std_ciphertext_size;
        std_output = std_data->std_plaintext;
        std_output_size = std_data->std_plaintext_size;
    }

    /* 拷贝数据 */
    input_size = std_input_size;
    memcpy(input, std_input, input_size); /* 将标准输入数据拷贝至 SoC 与 eHSM 的共享内存上。 */

    /* 打印密钥、标准输入输出数据。 */
    print_hex("The std e is: \r\n    ", std_data->std_key, std_data->std_pub_key_size);
    print_hex("The std n is: \r\n    ", &(std_data->std_key[std_data->std_pub_key_size]), std_data->std_n_size);
    print_hex("The std private key(d or p||q||dp||dq||u) is: \r\n    ",
        &(std_data->std_key[std_data->std_pub_key_size + std_data->std_n_size]), std_data->std_priv_key_size);
    print_hex("The std input is: \r\n    ", std_input, std_input_size);
    print_hex("The std output is: \r\n    ", std_output, std_output_size);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步，自动忽略回调函数。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /**
     * 2. 调用 RSA cipher API，使用明文密钥。需要输入所有消息，API 完成计算后会将输出数据返回给 SoC。
     * - output_size:
     *      - 首先作为输入，是输出 buffer 的大小，必须大于等于模长；
     *      - API 执行完成后作为输出，是输出数据的实际长度；
     */
    output_size = std_output_size;
    ret = ehsm_rsa_cipher_with_plain_key(ctx, (uint8_t *)rsa_key, enc, input, input_size, output, &output_size);
    ret = demo_check_val("The execution of RSA cipher with plain key API:", EHSM_OK, ret);

    /* 校验数据 */
    if (EHSM_OK == ret) {
        if (enc) {
            /* 由于加密含有随机数，因此无法直接校验输出数据，采用先加密，后解密生成明文，再比较明文和标准数据。 */
            print_hex("The generated ciphertext is:\r\n    ", output, output_size);

            /* 将 eHSM 加密后的密文作为输入，调用 RSA 解密，成功执行后的 input 是 eHSM 输出的明文 */
            input_size = std_input_size;
            ret = ehsm_rsa_cipher_with_plain_key(
                ctx, (uint8_t *)rsa_key, (!enc), output, output_size, input, &input_size);
            ret = demo_check_val(
                "The execution of RSA cipher with plain key API for decrypting to get plaintext:", EHSM_OK, ret);

            if (EHSM_OK == ret) {
                /** eHSM FW 返回成功，此时用标准明文数据和 eHSM 先加密，后解密出的明文数据进行对比 */
                ret = demo_check_data("Output comparison:", std_input, std_input_size, input, input_size);
            }
        } else {
            /* 解密，eHSM FW 返回成功，解密可以直接用标准数据校验 */
            ret = demo_check_data("Output comparison:", std_output, std_output_size, output, output_size);
        }
    }

    ret_str = (EHSM_OK == ret) ? "SUCCESS" : "FAILURE";
    ehsm_port_printf("[RSA cipher with plain key ends with %s !!!] \r\n\r\n", ret_str);
}

/**
 * @brief 一次性计算 RSA 签名生成和验证的示例，使用明文密钥。
 * @param[in] gen_sig 是否为签名生成
 *          - true 指签名生成；
 *          - false 指签名验证；
 * @param[in] std_data 标准数据，详见 @ref demo_rsa_sign_std_st
 *
 * @note
 * - 任意一次调用 API 返回错误后，应终止本次计算，并忽略任何中间数据，如 ctx，session 等。
 * - 对于签名生成，由于签名过程中引入随机数，eHSM 产生的签名和标准签名不一致，无法直接通过比对数据校验，
 * 因此采用先调用签名生成 API 对明文消息签名，再调用签名验证 API 对产生的签名进行验证。
 * - 对于签名验证，直接用标准数据进行 验证即可。
 * - 为确保 eHSM 的 API 都是正常的，应当先调用签名验证接口，对标准数据进行验证，证明 eHSM 的 API
 * 和功能是符合标准的，然后再采用先签名，后验签的方式验证签名 API 的功能。
 */
static void rsa_sign_ver_onepass_with_plain_key(bool_t gen_sig, const demo_rsa_sign_std_st *std_data)
{
    uint32_t ret;
    const char *ret_str = NULL;
    uint8_t *key = ehsm_demo_get_buffer(0);       /* 存储密钥值的buffer */
    uint8_t *msg = ehsm_demo_get_buffer(1);       /* 输入消息的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t msg_size;                            /* 输入消息的长度 */
    uint8_t *signature = ehsm_demo_get_buffer(2); /* 存储签名的 buff，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t signature_size;                      /* 签名的 fuffer 的长度 */
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置信息的 buffer */
    bool_t verify_result = false;           /* 用于接收签名验证的结果 */
    ehsm_rsa_key_st *rsa_key = (ehsm_rsa_key_st *)ehsm_demo_get_buffer(3); /* 存储RSA密钥结构的buffer */

    /* 初始化数据 buffer */
    memset(msg, 0x0U, 512U);
    memset(signature, 0x0U, 512U);

    /* 准备密钥数据，存储在rsa_key中*/
    memcpy(key, std_data->std_key, std_data->std_key_size);

    /* 设置RSA密钥结构 */
    rsa_key->crt_mode
        = (std_data->key_type == EHSM_KEY_TYPE_RSA_1024_CRT || std_data->key_type == EHSM_KEY_TYPE_RSA_2048_CRT
              || std_data->key_type == EHSM_KEY_TYPE_RSA_3072_CRT || std_data->key_type == EHSM_KEY_TYPE_RSA_4096_CRT)
        ? 1U
        : 0U;
    rsa_key->n_byte_sz = std_data->std_n_size;
    rsa_key->e_byte_sz = std_data->std_pub_key_size;
    rsa_key->e = ehsm_port_addr_to_raddr(key);
    rsa_key->n = ehsm_port_addr_to_raddr(&key[std_data->std_pub_key_size]);

    if (rsa_key->crt_mode) {
        /* CRT模式：密钥格式为 e||n||p||q||dp||dq||u */
        uint32_t offset = std_data->std_pub_key_size + std_data->std_n_size;
        uint32_t crt_param_size = std_data->std_priv_key_size / 5U; /* p, q, dp, dq, u 各占1/5 */
        rsa_key->p = ehsm_port_addr_to_raddr(&key[offset]);
        rsa_key->q = ehsm_port_addr_to_raddr(&key[offset + crt_param_size]);
        rsa_key->dp = ehsm_port_addr_to_raddr(&key[offset + 2U * crt_param_size]);
        rsa_key->dq = ehsm_port_addr_to_raddr(&key[offset + 3U * crt_param_size]);
        rsa_key->u = ehsm_port_addr_to_raddr(&key[offset + 4U * crt_param_size]);
        rsa_key->d = 0U; /* CRT模式不使用d */
    } else {
        /* 标准模式：密钥格式为 e||n||d */
        rsa_key->d = ehsm_port_addr_to_raddr(&key[std_data->std_pub_key_size + std_data->std_n_size]);
        rsa_key->p = 0U;
        rsa_key->q = 0U;
        rsa_key->dp = 0U;
        rsa_key->dq = 0U;
        rsa_key->u = 0U;
    }

    /* 拷贝数据 */
    msg_size = std_data->std_msg_size;
    memcpy(msg, std_data->std_msg, msg_size); /* 将标准输入数据拷贝至 SoC 与 eHSM 的共享内存上。 */
    if (!gen_sig) {
        /* 验签需要输入签名，将标准签名拷贝至 SoC 与 eHSM 的共享内存上。 */
        memcpy(signature, std_data->std_signature, std_data->std_signature_size);
    }

    /* 打印密钥、标准消息、签名。 */
    print_hex("The std e is: \r\n    ", std_data->std_key, std_data->std_pub_key_size);
    print_hex("The std n is: \r\n    ", &(std_data->std_key[std_data->std_pub_key_size]), std_data->std_n_size);
    print_hex("The std private key(d or p||q||dp||dq||u) is: \r\n    ",
        &(std_data->std_key[std_data->std_pub_key_size + std_data->std_n_size]), std_data->std_priv_key_size);
    print_hex("The std msg is: \r\n    ", std_data->std_msg, std_data->std_msg_size);
    print_hex("The std signature is: \r\n    ", std_data->std_signature, std_data->std_signature_size);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步，自动忽略回调函数。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    if (gen_sig) {
        /**
         * 2. 调用 RSA 签名生成 API，使用明文密钥。需要输入所有消息，API 完成计算后会将签名返回给 SoC。
         * - signature_size：指定 signature buffer 的长度，必须大于等于模长；
         */
        signature_size = std_data->std_signature_size;
        ret = ehsm_rsa_sign_onepass_gen_with_plain_key(ctx, std_data->algo, (uint8_t *)rsa_key, std_data->padding, msg,
            msg_size, signature, &signature_size, std_data->std_salt_size);
        ret = demo_check_val("The execution of RSA sign onepass with plain key API:", EHSM_OK, ret);
        if (EHSM_OK == ret) {
            /*由于签名生成含有随机数，因此无法直接校输出的签名，采用先签名，后验签的方式校验 */
            print_hex("The generated signature is:\r\n    ", signature, signature_size);

            /* 这里仅仅为了验证 eHSM 生成的签名，因此采用 onepass API。 */
            ret = ehsm_rsa_sign_onepass_verify_with_plain_key(ctx, std_data->algo, (uint8_t *)rsa_key,
                std_data->padding, msg, msg_size, signature, signature_size, std_data->std_salt_size, &verify_result);
            ret = demo_check_val(
                "The execution of RSA verify onepass with plain key API for verifing generated signature:", EHSM_OK,
                ret);
        }
    } else {
        /* 2. 调用 RSA 签名验证 API，使用明文密钥。需要输入所有消息，API 完成计算后会将验证结果返回给 SoC。*/
        signature_size = std_data->std_signature_size;
        ret = ehsm_rsa_sign_onepass_verify_with_plain_key(ctx, std_data->algo, (uint8_t *)rsa_key, std_data->padding,
            msg, msg_size, signature, signature_size, std_data->std_salt_size, &verify_result);
        ret = demo_check_val("The execution of RSA verify onepass with plain key API:", EHSM_OK, ret);
        if (EHSM_OK == ret) {
            /** eHSM FW 返回成功，检查签名验证结果 */
            ret = demo_check_val("RSA signature verification:", true, verify_result);
        }
    }

    ret_str = (EHSM_OK == ret) ? "SUCCESS" : "FAILURE";
    ehsm_port_printf("[RSA sign with plain key ends with %s !!!] \r\n\r\n", ret_str);
}

/**
 * @brief 分段计算 RSA 签名生成和验证的示例，使用明文密钥。
 * @param[in] gen_sig 是否为签名生成
 *          - true 指签名生成；
 *          - false 指签名验证；
 * @param[in] std_data 标准数据，详见 @ref demo_rsa_sign_std_st
 *
 * @note
 * - 任意一次调用 API 返回错误后，应终止本次计算，并忽略任何中间数据，如 ctx，session 等。
 * - 对于签名生成，由于签名过程中引入随机数，eHSM 产生的签名和标准签名不一致，无法直接通过比对数据校验，
 * 因此采用先调用签名生成 API 对明文消息签名，再调用签名验证 API 对产生的签名进行验证。
 * - 对于签名验证，直接用标准数据进行 验证即可。
 * - 为确保 eHSM 的 API 都是正常的，应当先调用签名验证接口，对标准数据进行验证，证明 eHSM 的 API
 * 和功能是符合标准的，然后再采用先签名，后验签的方式验证签名 API 的功能。
 * - 对于 init API，主要是初始化 hash 算法，设置填充模式和签名方向；
 * - 对于 update API，主要是对输入的消息计算 hash digest，所有的消息必须在 update 阶段输入。
 * - 对于 finish API，主要是对 update 阶段迭代计算的 hash 值进行签名，请确保已经通过 update API 将所有的消息输入。
 */
static void rsa_sign_ver_stepwise_with_plain_key(bool_t gen_sig, const demo_rsa_sign_std_st *std_data)
{
    uint32_t ret;
    const char *ret_str = NULL;
    ehsm_session_st *session = ehsm_demo_get_session(); /* eHSM 分段计算的会话 buffer，用于存储 eHSM
                                                           FW 计算的中间数据，必须是 SoC 与 eHSM 的共享内存 */
    uint8_t *key = ehsm_demo_get_buffer(0);             /* 存储密钥值的buffer */
    uint8_t *msg = ehsm_demo_get_buffer(1);       /* 输入消息的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t msg_size;                            /* 输入消息的长度 */
    uint8_t *signature = ehsm_demo_get_buffer(2); /* 存储签名的 buff，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t signature_size;                      /* 签名的 fuffer 的长度 */
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置信息的 buffer */
    uint32_t already_inputed_size;          /* 多次更新时，记录已经输入的数据总长度 */
    bool_t verify_result = false;           /* 用于接收签名验证的结果 */
    ehsm_rsa_key_st *rsa_key = (ehsm_rsa_key_st *)ehsm_demo_get_buffer(3); /* 存储RSA密钥结构的buffer */

    /* 初始化数据 buffer */
    memset(session, 0x0U, sizeof(ehsm_session_st));
    memset(msg, 0x0U, 512U);
    memset(signature, 0x0U, 512U);

    /* 准备密钥数据，存储在rsa_key中*/
    memcpy(key, std_data->std_key, std_data->std_key_size);

    /* 设置RSA密钥结构 */
    rsa_key->crt_mode
        = (std_data->key_type == EHSM_KEY_TYPE_RSA_1024_CRT || std_data->key_type == EHSM_KEY_TYPE_RSA_2048_CRT
              || std_data->key_type == EHSM_KEY_TYPE_RSA_3072_CRT || std_data->key_type == EHSM_KEY_TYPE_RSA_4096_CRT)
        ? 1U
        : 0U;
    rsa_key->n_byte_sz = std_data->std_n_size;
    rsa_key->e_byte_sz = std_data->std_pub_key_size;
    rsa_key->e = ehsm_port_addr_to_raddr(key);
    rsa_key->n = ehsm_port_addr_to_raddr(&key[std_data->std_pub_key_size]);

    if (rsa_key->crt_mode) {
        /* CRT模式：密钥格式为 e||n||p||q||dp||dq||u */
        uint32_t offset = std_data->std_pub_key_size + std_data->std_n_size;
        uint32_t crt_param_size = std_data->std_priv_key_size / 5U; /* p, q, dp, dq, u 各占1/5 */
        rsa_key->p = ehsm_port_addr_to_raddr(&key[offset]);
        rsa_key->q = ehsm_port_addr_to_raddr(&key[offset + crt_param_size]);
        rsa_key->dp = ehsm_port_addr_to_raddr(&key[offset + 2U * crt_param_size]);
        rsa_key->dq = ehsm_port_addr_to_raddr(&key[offset + 3U * crt_param_size]);
        rsa_key->u = ehsm_port_addr_to_raddr(&key[offset + 4U * crt_param_size]);
        rsa_key->d = 0U; /* CRT模式不使用d */
    } else {
        /* 标准模式：密钥格式为 e||n||d */
        rsa_key->d = ehsm_port_addr_to_raddr(&key[std_data->std_pub_key_size + std_data->std_n_size]);
        rsa_key->p = 0U;
        rsa_key->q = 0U;
        rsa_key->dp = 0U;
        rsa_key->dq = 0U;
        rsa_key->u = 0U;
    }

    /* 拷贝数据 */
    memcpy(msg, std_data->std_msg, std_data->std_msg_size); /* 将标准输入数据拷贝至 SoC 与 eHSM 的共享内存上。 */
    if (!gen_sig) {
        /* 验签需要输入签名，将标准签名拷贝至 SoC 与 eHSM 的共享内存上。 */
        memcpy(signature, std_data->std_signature, std_data->std_signature_size);
    }

    /* 打印密钥、标准消息、签名。 */
    print_hex("The std e is: \r\n    ", std_data->std_key, std_data->std_pub_key_size);
    print_hex("The std n is: \r\n    ", &(std_data->std_key[std_data->std_pub_key_size]), std_data->std_n_size);
    print_hex("The std private key(d or p||q||dp||dq||u) is: \r\n    ",
        &(std_data->std_key[std_data->std_pub_key_size + std_data->std_n_size]), std_data->std_priv_key_size);
    print_hex("The std msg is: \r\n    ", std_data->std_msg, std_data->std_msg_size);
    print_hex("The std signature is: \r\n    ", std_data->std_signature, std_data->std_signature_size);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步，自动忽略回调函数。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /* 2. 调用 init API，使用明文密钥。需要输入 hash 算法，密钥、生成/验证方向、分段计算的会话 buffer。*/
    ret = ehsm_rsa_sign_init_with_plain_key(
        ctx, std_data->algo, (uint8_t *)rsa_key, gen_sig, std_data->padding, session);
    ret = demo_check_val("The execution of RSA sign init with plain key API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        /* 3.1 第一次调用 update API 更新，假设本次更新 16 字节的消息。*/
        msg_size = 16U;
        ret = ehsm_rsa_sign_update(ctx, msg, msg_size);
        ret = demo_check_val("The 1st execution of update API:", EHSM_OK, ret);
        already_inputed_size = msg_size;

        /* 3.2 第二次调用 update API 更新，假设本次更新剩余所有消息。*/
        msg_size = std_data->std_msg_size - already_inputed_size;
        ret = ehsm_rsa_sign_update(ctx, &msg[already_inputed_size], msg_size);
        ret = demo_check_val("The 2nd execution of update API:", EHSM_OK, ret);
        already_inputed_size += msg_size;
    }

    if (EHSM_OK == ret) {
        if (gen_sig) { /* 签名生成 */
            /**
             * 4. 调用 finish API。需要输入所有消息，API 完成计算后会将签名返回给 SoC。
             *      - 首先作为输入，指定 signature buffer 的长度，必须大于等于模长；
             *      - 再作为输出，eHSM 会回写实际产生的签名长度；
             */
            signature_size = std_data->std_signature_size;
            ret = ehsm_rsa_sign_finish_gen(ctx, signature, &signature_size, std_data->std_salt_size);
            ret = demo_check_val("The execution of RSA sign finish gen API:", EHSM_OK, ret);

            if (EHSM_OK == ret) {
                /*由于签名生成含有随机数，因此无法直接校输出的签名，采用先签名，后验签的方式校验 */
                print_hex("The generated signature is:\r\n    ", signature, signature_size);

                /* 这里仅仅为了验证 eHSM 生成的签名，因此采用 onepass API。 */
                ret = ehsm_rsa_sign_onepass_verify_with_plain_key(ctx, std_data->algo, (uint8_t *)rsa_key,
                    std_data->padding, msg, msg_size, signature, signature_size, std_data->std_salt_size,
                    &verify_result);
                ret = demo_check_val(
                    "The execution of RSA verify onepass with plain key API for verifing generated signature:", EHSM_OK,
                    ret);
            }
        } else { /* 签名验证 */
            /* 4. 调用 finish API。需要输入所有消息，API 完成计算后会将验证结果返回给 SoC。*/
            signature_size = std_data->std_signature_size;
            ret = ehsm_rsa_sign_finish_verify(ctx, signature, signature_size, std_data->std_salt_size, &verify_result);
            ret = demo_check_val("The execution of RSA sign finish verify API:", EHSM_OK, ret);
            if (EHSM_OK == ret) {
                /** eHSM FW 返回成功，检查签名验证结果 */
                ret = demo_check_val("RSA signature verification:", true, verify_result);
            }
        }
    }

    ret_str = (EHSM_OK == ret) ? "SUCCESS" : "FAILURE";
    ehsm_port_printf("[RSA sign stepwise with plain key ends with %s !!!] \r\n\r\n", ret_str);
}

/**
 * @brief RSA 加解密的演示入口函数。
 */
static void rsa_cipher_entry(void)
{
    uint32_t i;
    const demo_rsa_cipher_std_st *std_data_arr = s_rsa_cipher_std_data_arr;
    uint32_t cnt = sizeof(s_rsa_cipher_std_data_arr) / sizeof(demo_rsa_cipher_std_st);

    ehsm_port_printf("\r\n\r\n==================== eHSM demo for RSA cipher starts. ==================== "
                     "\r\n\r\n");

    /* TODO：增加其他 RSA 长度 */
    for (i = 0U; i < cnt; i++) {
        /* 解密，先验证 RSA 的 API 功能是否正常 */
        ehsm_port_printf("[%s decryption for onepass with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        rsa_cipher(false, &std_data_arr[i]);

        /* 加密，采用先加密，后解密，最后比对明文的方式 */
        ehsm_port_printf("[%s encryption for onepass with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        rsa_cipher(true, &std_data_arr[i]);

        /* 解密，使用明文密钥，先验证 RSA 的 API 功能是否正常 */
        ehsm_port_printf(
            "[%s decryption with plain key for onepass with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        rsa_cipher_with_plain_key(false, &std_data_arr[i]);

        /* 加密，使用明文密钥，采用先加密，后解密，最后比对明文的方式 */
        ehsm_port_printf(
            "[%s encryption with plain key for onepass with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        rsa_cipher_with_plain_key(true, &std_data_arr[i]);
    }

    ehsm_port_printf("\r\n==================== eHSM demo for RSA cipher ends. ==================== \r\n\r\n");
}

/**
 * @brief RSA 签名生成和校验的演示入口函数。
 */
static void rsa_sign_ver_entry(void)
{
    uint32_t i;
    const demo_rsa_sign_std_st *std_data_arr = s_rsa_sign_std_data_arr;
    uint32_t cnt = sizeof(s_rsa_sign_std_data_arr) / sizeof(demo_rsa_sign_std_st);

    ehsm_port_printf("\r\n\r\n==================== eHSM demo for RSA signature starts. ==================== "
                     "\r\n\r\n");

    /* TODO：增加其他 RSA 长度和 hash 算法 */
    for (i = 0U; i < cnt; i++) {
        /* 一次性签名验证，先验证 RSA 签名 API 的功能是否正常 */
        ehsm_port_printf("[%s verification for onepass with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        rsa_sign_ver_onepass(false, &std_data_arr[i]);

        /* 一次性签名生成，采用先生成签名，后验证签名的方式 */
        ehsm_port_printf("[%s generation for onepass with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        rsa_sign_ver_onepass(true, &std_data_arr[i]);

        /* 分段签名验证，先验证 RSA 签名 API 的功能是否正常 */
        ehsm_port_printf("[%s verification for stepwise with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        rsa_sign_ver_stepwise(false, &std_data_arr[i]);

        /* 分段签名生成，采用先生成签名，后验证签名的方式 */
        ehsm_port_printf("[%s generation for stepwise with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        rsa_sign_ver_stepwise(true, &std_data_arr[i]);

        /* 一次性签名验证，使用明文密钥，先验证 RSA 签名 API 的功能是否正常 */
        ehsm_port_printf(
            "[%s verification with plain key for onepass with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        rsa_sign_ver_onepass_with_plain_key(false, &std_data_arr[i]);

        /* 一次性签名生成，使用明文密钥，采用先生成签名，后验证签名的方式 */
        ehsm_port_printf(
            "[%s generation with plain key for onepass with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        rsa_sign_ver_onepass_with_plain_key(true, &std_data_arr[i]);

        /* 分段签名验证，使用明文密钥，先验证 RSA 签名 API 的功能是否正常 */
        ehsm_port_printf(
            "[%s verification with plain key for stepwise with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        rsa_sign_ver_stepwise_with_plain_key(false, &std_data_arr[i]);

        /* 分段签名生成，使用明文密钥，采用先生成签名，后验证签名的方式 */
        ehsm_port_printf(
            "[%s generation with plain key for stepwise with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        rsa_sign_ver_stepwise_with_plain_key(true, &std_data_arr[i]);
    }

    ehsm_port_printf("\r\n==================== eHSM demo for RSA signature ends. ==================== \r\n\r\n");
}

/**
 * @brief RSA 算法演示入口函数。
 */
void ehsm_demo_rsa_entry(void)
{
    /* RSA 加解密示例入口函数。 */
    rsa_cipher_entry();

    /* RSA 签名生成和校验示例入口函数。 */
    rsa_sign_ver_entry();
}

