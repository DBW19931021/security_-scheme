#include "lib_extension.h"
#include "cpu_porting.h"
#include "mmap.h"
#include "common_config.h"

#define __weak __attribute__((weak))

#ifdef CONFIG_UNIT_TEST
static uint32_t g_cfg_reg[0x02];
#define SYS_DMA_CFG_REG ((volatile uint32_t *)g_cfg_reg)
#else
#define SYS_DMA_CFG_REG (CRYPTO_BASE_ADDR + 0x30006000U)
#endif

#define AXI_READ_AHB_WRITE_VAL   (0x0U)
#define DMA_CTR_BIT              (1U)
#define AXI_READ_AHB_WRITE_MASK  (0x1U << DMA_CTR_BIT)
#define KMU_TRANSP_KEY_TO_SKE    (0x00U)
#define KMU_TRANSP_KEY_TO_HASH   (0x01U)
#define KMU_TRANSP_KEY_TO_CHACHA (0x02U)
#define KMU_TRANSP_KEY_TO_PKE    (0x03U)

/* function: get ahb dma cfg reg
 * parameters:
 * return:
 * caution:
 */
static inline volatile uint32_t *get_ahb_dma_cfg_reg(void)
{
    return (volatile uint32_t *)SYS_DMA_CFG_REG;
}

/* function: whether is ahb dma write
 * parameters:
 * return:
 * caution:
 */
static inline uint32_t is_ahb_dma_write(void)
{
    uint32_t ret;
    const volatile uint32_t *dma_cfg_reg = get_ahb_dma_cfg_reg();
    uint32_t cfg_val = *dma_cfg_reg;
    if ((cfg_val & AXI_READ_AHB_WRITE_MASK) == AXI_READ_AHB_WRITE_VAL) {
        ret = 1U;
    } else {
        ret = 0U;
    }
    return ret;
}

/* function: address remap
 * parameters:
 *     addr_h ------------------- input, address high 32bit
 *     addr_l ------------------- input, address low 32bit
 *     is_dma_read_addr --------- input, whether the addr is dma input or output, zero is output, othewise is input
 * return:
 * caution:
 */
__weak uint8_t *lib_addr_arch32_lock_remap(uint32_t addr_h, uint32_t addr_l, uint32_t is_dma_read_addr)
{
    uint8_t *addr;

    if (0U != is_ahb_dma_write()) {
        addr = (uint8_t *)(uintptr_t)addr_l;
    } else {
        addr = mmap_remap_addr_u32(addr_h, addr_l);
    }
    return addr;
}

/* function: address unremap
 * parameters:
 *     addr_h ------------------- input, address high 32bit
 *     addr_l ------------------- input, address low 32bit
 * return:
 * caution:
 */
__weak void lib_addr_arch32_unlock_remap(void)
{
    return;
}

/* function: lib ske config secure port
 * parameters:
 *     alg        ------------------- input, ske alg
 *     sp_key_idx ------------------- input, secure port key id
 * return:
 * caution:
 */
__weak uint32_t lib_ske_secure_port_config(uint32_t alg, uint16_t sp_key_idx)
{
    return 0U;
}

/* function: lib hash config secure port
 * parameters:
 *     sp_key_idx ------------------- input, secure port key id
 * return:
 * caution:
 */
__weak uint32_t lib_hash_secure_port_config(uint16_t sp_key_idx)
{
    return 0U;
}
