

#ifndef OTPTOOL_CLIB_H
#define OTPTOOL_CLIB_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * @brief Generates OTP data from TOML layout and values.
 *
 * @param[in] layout_toml - A C string containing the TOML layout.
 * @param[in] values_toml - A C string containing the TOML values.
 * @param[in] base_dir - A C string containing the base directory for resolving relative key_file paths, can be NULL.
 * @param[out] otp_data_buf - A pointer to a uint8_t buffer that will be filled with the generated OTP data.
 * @param[in,out] otp_data_len - A pointer to a uint32_t that filled with buffer length, and will be filled with the length of OTP data, cannot be NULL.
 * @param[out] parse_txt_buf - A pointer to a char buffer that will be filled with the parse text string, can be NULL.
 * @param[in] parse_txt_len - A pointer to a char that filled with buffer length.
 * @param[out] err_msg_buf - A pointer to a char buffer that will be filled with the error message if processing failed, can be NULL.
 * @param[in] err_msg_len - The length of err_msg_buf.
 *
 * @return 0: OK, other: error
 */
uint32_t gen_otp_data_from_toml(const char *layout_toml,
                                const char *values_toml,
                                const char *base_dir,
                                uint8_t *otp_data_buf,
                                uint32_t *otp_data_len,
                                char *parse_txt_buf,
                                uint32_t parse_txt_len,
                                char *err_msg_buf,
                                uint32_t err_msg_len);

/**
 * @brief Parses OTP data using a TOML layout.
 *
 * @param[in] layout_toml - A C string containing the TOML layout.
 * @param[in] otp_data - A pointer to a uint8_t buffer that will be filled with the generated OTP data.
 * @param[in] otp_data_len - The length of OTP data.
 * @param[out] parse_txt_buf - A pointer to a char buffer that will be filled with the parse text string, can be NULL.
 * @param[in] parse_txt_len - A pointer to a char that filled with buffer length.
 * @param[out] err_msg_buf - A pointer to a char buffer that will be filled with the error message if processing failed, can be NULL.
 * @param[in] err_msg_len - The length of err_msg_buf.
 *
 * @return 0: OK, other: error
 */
uint32_t parse_otp_data(const char *layout_toml,
                        const uint8_t *otp_data,
                        uintptr_t otp_data_len,
                        char *parse_txt_buf,
                        uint32_t parse_txt_len,
                        char *err_msg_buf,
                        uint32_t err_msg_len);

/**
 * @brief Generates a C array source string from OTP binary data.
 *
 * @param[in] otp_data - A pointer to the OTP binary data.
 * @param[in] otp_data_len - The length of OTP data.
 * @param[in] var_name - A C string containing the variable name (e.g. "g_otp_data").
 * @param[out] c_array_buf - A pointer to a char buffer that will be filled with the C array source string.
 * @param[in] c_array_len - The length of c_array_buf.
 * @param[out] err_msg_buf - A pointer to a char buffer that will be filled with the error message if processing failed, can be NULL.
 * @param[in] err_msg_len - The length of err_msg_buf.
 *
 * @return 0: OK, other: error
 */
uint32_t gen_c_array(const uint8_t *otp_data,
                     uint32_t otp_data_len,
                     const char *var_name,
                     char *c_array_buf,
                     uint32_t c_array_len,
                     char *err_msg_buf,
                     uint32_t err_msg_len);

#endif  /* OTPTOOL_CLIB_H */
