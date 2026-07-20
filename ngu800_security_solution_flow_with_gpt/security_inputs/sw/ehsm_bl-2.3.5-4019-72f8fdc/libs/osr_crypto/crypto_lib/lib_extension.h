#ifndef LIB_EXTENSION_H
#define LIB_EXTENSION_H

#include "../crypto_include/common_config.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(CONFIG_EHSM_ENABLE_FUSA_ASIL_B) && CONFIG_EHSM_ENABLE_FUSA_ASIL_B
#define CONFIG_EHSM_SUPPORT_REGISTER_LOCK
#endif

uint32_t lib_ske_secure_port_config(uint32_t alg, uint16_t sp_key_idx);

uint32_t lib_hash_secure_port_config(uint16_t sp_key_idx);

uint8_t *lib_addr_arch32_lock_remap(uint32_t addr_h, uint32_t addr_l, uint32_t is_dma_read_addr);

void lib_addr_arch32_unlock_remap(void);


#ifdef CONFIG_EHSM_SUPPORT_REGISTER_LOCK
#define LIB_REGISTER_LOCK_VALUE    (0x00U)
#define LIB_REGISTER_UNLOCK_VALUE  (0xa55a5aa5U)
#define LIB_REGISTER_OFFSET        (0x80000U)

#ifdef CONFIG_UNIT_TEST
uint32_t ip_lock_register[1];
#endif

/* function: lib lock register
 * parameters:
 *     ip_base_addr ------------------- input, ip base address
 * return:
 * caution:
 */
static inline void lib_register_lock(volatile const void *ip_base_addr)
{
#ifdef CONFIG_UNIT_TEST
    ip_lock_register[0] = LIB_REGISTER_LOCK_VALUE;
#else
    *((volatile uint32_t *)((uintptr_t)ip_base_addr+LIB_REGISTER_OFFSET)) = LIB_REGISTER_LOCK_VALUE;
#endif
}

/* function: lib unlock register
 * parameters:
 *     ip_base_addr ------------------- input, ip base address
 * return:
 * caution:
 */
static inline void lib_register_unlock(volatile const void *ip_base_addr)
{
#ifdef CONFIG_UNIT_TEST
    ip_lock_register[0] = LIB_REGISTER_UNLOCK_VALUE;
#else
    *((volatile uint32_t *)((uintptr_t)ip_base_addr+LIB_REGISTER_OFFSET)) = LIB_REGISTER_UNLOCK_VALUE;
#endif
}
#else
static inline void lib_register_lock(volatile const void *ip_base_addr)
{
    (void)ip_base_addr;
}

static inline void lib_register_unlock(volatile const void *ip_base_addr)
{
    (void)ip_base_addr;
}
#endif


#ifdef __cplusplus
}
#endif

#endif 
