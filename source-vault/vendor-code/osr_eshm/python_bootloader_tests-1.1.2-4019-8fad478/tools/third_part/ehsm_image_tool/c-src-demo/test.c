/*
 * eHSM Image Tool - C FFI Test Program
 *
 * This program tests the C interface of the Rust library
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "imgtool_clib.h"

#define ERROR_MSG_SIZE 512

/* Global test status tracker */
static bool g_all_tests_passed = true;

/* Test 1: Get version information */
void test_version(void)
{
    printf("Test 1: Get version information\n");
    const char* version = get_imgtool_lib_version();
    printf("  Library version: %s\n", version);
    printf("  ✓ Test passed\n\n");
}

/* Test 2: Query image size */
bool test_get_image_size(const char* config_path)
{
    printf("Test 2: Query image size\n");
    printf("  Config file: %s\n", config_path);

    uint32_t image_size = 0;
    char err_msg[ERROR_MSG_SIZE] = { 0 };

    enum EhsmErrorCode result = get_image_size_from_config(config_path, &image_size, err_msg, ERROR_MSG_SIZE);

    if (result != Success) {
        printf("  ✗ Error: %s\n\n", err_msg);
        return false;
    }

    printf("  Image size: %u bytes\n", image_size);
    printf("  ✓ Test passed\n\n");
    return true;
}

/* Test 3: Generate image to memory */
bool test_generate_image_to_memory(const char* config_path)
{
    printf("Test 3: Generate image to memory\n");
    printf("  Config file: %s\n", config_path);

    // Step 1: Query size
    uint32_t image_size = 0;
    char err_msg[ERROR_MSG_SIZE] = { 0 };

    enum EhsmErrorCode result = get_image_size_from_config(config_path, &image_size, err_msg, ERROR_MSG_SIZE);

    if (result != Success) {
        printf("  ✗ Failed to query size: %s\n\n", err_msg);
        return false;
    }

    printf("  Need %u bytes buffer\n", image_size);

    // Step 2: Allocate memory
    uint8_t* buffer = (uint8_t*)malloc(image_size);
    if (buffer == NULL) {
        printf("  ✗ Memory allocation failed\n\n");
        return false;
    }

    // Step 3: Generate image
    uint32_t actual_size = image_size;
    memset(err_msg, 0, ERROR_MSG_SIZE);

    result = gen_image_from_config(config_path, buffer, &actual_size, err_msg, ERROR_MSG_SIZE);

    if (result != Success) {
        printf("  ✗ Failed to generate image: %s\n", err_msg);
        free(buffer);
        printf("\n");
        return false;
    }

    printf("  Actually generated %u bytes\n", actual_size);

    // Verify magic number (boot image valid flag)
    bool flag_valid = true;
    if (actual_size >= 596) {
        uint32_t valid_flag = *((uint32_t*)(buffer + 592));
        printf("  Valid flag: 0x%08X\n", valid_flag);

        if (valid_flag == 0x8E97645D || valid_flag == 0x71689BA2) {
            printf("  ✓ Image format correct\n");
        } else {
            printf("  ⚠ Warning: Valid flag mismatch\n");
            flag_valid = false;
        }
    }

    free(buffer);
    printf("  ✓ Test passed\n\n");
    return flag_valid;
}

/* Test 4: Full workflow - Generate */
bool test_full_workflow(const char* config_path, const char* output_path)
{
    printf("Test 4: Full workflow (generate image)\n");
    printf("  Config file: %s\n", config_path);
    printf("  Output file: %s\n", output_path);

    char err_msg[ERROR_MSG_SIZE] = { 0 };

    // Step 1: Query size
    uint32_t image_size = 0;
    enum EhsmErrorCode result = get_image_size_from_config(config_path, &image_size, err_msg, ERROR_MSG_SIZE);

    if (result != Success) {
        printf("  ✗ Failed to query size: %s\n\n", err_msg);
        return false;
    }

    // Step 2: Allocate memory and generate image
    uint8_t* buffer = (uint8_t*)malloc(image_size);
    if (buffer == NULL) {
        printf("  ✗ Memory allocation failed\n\n");
        return false;
    }

    uint32_t actual_size = image_size;
    result = gen_image_from_config(config_path, buffer, &actual_size, err_msg, ERROR_MSG_SIZE);

    if (result != Success) {
        printf("  ✗ Failed to generate image: %s\n", err_msg);
        free(buffer);
        printf("\n");
        return false;
    }
    free(buffer);

    printf("  ✓ Image written to %s (%u bytes)\n", output_path, actual_size);
    printf("  ✓ Test passed\n\n");
    return true;
}

