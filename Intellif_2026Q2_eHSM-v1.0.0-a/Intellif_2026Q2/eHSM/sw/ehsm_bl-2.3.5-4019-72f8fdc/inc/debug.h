/*
 * debug.h
 *
 */

#ifndef DEBUG_H
#define DEBUG_H
/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "stdio.h"
#include "string.h"
#include "types.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
static inline const char *filename(const char *p)
{
    const char *slash = strrchr(p, '/');
    return slash ? slash + 1 : p;
}

#define LOG_ENABLE 1 /* enable log */
#define LOG_PREFIX 0 /* add level, file, line information for each message */
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_INFO
#endif // LOG_LEVEL

#if LOG_ENABLE
uint32_t debug_printf(const char *fmt, ...);
void debug_flush(void);

#define LOG_NONE 0
#define LOG_ERR  1
#define LOG_WARN 2
#define LOG_INFO 3
#define LOG_DBG  4

#if LOG_PREFIX
#define _log_msg(level, fmt, ...)                                                                    \
    do {                                                                                             \
        if ((level) <= LOG_LEVEL) {                                                                  \
            switch (level) {                                                                         \
            case LOG_ERR:                                                                            \
                debug_printf("[ehsm_err]:(%s %s %d)", filename(__FILE__), __FUNCTION__, __LINE__);   \
                break;                                                                               \
            case LOG_WARN:                                                                           \
                debug_printf("[ehsm_warn]:(%s %s %d)", filename(__FILE__), __FUNCTION__, __LINE__);  \
                break;                                                                               \
            case LOG_DBG:                                                                            \
                debug_printf("[ehsm_debug]:(%s %s %d)", filename(__FILE__), __FUNCTION__, __LINE__); \
                break;                                                                               \
            case LOG_INFO:                                                                           \
                debug_printf("[ehsm_info]:(%s %s %d)", filename(__FILE__), __FUNCTION__, __LINE__);  \
                break;                                                                               \
            default:                                                                                 \
                break;                                                                               \
            }                                                                                        \
            debug_printf(fmt, ##__VA_ARGS__);                                                        \
        }                                                                                            \
    } while (0)
#else // !LOG_PREFIX
#define _log_msg(level, fmt, ...) debug_printf(fmt, ##__VA_ARGS__)
#endif // LOG_PREFIX

#define log_flush() debug_flush()

// judge case print or not
#if (LOG_LEVEL >= LOG_ERR)
#define log_error(...) _log_msg(LOG_ERR, ##__VA_ARGS__)
#else
#define log_error(...)
#endif

#if (LOG_LEVEL >= LOG_WARN)
#define log_warn(...) _log_msg(LOG_WARN, ##__VA_ARGS__)
#else
#define log_warn(...)
#endif

#if (LOG_LEVEL >= LOG_DBG)
#define log_debug(...) _log_msg(LOG_DBG, ##__VA_ARGS__)
#else
#define log_debug(...)
#endif

#if (LOG_LEVEL >= LOG_INFO)
#define log_info(...) _log_msg(LOG_INFO, ##__VA_ARGS__)
#else
#define log_info(...)
#endif

#if (LOG_LEVEL >= LOG_DBG)
static inline void log_debug_hex(const char *prefix, const uint8_t *data, uint32_t size)
{
    log_debug("%s: [%u]", prefix, size);
    for (uint32_t i = 0; i < size; i++) {
        log_debug("%02x", data[i]);
    }
    log_debug("\n");
}
#else
#define log_debug_hex(...)
#endif

#else // !LOG_ENABLE

#define log_error(...)
#define log_warn(...)
#define log_debug(...)
#define log_info(...)
#define log_flush()
#define log_debug_hex(...)

#endif // LOG_ENABLE

/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
#endif
