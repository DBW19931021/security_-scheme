/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "kms.h"
#include "kds.h"
#include "util.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
/*Definition the un-used key handle*/
#define KDS_UNUSED_KEY_HANDLE                   (KMS_INVALID_KEY_ID)

/*The minimum size of key is AES128*/
#define AES128_KEY_SIZE_BYTES (0x10U)
#define KDS_KEY_DATA_MIN_SIZE (sizeof(kms_key_format_st) + AES128_KEY_SIZE_BYTES)
#define KDS_RAM_KEY_MAX_NUM   (CONFIG_EHSM_RAM_KEY_BUFFER_MAX_SIZE / KDS_KEY_DATA_MIN_SIZE)
/***********************************************************************************************************************
 *  EXTERN
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL TYPEDEFS
 **********************************************************************************************************************/
/*Definition the node adjust mode*/
typedef enum
{
    INCREASE_NODE_SIZE,
    DECREASE_NODE_SIZE
} kds_adjust_node_mode_e;

/*Definition the memory node struct*/
typedef struct
{
    uint16_t size;
    uint16_t mem_offset;
    uint32_t key_handle;
} kds_mem_node_t;

/*Definition the memory head struct*/
typedef struct
{
    uint8_t *mem_start;
    uint16_t mem_size;
    uint16_t used_size;
    uint16_t node_num;
    kds_mem_node_t node_table[KDS_RAM_KEY_MAX_NUM];
} kds_mem_head_t;
/***********************************************************************************************************************
 *  LOCAL CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL VARIABLES
 **********************************************************************************************************************/
static bool_t g_kds_init = false;
static kds_mem_head_t g_mem_head = { 0 };
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
static uint8_t *kds_get_node_addr(uint16_t offset)
{
    return ((uint8_t *)&g_mem_head.mem_start[offset]);
}

static uint32_t check_key_handle_exist(uint32_t key_handle, uint32_t *idx)
{
    uint32_t i = 0;
    bool_t bexit_loop = false;
    bool_t bfind_handle = false;
    uint32_t search_key_num = 0;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (idx == NULL)
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        ret = EHSM_ERR_SW_SUCCESS;
    }

    if (ret == EHSM_ERR_SW_SUCCESS)
    {
        for (i = 0U; i < KDS_RAM_KEY_MAX_NUM; i++)
        {
            if (g_mem_head.node_table[i].key_handle == key_handle)
            {
                *idx = i;
                bexit_loop = true;
                bfind_handle = true;
            }

            if (g_mem_head.node_table[i].key_handle != KDS_UNUSED_KEY_HANDLE)
            {
                search_key_num += 1U;
                if (search_key_num >= g_mem_head.node_num)
                {
                    bexit_loop = true;
                }
            }

            if (bexit_loop == true)
            {
                break;
            }
        }

        if (bfind_handle == false)
        {
            ret = EHSM_ERR_NOT_EXIST_KEY;
        }
    }

    return ret;
}

static uint32_t find_unused_key_handle_idx(uint32_t *idx)
{
    uint32_t i = 0;
    uint32_t ckid = 0;
    bool_t bfind = false;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (idx == NULL)
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        for (i = 0U; i < KDS_RAM_KEY_MAX_NUM; i++)
        {
            if (g_mem_head.node_table[i].key_handle == KDS_UNUSED_KEY_HANDLE)
            {
                ret = check_key_handle_exist((KMS_KEY_TYPE_EHSM + i), &ckid);
            }

            if (ret == EHSM_ERR_NOT_EXIST_KEY)
            {
                *idx = i;
                bfind = true;
                ret = EHSM_ERR_SW_SUCCESS;
                break;
            }
        }

        if (bfind == false)
        {
            ret = EHSM_ERR_NOT_EMPTY_KEY_HANDLE;
        }
    }

    return ret;
}

