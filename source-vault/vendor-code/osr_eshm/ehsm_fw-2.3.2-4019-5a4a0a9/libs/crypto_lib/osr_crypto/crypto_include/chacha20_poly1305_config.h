#ifndef CHACHA20_POLY1305_CONFIG_H
#define CHACHA20_POLY1305_CONFIG_H


#include "common_config.h"






/************************************************************************************************
 ********************************    chacha20_poly1305 config    ********************************
 ************************************************************************************************/


/*
 *function: chacha20_poly1305 register base address
 *caution:
 */
#define CHACHA20_POLY1305_BASE_ADDR			(CRYPTO_BASE_ADDR + 0x31B00000UL)


/*
 *function: chacha20_poly1305 support ske dma function
 *caution:
 */
#define CHACHA20_POLY1305_DMA_FUNCTION


/*
 *function: support multiple thread function
 *caution:
 */
//open the macro supoort the interleaved computation
#define CONFIG_CHACHA20_POLY1305_SUPPORT_MUL_THREAD

#endif
