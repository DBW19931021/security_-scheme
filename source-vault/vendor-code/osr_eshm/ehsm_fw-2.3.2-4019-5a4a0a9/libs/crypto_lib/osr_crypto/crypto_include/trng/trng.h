#ifndef TRNG_H
#define TRNG_H



#include "../../crypto_hal/trng_basic.h"



#ifdef __cplusplus
extern "C" {
#endif


uint32_t trng_set_global_init_config(uint32_t config_choice);

uint32_t get_rand_internal(uint8_t *random, uint32_t bytes);

uint32_t get_rand_fast(uint8_t *random, uint32_t bytes);

#ifndef CONFIG_TRNG_GENERATE_BY_HARDWARE
uint32_t get_rand_register(void);
#endif

uint32_t get_rand(uint8_t *random, uint32_t bytes);

uint32_t get_trng_rand(uint8_t *random, uint32_t bytes);

#ifdef __cplusplus
}
#endif

#endif
