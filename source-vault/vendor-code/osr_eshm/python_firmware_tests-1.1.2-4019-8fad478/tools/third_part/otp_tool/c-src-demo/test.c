#include <stdio.h>
#include <string.h>
#include "otptool_clib.h"

const char* layout_toml = "[config]\n"
                          "\n"
                          "default_one = false\n"
                          "otp_size = 24\n"
                          "\n"
                          "[[bytes]]\n"
                          "name = \"lifecycle\"\n"
                          "offset = 0\n"
                          "length = 4\n"
                          "choices = { test = \"00000000\", dev = \"14848142\", manu = \"16a4c146\", user = "
                          "\"96ecc157\", debug = \"d7c5fcde\", destory = \"ffffffff\" }\n"
                          "\n"
                          "[[bytes]]\n"
                          "name = \"uid\"\n"
                          "offset = 4\n"
                          "length = 16\n"
                          "add_crc32 = true\n";

const char* values_toml = "lifecycle = \"dev\"\n"
                          "uid = \"00000000000000000000000000000001\"\n";

const uint8_t otp_data_expect[24] = { 0x14, 0x84, 0x81, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x3f, 0xec, 0x51 };

int main()
{
    uint8_t otp_data_buf[1024];
    uint32_t otp_data_len = 1024;

    char parse_txt_buf[2048];
    char err_msg_buf[1024];

    uint32_t ret = gen_otp_data_from_toml(layout_toml, values_toml, NULL, otp_data_buf, &otp_data_len, parse_txt_buf,
        sizeof(parse_txt_buf), err_msg_buf, sizeof(err_msg_buf));
    if (ret != 0) {
        printf("gen_otp_data_from_toml failed, error: %s\n", err_msg_buf);
        return 1;
    }
    if (otp_data_len != sizeof(otp_data_expect) || memcmp(otp_data_buf, otp_data_expect, otp_data_len) != 0) {
        printf("generated OTP data not match expected!\n");
        return 1;
    }
    printf("parse text: %s\n", parse_txt_buf);

    ret = gen_otp_data_from_toml(
        layout_toml, values_toml, NULL, otp_data_buf, &otp_data_len, parse_txt_buf, 2, err_msg_buf, sizeof(err_msg_buf));
    if (ret != 0) {
        printf("gen_otp_data_from_toml failed, error: %s\n", err_msg_buf);
        return 1;
    }
    if (strlen(parse_txt_buf) != 1) {
        printf("parse_txt_buf length error!\n");
        return 1;
    }

    ret = gen_otp_data_from_toml(layout_toml, values_toml, NULL, otp_data_buf, &otp_data_len, NULL, 0, NULL, 0);
    if (ret != 0) {
        printf("gen_otp_data_from_toml failed with NULL prase_txt_buf and err_msg_buf, error: %s\n", err_msg_buf);
        return 1;
    }

    ret = gen_otp_data_from_toml(
        NULL, values_toml, NULL, otp_data_buf, &otp_data_len, NULL, 0, err_msg_buf, sizeof(err_msg_buf));
    if (ret == 0) {
        printf("gen_otp_data_from_toml should fail with NULL layout_toml\n");
        return 1;
    }
    printf("error: %s\n", err_msg_buf);

    ret = gen_otp_data_from_toml(
        layout_toml, NULL, NULL, otp_data_buf, &otp_data_len, NULL, 0, err_msg_buf, sizeof(err_msg_buf));
    if (ret == 0) {
        printf("gen_otp_data_from_toml should fail with NULL values_toml\n");
        return 1;
    }

    ret = gen_otp_data_from_toml(
        layout_toml, values_toml, NULL, NULL, &otp_data_len, NULL, 0, err_msg_buf, sizeof(err_msg_buf));
    if (ret == 0) {
        printf("gen_otp_data_from_toml should fail with NULL otp_data_buf\n");
        return 1;
    }

    ret = gen_otp_data_from_toml(
        layout_toml, values_toml, NULL, otp_data_buf, NULL, NULL, 0, err_msg_buf, sizeof(err_msg_buf));
    if (ret == 0) {
        printf("gen_otp_data_from_toml should fail with NULL otp_data_len\n");
        return 1;
    }

    ret = parse_otp_data(layout_toml, otp_data_expect, sizeof(otp_data_expect), parse_txt_buf, sizeof(parse_txt_buf),
        err_msg_buf, sizeof(err_msg_buf));
    if (ret != 0) {
        printf("parse_otp_data failed, error: %s\n", err_msg_buf);
        return 1;
    }
    printf("parse text: %s\n", parse_txt_buf);

    ret = parse_otp_data(NULL, otp_data_expect, sizeof(otp_data_expect), parse_txt_buf, sizeof(parse_txt_buf),
        err_msg_buf, sizeof(err_msg_buf));
    if (ret == 0) {
        printf("parse_otp_data should fail with NULL layout_toml\n");
        return 1;
    }
    printf("error: %s\n", err_msg_buf);
    ret = parse_otp_data(layout_toml, NULL, sizeof(otp_data_expect), parse_txt_buf, sizeof(parse_txt_buf), err_msg_buf,
        sizeof(err_msg_buf));
    if (ret == 0) {
        printf("parse_otp_data should fail with NULL otp_data\n");
        return 1;
    }
    printf("error: %s\n", err_msg_buf);
    ret = parse_otp_data(
        layout_toml, otp_data_expect, 1, parse_txt_buf, sizeof(parse_txt_buf), err_msg_buf, sizeof(err_msg_buf));
    if (ret == 0) {
        printf("parse_otp_data should fail with wrong otp_data_len\n");
        return 1;
    }
    printf("error: %s\n", err_msg_buf);
    ret = parse_otp_data(layout_toml, otp_data_expect, sizeof(otp_data_expect), NULL, sizeof(parse_txt_buf),
        err_msg_buf, sizeof(err_msg_buf));
    if (ret == 0) {
        printf("parse_otp_data should fail with NULL parse_txt_buf\n");
        return 1;
    }
    printf("error: %s\n", err_msg_buf);

    // ---- gen_c_array tests ----

    char c_array_buf[4096];
    ret = gen_c_array(otp_data_expect, sizeof(otp_data_expect), "g_otp_data", c_array_buf, sizeof(c_array_buf),
        err_msg_buf, sizeof(err_msg_buf));
    if (ret != 0) {
        printf("gen_c_array failed, error: %s\n", err_msg_buf);
        return 1;
    }
    // Verify output contains expected patterns
    if (strstr(c_array_buf, "const unsigned char g_otp_data[24]") == NULL) {
        printf("gen_c_array output missing array declaration\n");
        return 1;
    }
    if (strstr(c_array_buf, "// clang-format off") == NULL) {
        printf("gen_c_array output missing clang-format off\n");
        return 1;
    }
    if (strstr(c_array_buf, "/* 0000 */") == NULL) {
        printf("gen_c_array output missing offset comment\n");
        return 1;
    }
    printf("c_array output:\n%s\n", c_array_buf);

    // gen_c_array with NULL otp_data should fail
    ret = gen_c_array(NULL, sizeof(otp_data_expect), "g_otp_data", c_array_buf, sizeof(c_array_buf),
        err_msg_buf, sizeof(err_msg_buf));
    if (ret == 0) {
        printf("gen_c_array should fail with NULL otp_data\n");
        return 1;
    }
    printf("error: %s\n", err_msg_buf);

    // gen_c_array with NULL var_name should fail
    ret = gen_c_array(otp_data_expect, sizeof(otp_data_expect), NULL, c_array_buf, sizeof(c_array_buf),
        err_msg_buf, sizeof(err_msg_buf));
    if (ret == 0) {
        printf("gen_c_array should fail with NULL var_name\n");
        return 1;
    }
    printf("error: %s\n", err_msg_buf);

    // gen_c_array with NULL c_array_buf should fail
    ret = gen_c_array(otp_data_expect, sizeof(otp_data_expect), "g_otp_data", NULL, 0,
        err_msg_buf, sizeof(err_msg_buf));
    if (ret == 0) {
        printf("gen_c_array should fail with NULL c_array_buf\n");
        return 1;
    }
    printf("error: %s\n", err_msg_buf);

    // gen_c_array with too small buffer should fail
    char tiny_buf[8];
    ret = gen_c_array(otp_data_expect, sizeof(otp_data_expect), "g_otp_data", tiny_buf, sizeof(tiny_buf),
        err_msg_buf, sizeof(err_msg_buf));
    if (ret == 0) {
        printf("gen_c_array should fail with too small c_array_buf\n");
        return 1;
    }
    printf("error: %s\n", err_msg_buf);

    // gen_c_array with empty data should fail
    uint8_t empty_data[1];
    ret = gen_c_array(empty_data, 0, "g_empty", c_array_buf, sizeof(c_array_buf),
        err_msg_buf, sizeof(err_msg_buf));
    if (ret == 0) {
        printf("gen_c_array should fail with empty data\n");
        return 1;
    }
    printf("error: %s\n", err_msg_buf);

    printf("great, all tests passed!\n");
    return 0;
}
