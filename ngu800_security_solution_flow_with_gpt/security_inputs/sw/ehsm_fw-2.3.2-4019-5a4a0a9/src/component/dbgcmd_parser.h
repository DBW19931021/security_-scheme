#ifndef DBGCMD_PARSER_H
#define DBGCMD_PARSER_H

/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "types.h"
#include "config.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define MAX_DBG_PROTO_SIZE 2048U

#define DBG_CMD_ID 0xFFF0U
/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/
typedef enum {
    DBG_STATE_INIT = 0U,
    DBG_STATE_RECEIVING,
    DBG_STATE_RECV_COMPLETE,
    DBG_STATE_PROCESS,
    DBG_STATE_SENDING_RESP
} dbg_state_e;

typedef struct {
    /*buffer for received data*/
    uint8_t cmd_buffer[MAX_DBG_PROTO_SIZE];
    /*size of received data*/
    uint32_t data_size;
    /*size of response data*/
    uint32_t rsp_size;
    /*index of response data to be send*/
    uint32_t rsp_index;
    /*refers to uart_cmd_state_e*/
    dbg_state_e state;
} dbgcmdparser_st;

typedef struct {
    uint16_t cmd_id;     /* [offset: 0] command id = 0x0103 */
    uint16_t cmd_id_inv; /* [offset: 2] command id inverse */
    dbgcmdparser_st *parser;
} dbg_cmd_st;

typedef struct {
    uint8_t *data;
    uint32_t data_size;
    uint8_t *reps;
    uint32_t reps_size;
} dbg_cmd_param_st;
/***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t dbgcmdpars_init_parser(dbgcmdparser_st *parser);

uint32_t dbgcmdpars_append_data(dbgcmdparser_st *parser, const uint8_t *data, uint32_t size);

uint32_t dbgcmdpars_recv_complete(const dbgcmdparser_st *parser);

uint32_t dbgcmdpars_trans_to_packet(dbgcmdparser_st *parser, cmd_packet_st *packet);

uint32_t dbgcmdpars_srv_handler(const cmd_packet_st *packet);
#endif // DBGCMD_PARSER_H
