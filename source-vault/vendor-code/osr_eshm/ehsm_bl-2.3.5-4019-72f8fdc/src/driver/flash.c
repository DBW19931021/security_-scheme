/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "flash.h"
#include "util.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define FLASH_BASE_ADDR  (0x33000000U)
#define FLASH_CHIP_SIZE  (0x400U)
#define FLASH_BLOCK_SIZE (0x400U)
#define FLASH_WRITE_UNIT (0x4U)
#define FLASH_INIT_VALUE (0xFFU)

/***********************************************************************************************************************
 *  EXTERN
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL TYPEDEFS
 **********************************************************************************************************************/

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

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t flash_init(void)
{
    return EHSM_ERR_SW_SUCCESS;
}

uint32_t flash_read(uint32_t addr, uint8_t *data, uint32_t size)
{
    uint32_t ret;

    if ((data == NULL) || (size == 0U)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if ((addr < FLASH_BASE_ADDR) || (addr > (FLASH_BASE_ADDR + FLASH_CHIP_SIZE))
        || (size > (FLASH_BASE_ADDR + FLASH_CHIP_SIZE - addr))) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        (void)util_memcpy(data, (uint8_t *)addr, size);
        ret = EHSM_ERR_SW_SUCCESS;
    }

    return ret;
}

uint32_t flash_write(uint32_t addr, const uint8_t *data, uint32_t size)
{
    uint32_t ret;

    if ((data == NULL) || (size == 0U)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if ((addr < FLASH_BASE_ADDR) || (addr > (FLASH_BASE_ADDR + FLASH_CHIP_SIZE))
        || (size > (FLASH_BASE_ADDR + FLASH_CHIP_SIZE - addr))) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if (((addr % FLASH_WRITE_UNIT) != 0U) || ((size % FLASH_WRITE_UNIT) != 0U)) {
        ret = EHSM_ERR_NOT_ALIGNED;
    } else {
        (void)util_memcpy((uint8_t *)addr, data, size);
        ret = EHSM_ERR_SW_SUCCESS;
    }

    return ret;
}

uint32_t flash_erase(uint32_t addr, uint32_t size)
{
    uint32_t ret;

    if (size == 0U) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if ((addr < FLASH_BASE_ADDR) || (addr > (FLASH_BASE_ADDR + FLASH_CHIP_SIZE))
        || (size > (FLASH_BASE_ADDR + FLASH_CHIP_SIZE - addr))) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if (((addr % FLASH_BLOCK_SIZE) != 0U) || ((size % FLASH_BLOCK_SIZE) != 0U)) {
        ret = EHSM_ERR_NOT_ALIGNED;
    } else {
        (void)util_memset((uint8_t *)addr, FLASH_INIT_VALUE, size);
        ret = EHSM_ERR_SW_SUCCESS;
    }

    return ret;
}

uint32_t flash_verify(uint32_t addr, uint32_t size)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t i = 0;
    uint8_t blank[FLASH_WRITE_UNIT];

    if (size == 0U) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if ((addr < FLASH_BASE_ADDR) || (addr > (FLASH_BASE_ADDR + FLASH_CHIP_SIZE))
        || (size > (FLASH_BASE_ADDR + FLASH_CHIP_SIZE - addr))) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if (((addr % FLASH_WRITE_UNIT) != 0U) || ((size % FLASH_WRITE_UNIT) != 0U)) {
        ret = EHSM_ERR_NOT_ALIGNED;
    } else {
        (void)util_memset(blank, FLASH_INIT_VALUE, FLASH_WRITE_UNIT);

        for (i = 0; i < size; i += FLASH_WRITE_UNIT) {
            if (util_memcmp(blank, (uint8_t *)(addr + i), FLASH_WRITE_UNIT) != 0) {
                ret = EHSM_ERR_DATA_NOT_EMPTY;
                break;
            } else {
                // nothing to do
            }
        }
    }

    return ret;
}
