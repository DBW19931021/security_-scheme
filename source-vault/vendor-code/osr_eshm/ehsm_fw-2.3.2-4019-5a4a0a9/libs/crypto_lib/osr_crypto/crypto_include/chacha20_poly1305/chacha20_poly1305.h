#ifndef CHACHA20_POLY1305_H
#define CHACHA20_POLY1305_H


#include "../../crypto_hal/chacha20_poly1305_basic.h"


#ifdef __cplusplus
extern "C" {
#endif



/* User APIs */
uint32_t chacha20_poly1305_init(chacha20_poly1305_st *ctx, chacha20_poly1305_crypto_e crypto, const uint8_t *key, uint32_t constant, 
	const uint8_t iv[8], const uint8_t *aad , uint64_t aad_bytes);

uint32_t chacha20_poly1305_update_excluding_last_data(chacha20_poly1305_st *ctx, const uint8_t *payload_in, 
	uint8_t *payload_out, uint64_t payload_bytes);

uint32_t chacha20_poly1305_update_including_last_data(chacha20_poly1305_st *ctx, const uint8_t *payload_in, 
	uint8_t *payload_out, uint64_t payload_bytes, uint8_t tag[16]);

uint32_t chacha20_poly1305(chacha20_poly1305_crypto_e crypto, const uint8_t key[32], uint32_t constant, 
	const uint8_t iv[8], const uint8_t *aad , uint64_t aad_bytes, const uint8_t *payload_in, uint8_t *payload_out, uint64_t payload_bytes, uint8_t tag[16]);


#ifdef CHACHA20_POLY1305_DMA_FUNCTION
uint32_t chacha20_poly1305_dma_init(chacha20_poly1305_dma_st *ctx, chacha20_poly1305_crypto_e crypto, const uint8_t key[32], 
	uint32_t constant, const uint8_t iv[8], const uint32_t *aad , uint32_t aad_bytes);

uint32_t chacha20_poly1305_dma_update_excluding_last_data(chacha20_poly1305_dma_st *ctx, const uint32_t *payload_in, const uint32_t *payload_out, 
	uint32_t payload_bytes);

uint32_t chacha20_poly1305_dma_update_including_last_data(chacha20_poly1305_dma_st *ctx, const uint32_t *payload_in,
	const uint32_t *payload_out, uint32_t payload_bytes, uint8_t tag[16]);

uint32_t chacha20_poly1305_dma(chacha20_poly1305_crypto_e crypto, const uint8_t key[32], uint32_t constant, 
	const uint8_t iv[8], const uint32_t *aad , uint32_t aad_bytes, const uint32_t *payload_in, const uint32_t *payload_out, uint32_t payload_bytes, uint8_t tag[16]);


#endif



#ifdef __cplusplus
}
#endif

#endif

