/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "cmd_pool.h"
#include "queue.h"

#include "util.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define CMD_BUFFER_INDEX_INVALID ((uint32_t)0xFFFFFFFFU)
#define DEFAULT_CMD_PACKET_NUM   32U
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
static cmd_packet_st g_cmd_buffer[DEFAULT_CMD_PACKET_NUM];
static bool_t g_cmd_used[DEFAULT_CMD_PACKET_NUM];

static queue_st g_cmd_queue[1];

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
static uint32_t cmdpool_get_cmd_index(const cmd_packet_st *packet)
{
    uint32_t index;
    uint32_t offset;
    uint32_t packet_size;

    if ((packet < &g_cmd_buffer[0]) || (packet > &g_cmd_buffer[DEFAULT_CMD_PACKET_NUM - 1U])) {
        index = CMD_BUFFER_INDEX_INVALID;
    } else {
        packet_size = sizeof(cmd_packet_st);
        offset = (uint32_t)((uintptr_t)packet - (uintptr_t)&(g_cmd_buffer[0]));
        if (0U == (uint32_t)(offset % packet_size)) {
            index = offset / packet_size;
        } else {
            index = CMD_BUFFER_INDEX_INVALID;
        }
    }
    return index;
}

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t cmdpool_init(void)
{
    uint32_t i;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    static queue_element_t g_queue_element_array[DEFAULT_CMD_PACKET_NUM];

    util_memset(&g_cmd_buffer[0], 0, sizeof(cmd_packet_st) * DEFAULT_CMD_PACKET_NUM);
    ret = queue_init(g_cmd_queue, g_queue_element_array, DEFAULT_CMD_PACKET_NUM);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        for (i = 0U; i < DEFAULT_CMD_PACKET_NUM; i++) {
            ret = queue_push(g_cmd_queue, (queue_element_t)&g_cmd_buffer[i]);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                g_cmd_used[i] = false;
            } else {
                break;
            }  
        }
    }

    return ret;
}

uint32_t cmdpool_alloc(cmd_packet_st **packet)
{
    uint32_t ret;
    queue_element_t queue_data;
    uint32_t index;

    if (NULL == packet) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        ret = queue_pop(g_cmd_queue, &queue_data);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            util_memcpy((void *)packet, &queue_data, sizeof(queue_element_t));
            index = cmdpool_get_cmd_index(*packet);
            if (CMD_BUFFER_INDEX_INVALID != index) {
                g_cmd_used[index] = true;
            }
        } else if (EHSM_ERR_QUEUE_EMPTY == ret) {
            ret = EHSM_ERR_CMDPOOL_EMPTY;
        } else {
            //never run here, do nothing
        }
    }

    return ret;
}

uint32_t cmdpool_free(cmd_packet_st *packet)
{
    uint32_t ret;
    uint32_t index;

    if (NULL == packet) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        index = cmdpool_get_cmd_index(packet);
        if (CMD_BUFFER_INDEX_INVALID == index) {
            ret = EHSM_ERR_PARAM_ERROR;
        } else if (!g_cmd_used[index]) {
            ret = EHSM_ERR_FREE_REPEATED;
        } else {
            ret = queue_push(g_cmd_queue, (queue_element_t)packet);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                g_cmd_used[index] = false;
            }
        }
    }

    return ret;
}

/**
 *
 */