static uint32_t adjust_mem_node_size(uint32_t node_index, uint16_t adjust_size, kds_adjust_node_mode_e adjust_mode)
{
    uint32_t i = 0;
    uint16_t move_size = 0;
    uint16_t node_size = 0;
    uint16_t src_mem_offset = 0;
    uint16_t dst_mem_offset = 0;
    uint16_t node_mem_offset = 0;
    uint32_t err_code = EHSM_ERR_SW_SUCCESS;

    if ((node_index >= KDS_RAM_KEY_MAX_NUM) || (adjust_mode > DECREASE_NODE_SIZE))
	{
        err_code = EHSM_ERR_PARAM_ERROR;
    }
    else if (adjust_size == 0U)
    {
        err_code = EHSM_ERR_SW_SUCCESS;
    }
    else
    {
        node_size = g_mem_head.node_table[node_index].size;
        node_mem_offset = g_mem_head.node_table[node_index].mem_offset;

        if ((adjust_mode == INCREASE_NODE_SIZE) && ((g_mem_head.mem_size - g_mem_head.used_size) < adjust_size))
        {
            err_code = EHSM_ERR_NOT_FREE_SPACE;
        }
        else if ((adjust_mode == DECREASE_NODE_SIZE) && (adjust_size > node_size))
        {
            err_code = EHSM_ERR_NOT_FREE_SPACE;
        }
        else
        {
            //nothing to do
        }
    }

    if ((err_code == EHSM_ERR_SW_SUCCESS) && (adjust_size != 0U))
    {
        //calculate the source offset
        src_mem_offset = node_mem_offset + node_size;
        //calculate the destination offset according to adjust mode
        if (adjust_mode == INCREASE_NODE_SIZE)
        {
            dst_mem_offset = src_mem_offset + adjust_size;
        }
        else
        {
            dst_mem_offset = src_mem_offset - adjust_size;
        }

        move_size = g_mem_head.used_size - src_mem_offset;
        //move all key data from source to destination address
        (void)kds_util_memmove(kds_get_node_addr(dst_mem_offset), kds_get_node_addr(src_mem_offset), move_size);

        for (i = 0; i < KDS_RAM_KEY_MAX_NUM; i++)
        {
            //If node record storage memory offset bigger than source memory adjust the each node table mem_offset
            if (g_mem_head.node_table[i].mem_offset >= src_mem_offset)
            {
                node_mem_offset = g_mem_head.node_table[i].mem_offset;
                g_mem_head.node_table[i].mem_offset = (adjust_mode == INCREASE_NODE_SIZE) ? (node_mem_offset + adjust_size) : (node_mem_offset - adjust_size);
            }
        }

        //Update the memory head total size and current node size according to the adjust mode 
        if (adjust_mode == INCREASE_NODE_SIZE)
        {
            g_mem_head.used_size = g_mem_head.used_size + adjust_size;
            g_mem_head.node_table[node_index].size = node_size + adjust_size;
        }
        else
        {
            g_mem_head.used_size = g_mem_head.used_size - adjust_size;
            g_mem_head.node_table[node_index].size = node_size - adjust_size;
        }
    }

    return err_code;
}

/**
 *   @brief     write key data then return a key handle to indicate this key
 *
 *   @param [in] data a pointer point to data buffer that want to be write
 *   @param [in] size size of data buffer
 *   @param [in/out] a pointer point to uint32 buffer used to store key handle 
 *
 *   @return     uint32_t
 *
 *   @note if input key handle is KDS_UNUSED_KEY_HANDLE the kds will assign a value as handle
 *         if input key handle is't KDS_UNUSED_KEY_HANDLE will use it value as key handle directly
 */
