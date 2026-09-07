#ifndef PKE_COMMON_H
#define PKE_COMMON_H



#include "../crypto_include/pke_config.h"





//ECC point conversion form
#define POINT_COMPRESSED          (0x02U)   //pc||x, pc = 0x02|LSB(y)
#define POINT_UNCOMPRESSED        (0x04U)   //pc||x||y, pc=0x04
typedef uint8_t EC_POINT_FORM;



//define KDF
typedef void *(*KDF_FUNC)(const void *input, uint32_t byteLen, uint8_t *key, uint32_t keyByteLen);


//APIs

void pke_load_operand(uint32_t *baseaddr, const uint32_t *data, uint32_t wordLen);

void pke_load_operand_256bits(uint32_t *baseaddr, const uint32_t *data);

void pke_read_operand(const uint32_t *baseaddr, uint32_t *data, uint32_t wordLen);

void pke_read_operand_256bits(const uint32_t *baseaddr, uint32_t *data);

void pke_load_operand_U8(uint32_t *baseaddr, const uint8_t *data, uint32_t byteLen);

void pke_read_operand_U8(const uint32_t *baseaddr, uint8_t *data, uint32_t byteLen);

void pke_set_operand_uint32_value(uint32_t *baseaddr, uint32_t wordLen, uint32_t b);

void pke_set_operand_uint32_value_256bits(uint32_t *baseaddr, uint32_t b);

uint32_t is_k_equal_to_n_minus_1(const uint32_t *k, const uint32_t *n, uint32_t words);

#endif

