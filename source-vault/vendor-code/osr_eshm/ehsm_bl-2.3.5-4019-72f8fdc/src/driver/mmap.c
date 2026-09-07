/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "types.h"
#include "cpu_porting.h"
#include "mmap.h"
#include "component/util.h"
/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define MEMREMAP_BIT  (0x1U << 31)
#define MASK_BIT      (0x1U << 30)
#define MMAP_MAX_SIZE (0x40000000U) /* 1GB */
/***********************************************************************************************************************
 *  EXTERN
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL TYPEDEFS
 **********************************************************************************************************************/
typedef struct {
    uint32_t sys_soc_mem_ba_l;
    uint32_t sys_soc_mem_ba_h;
} mmap_reg_st;

/***********************************************************************************************************************
 *  LOCAL CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
#if defined(CONFIG_UNIT_TEST)
static mmap_reg_st ut_reg;

static inline volatile mmap_reg_st *get_mmap_reg_base(void)
{
    return &ut_reg;
}
#else
static inline volatile mmap_reg_st *get_mmap_reg_base(void)
{
    return (volatile mmap_reg_st *)(0x30003800U);
}
#endif

#define MEMREMAP_REG_BASE get_mmap_reg_base()

static inline uint32_t raddr_get_low(raddr_t val)
{
    return (uint32_t)val;
}

static inline uint32_t raddr_get_high(raddr_t val)
{
    return (uint32_t)(val >> 32);
}

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint8_t *mmap_remap_addr_u64(raddr_t raddr)
{
    return mmap_remap_addr_u32(raddr_get_high(raddr), raddr_get_low(raddr));
}

uint8_t *mmap_remap_addr_u32(uint32_t raddr_h, uint32_t raddr_l)
{
    uint8_t *remap_addr;
    if ((0U == raddr_h) && (0U == raddr_l)) {
        remap_addr = NULL;
    } else {
        MEMREMAP_REG_BASE->sys_soc_mem_ba_l = raddr_l;
        MEMREMAP_REG_BASE->sys_soc_mem_ba_h = raddr_h;

        if (MEMREMAP_REG_BASE->sys_soc_mem_ba_l != raddr_l || MEMREMAP_REG_BASE->sys_soc_mem_ba_h != raddr_h) {
            // Register could NOT be written
            remap_addr = NULL;
        } else {
            /*
             * The value of low address should be fetched from the sys_soc_mem_ba_l but not the variable,
             * so that the caller will clearly know whether the value has been properly writen to the register.
             */
            remap_addr = (uint8_t *)((raddr_l | MEMREMAP_BIT) & ~MASK_BIT);
        }
    }
    return remap_addr;
}

uint32_t mmap_write_remote_data(raddr_t raddr, const void *lbuf_ptr, uint32_t sz)
{
    uint32_t ret;
    uint32_t level;
    uint8_t *remap_addr;
    if ((0UL == raddr) || (NULL == lbuf_ptr) || (sz >= MMAP_MAX_SIZE)) {
        ret = EHSM_ERR_INVALID_ADDRESS;
    } else {
        level = cpu_enter_critical();
        remap_addr = mmap_remap_addr_u64(raddr);
        if (NULL != remap_addr) {
            util_memcpy(remap_addr, lbuf_ptr, sz);
            ret = EHSM_ERR_SW_SUCCESS;
        } else {
            ret = EHSM_ERR_REMAP_FAILED;
        }
        cpu_exit_critical(level);
    }
    return ret;
}

uint32_t mmap_read_remote_data(void *lbuf_ptr, raddr_t raddr, uint32_t sz)
{
    uint32_t ret;
    uint32_t level;
    const uint8_t *remap_addr;
    if ((0UL == raddr) || (NULL == lbuf_ptr) || (sz >= MMAP_MAX_SIZE)) {
        ret = EHSM_ERR_INVALID_ADDRESS;
    } else {
        level = cpu_enter_critical();
        remap_addr = mmap_remap_addr_u64(raddr);
        if (NULL != remap_addr) {
            util_memcpy(lbuf_ptr, remap_addr, sz);
            ret = EHSM_ERR_SW_SUCCESS;
        } else {
            ret = EHSM_ERR_REMAP_FAILED;
        }
        cpu_exit_critical(level);
    }
    return ret;
}