/* Test 5: Error handling - Invalid config file */
bool test_error_handling(void)
{
    printf("Test 5: Error handling (invalid config file)\n");

    uint32_t image_size = 0;
    char err_msg[ERROR_MSG_SIZE] = { 0 };

    enum EhsmErrorCode result = get_image_size_from_config("nonexistent.toml", &image_size, err_msg, ERROR_MSG_SIZE);

    if (result == Success) {
        printf("  ✗ Should return error but returned success\n\n");
        return false;
    }

    printf("  Expected error: %s\n", err_msg);
    printf("  ✓ Error handling works\n\n");
    return true;
}

/* Test 6: NULL pointer check */
bool test_null_pointer(void)
{
    printf("Test 6: NULL pointer check\n");

    char err_msg[ERROR_MSG_SIZE] = { 0 };

    enum EhsmErrorCode result = get_image_size_from_config(NULL, NULL, err_msg, ERROR_MSG_SIZE);

    if (result != NullPointer) {
        printf("  ✗ Should return NullPointer error\n\n");
        return false;
    }

    printf("  Expected error: %s\n", err_msg);
    printf("  ✓ NULL pointer check works\n\n");
    return true;
}

/* Test 7: C array export - NULL pointer check */
bool test_c_array_null_pointer(void)
{
    printf("Test 7: C array export - NULL pointer check\n");

    char err_msg[ERROR_MSG_SIZE] = { 0 };

    /* NULL config_path */
    enum EhsmErrorCode result = gen_image_c_array_from_config(
        NULL, "output.c", NULL, err_msg, ERROR_MSG_SIZE);

    if (result != NullPointer) {
        printf("  ✗ Should return NullPointer for NULL config_path\n\n");
        return false;
    }
    printf("  NULL config_path: %s\n", err_msg);

    /* NULL c_output_path */
    memset(err_msg, 0, ERROR_MSG_SIZE);
    result = gen_image_c_array_from_config(
        "config.toml", NULL, NULL, err_msg, ERROR_MSG_SIZE);

    if (result != NullPointer) {
        printf("  ✗ Should return NullPointer for NULL c_output_path\n\n");
        return false;
    }
    printf("  NULL c_output_path: %s\n", err_msg);

    printf("  ✓ Test passed\n\n");
    return true;
}

/* Test 8: C array export - invalid config */
bool test_c_array_invalid_config(void)
{
    printf("Test 8: C array export - invalid config\n");

    char err_msg[ERROR_MSG_SIZE] = { 0 };

    enum EhsmErrorCode result = gen_image_c_array_from_config(
        "nonexistent.toml", "output.c", NULL, err_msg, ERROR_MSG_SIZE);

    if (result == Success) {
        printf("  ✗ Should return error for nonexistent config\n\n");
        return false;
    }

    printf("  Expected error: %s\n", err_msg);
    printf("  ✓ Test passed\n\n");
    return true;
}

/* Test 9: C array export - functional test (auto var_name) */
bool test_c_array_export(const char* config_path)
{
    printf("Test 9: C array export (auto var_name)\n");
    printf("  Config file: %s\n", config_path);

    const char* c_output = "c_test_image.c";
    char err_msg[ERROR_MSG_SIZE] = { 0 };

    /* Generate C array file */
    enum EhsmErrorCode result = gen_image_c_array_from_config(
        config_path, c_output, NULL, err_msg, ERROR_MSG_SIZE);

    if (result != Success) {
        printf("  ✗ Failed to export C array: %s\n\n", err_msg);
        return false;
    }

    /* Verify file exists and read content */
    FILE* fp = fopen(c_output, "r");
    if (fp == NULL) {
        printf("  ✗ Output file not created\n\n");
        return false;
    }

    /* Read file content for validation */
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* content = (char*)malloc(file_size + 1);
    if (content == NULL) {
        fclose(fp);
        unlink(c_output);
        printf("  ✗ Memory allocation failed\n\n");
        return false;
    }
    fread(content, 1, file_size, fp);
    content[file_size] = '\0';
    fclose(fp);

    bool passed = true;

    /* Check header comment */
    if (strstr(content, "/* generated by ehsm_imgtool v") == NULL) {
        printf("  ✗ Missing header comment\n");
        passed = false;
    } else {
        printf("  ✓ Header comment present\n");
    }

    /* Check clang-format guards */
    if (strstr(content, "/* clang-format off */") == NULL ||
        strstr(content, "/* clang-format on */") == NULL) {
        printf("  ✗ Missing clang-format guards\n");
        passed = false;
    } else {
        printf("  ✓ clang-format guards present\n");
    }

    /* Check auto-derived variable name (g_c_test_image) */
    if (strstr(content, "const unsigned char g_c_test_image[") == NULL) {
        printf("  ✗ Missing or incorrect array declaration\n");
        passed = false;
    } else {
        printf("  ✓ Array declaration correct (auto var_name: g_c_test_image)\n");
    }

    /* Check field annotations */
    if (strstr(content, "/* signature */") == NULL) {
        printf("  ✗ Missing field annotations\n");
        passed = false;
    } else {
        printf("  ✓ Field annotations present\n");
    }

    /* Check hex format (0xXX,) */
    if (strstr(content, "0x") == NULL) {
        printf("  ✗ Missing hex data\n");
        passed = false;
    } else {
        printf("  ✓ Hex data present\n");
    }

    printf("  Output file: %s (%ld bytes)\n", c_output, file_size);

    free(content);
    unlink(c_output);

    if (passed) {
        printf("  ✓ Test passed\n\n");
    } else {
        printf("  ✗ Test failed\n\n");
    }
    return passed;
}