uint32_t kds_write_key(const void *data, uint16_t size, uint32_t *handle)
{
    uint32_t idx = 0;
    uint32_t key_type;
    uint16_t node_size;
    bool_t is_fixed_handle = false;
    bool_t is_update_node = false;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (g_kds_init != true)
    {
        ret = EHSM_ERR_NOT_INIT;
    }
    else if ((data == NULL) || (size == 0U) || (size > KMS_KEY_DATA_MAX_SIZE) || (handle == NULL))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        if (*handle == KDS_UNUSED_KEY_HANDLE)
        {
            key_type = KMS_KEY_TYPE_EHSM;
            ret = find_unused_key_handle_idx(&idx);
        }
        else
        {
            is_fixed_handle = true;
            key_type = (*handle & KMS_KEY_TYPE_MASK);

            ret = check_key_handle_exist(*handle, &idx);
            if (ret == EHSM_ERR_SW_SUCCESS)
            {
                is_update_node = true;
                node_size = g_mem_head.node_table[idx].size;
                uint16_t adjust_size = (size > node_size) ? (size - node_size) : (node_size - size);
                kds_adjust_node_mode_e adjust_mode = (size > node_size) ? INCREASE_NODE_SIZE : DECREASE_NODE_SIZE;
                ret = adjust_mem_node_size(idx, adjust_size, adjust_mode);
            }
            else
            {
                ret = find_unused_key_handle_idx(&idx);
            }
        }
    }

    if (ret == EHSM_ERR_SW_SUCCESS)
    {
        if ((is_update_node == false) && ((g_mem_head.mem_size - g_mem_head.used_size) < size))
        {
            ret = EHSM_ERR_NOT_FREE_SPACE;
        }
        else if (key_type != KMS_KEY_TYPE_EHSM)
        {
            ret = EHSM_ERR_INVALID_HANDLE;
        }
        else
        {
            if (is_fixed_handle == false)
            {
                *handle = key_type + idx;
            }

            if (is_update_node == false)
            {
                g_mem_head.node_num++;
                g_mem_head.node_table[idx].mem_offset = g_mem_head.used_size;
                g_mem_head.used_size += size;
                g_mem_head.node_table[idx].size = size;
                g_mem_head.node_table[idx].key_handle = *handle;
            }

            util_memcpy(kds_get_node_addr(g_mem_head.node_table[idx].mem_offset), data, size);
        }
    }

    return ret;
}

/**
 *   @brief     read key data for key_handle and store to user buffer
 *
 *   @param [in] key_handle The key handle indicate which key to be read
 *   @param [in] offset The offset of read data
 *   @param [in] data Buffer used to store key data
 *   @param [in] size The size of read key
 *
 *   @return     uint32_t
 *
 *   @note 
 */
uint32_t kds_read_key(uint32_t key_handle, uint16_t offset, void *data, uint16_t size)
{
    uint32_t idx = 0;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (g_kds_init != true)
    {
        ret = EHSM_ERR_NOT_INIT;
    }
    else if ((key_handle == KDS_UNUSED_KEY_HANDLE) || (data == NULL) || (size == 0U))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        ret = check_key_handle_exist(key_handle, &idx);
        if (ret == EHSM_ERR_SW_SUCCESS)
		{
            if ((offset >= g_mem_head.node_table[idx].size) || (size > (g_mem_head.node_table[idx].size - offset)))
			{
                ret = EHSM_ERR_OUT_OF_MEM;
            }
            else
            {
                util_memcpy(data, kds_get_node_addr(g_mem_head.node_table[idx].mem_offset + offset), (uint32_t)size);
            }
        }
    }

    return ret;
}

/**
 *   @brief     read key data for key_handle and store to user buffer
 *
 *   @param [in] key_handle The key handle indicate which key to be read
 *   @param [in] data Buffer used to store key data
 *   @param [in] size The size of Buffer
 *
 *   @return     uint32_t
 *
 *   @note
 */
uint32_t kds_read_entire_key(uint32_t key_handle, void *data, uint16_t size)
{
    uint32_t idx = 0;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (g_kds_init != true)
    {
        ret = EHSM_ERR_NOT_INIT;
    }
    else if ((key_handle == KDS_UNUSED_KEY_HANDLE) || (data == NULL) || (size == 0U))
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        ret = check_key_handle_exist(key_handle, &idx);
        if (ret == EHSM_ERR_SW_SUCCESS)
        {
            if (size < g_mem_head.node_table[idx].size)
            {
                ret = EHSM_ERR_OUT_OF_MEM;
            }
            else
            {
                util_memcpy(data, kds_get_node_addr(g_mem_head.node_table[idx].mem_offset),
                    (uint32_t)g_mem_head.node_table[idx].size);
            }
        }
    }

    return ret;
}