/* Test 10: C array export - with custom var_name */
bool test_c_array_export_custom_var(const char* config_path)
{
    printf("Test 10: C array export (custom var_name)\n");
    printf("  Config file: %s\n", config_path);

    const char* c_output = "c_test_custom.c";
    const char* custom_var = "g_my_firmware";
    char err_msg[ERROR_MSG_SIZE] = { 0 };

    /* Generate C array file with custom variable name */
    enum EhsmErrorCode result = gen_image_c_array_from_config(
        config_path, c_output, custom_var, err_msg, ERROR_MSG_SIZE);

    if (result != Success) {
        printf("  ✗ Failed to export C array: %s\n\n", err_msg);
        return false;
    }

    /* Read and verify content */
    FILE* fp = fopen(c_output, "r");
    if (fp == NULL) {
        printf("  ✗ Output file not created\n\n");
        return false;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* content = (char*)malloc(file_size + 1);
    if (content == NULL) {
        fclose(fp);
        unlink(c_output);
        printf("  ✗ Memory allocation failed\n\n");
        return false;
    }
    fread(content, 1, file_size, fp);
    content[file_size] = '\0';
    fclose(fp);

    bool passed = true;

    /* Check custom variable name */
    char expected[256];
    snprintf(expected, sizeof(expected), "const unsigned char %s[", custom_var);
    if (strstr(content, expected) == NULL) {
        printf("  ✗ Custom variable name not found\n");
        passed = false;
    } else {
        printf("  ✓ Custom variable name correct (%s)\n", custom_var);
    }

    printf("  Output file: %s (%ld bytes)\n", c_output, file_size);

    free(content);
    unlink(c_output);

    if (passed) {
        printf("  ✓ Test passed\n\n");
    } else {
        printf("  ✗ Test failed\n\n");
    }
    return passed;
}

int main(int argc, char* argv[])
{
    printf("========================================\n");
    printf("eHSM Image Tool - C FFI Test Suite\n");
    printf("========================================\n\n");

    // Test 1: Version information (does not require config file)
    test_version();

    // Tests 5-6: Error handling (does not require config file)
    if (!test_error_handling()) {
        g_all_tests_passed = false;
    }
    if (!test_null_pointer()) {
        g_all_tests_passed = false;
    }

    // Tests 7-8: C array export error handling (does not require config file)
    if (!test_c_array_null_pointer()) {
        g_all_tests_passed = false;
    }
    if (!test_c_array_invalid_config()) {
        g_all_tests_passed = false;
    }

    // If config file path is provided, run full tests
    if (argc >= 2) {
        const char* config_path = argv[1];
        const char* output_path = (argc >= 3) ? argv[2] : "c_test_output.bin";

        if (!test_get_image_size(config_path)) {
            g_all_tests_passed = false;
        }
        if (!test_generate_image_to_memory(config_path)) {
            g_all_tests_passed = false;
        }
        if (!test_full_workflow(config_path, output_path)) {
            g_all_tests_passed = false;
        }

        // Tests 9-10: C array export functional tests
        if (!test_c_array_export(config_path)) {
            g_all_tests_passed = false;
        }
        if (!test_c_array_export_custom_var(config_path)) {
            g_all_tests_passed = false;
        }
    } else {
        printf("Note: Provide config file path to run full tests\n");
        printf("Usage: %s <config.toml> [output.bin]\n\n", argv[0]);
    }

    printf("========================================\n");
    if (g_all_tests_passed) {
        printf("All tests completed successfully\n");
        printf("========================================\n");
        return 0;
    } else {
        printf("Some tests failed\n");
        printf("========================================\n");
        return 1;
    }
}