/**
 *   @brief     remove key
 *
 *   @param [in] key_handle The handle of remove key
 *
 *   @return     uint32_t
 *
 *   @note 
 */
uint32_t kds_remove_key(uint32_t key_handle)
{
    uint32_t idx = 0;
    uint16_t adjust_size;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (g_kds_init != true)
    {
        ret = EHSM_ERR_NOT_INIT;
    }
    else if (key_handle == KDS_UNUSED_KEY_HANDLE)
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        ret = check_key_handle_exist(key_handle, &idx);
        if (ret == EHSM_ERR_SW_SUCCESS)
        {
            adjust_size = g_mem_head.node_table[idx].size;
            ret = adjust_mem_node_size(idx, adjust_size, DECREASE_NODE_SIZE);
            if (ret == EHSM_ERR_SW_SUCCESS)
            {
                g_mem_head.node_num--;
                util_memset(&g_mem_head.node_table[idx], 0x00, sizeof(kds_mem_node_t));
                g_mem_head.node_table[idx].key_handle = KDS_UNUSED_KEY_HANDLE;
            }
        }
    }

    return ret;
}

/**
 *   @brief     init the kds(key data storage) module
 *
 *   @param [in] ram_key_addr The buffer used to store key data
 *   @param [in] ram_key_size The size of buffer
 *
 *   @return     uint32_t
 *
 *   @note 
 */
uint32_t kds_init(uint8_t *ram_key_addr, uint32_t ram_key_size)
{
    uint32_t i = 0;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((ram_key_addr == NULL) || (ram_key_size > UINT16_MAX) || (ram_key_size == 0U))
	{
        ret = EHSM_ERR_PARAM_ERROR;
    }
    else
    {
        util_memset(&g_mem_head, 0x00, sizeof(kds_mem_head_t));

        g_mem_head.mem_start = ram_key_addr;
        g_mem_head.mem_size = (uint16_t)ram_key_size;

        for (i = 0U; i < KDS_RAM_KEY_MAX_NUM; i++)
        {
            g_mem_head.node_table[i].key_handle = KDS_UNUSED_KEY_HANDLE;
        }

        g_kds_init = true;
    }

    return ret;
}

/**
 *   @brief     remove all key data
 *
 *   @return     uint32_t
 *
 *   @note 
 */
uint32_t kds_remove_all_key(void)
{
    uint32_t i = 0;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (g_kds_init != true)
	{
        ret = EHSM_ERR_NOT_INIT;
    } else {
        util_memset(g_mem_head.mem_start, 0x00, g_mem_head.mem_size);
        util_memset(g_mem_head.node_table, 0x00, sizeof(g_mem_head.node_table));
        g_mem_head.node_num = 0;
        g_mem_head.used_size = 0;

        for (i = 0U; i < KDS_RAM_KEY_MAX_NUM; i++)
		{
            g_mem_head.node_table[i].key_handle = KDS_UNUSED_KEY_HANDLE;
        }
    }

    return ret;
}

/**
 * @brief Move a block of memory from one location to another, handling overlapping regions correctly.
 *
 * This function moves a block of memory from the source location to the destination location.
 * It handles the case where the source and destination regions overlap by copying the data in the correct order.
 *
 * @param dest A pointer to the destination memory block where the data will be moved to.
 * @param src A pointer to the source memory block from which the data will be moved.
 * @param size The number of bytes to move.
 * @return A pointer to the destination memory block (same as the 'dest' parameter).
 */
void *kds_util_memmove(void *dest, const void *src, uint32_t size)
{
    void *ret = dest;
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    uint32_t n = size;

    if ((dest != NULL) && (src != NULL)) {
        if (d > s) {
            while ((n != 0U)) {
                n--;
                d[n] = s[n];
            }
        } else {
            while ((n != 0U)) {
                *d = *s;
                d++;
                s++;
                n--;
            }
        }
    }

    return ret;
}
