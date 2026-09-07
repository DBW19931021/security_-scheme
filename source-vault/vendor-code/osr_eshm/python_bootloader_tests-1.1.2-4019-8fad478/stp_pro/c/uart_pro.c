#include <stdio.h>
#include <string.h>

#include "strans_pro.h"
#include "uart_hal.h"

#if 0
#define log printf
void log_hex(const char *prefix, const uint8_t *data, uint32_t size)
{
    printf("%s", prefix);
    for (uint32_t i = 0; i < size; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

#else
#define log(...)
#define log_hex(...)
#endif

#define OFF_LEAD              0
#define OFF_TYPE              1
#define OFF_LEN               2
#define OFF_PAYLOAD           3

#define FRAME_LEAD            0x5A

#define STRING_FRAME_LEAD1    0xFE
#define STRING_FRAME_LEAD2    0xF0
#define STRING_FRAME_END1     0xFE
#define STRING_FRAME_END2     0xFE

#define STRING_BUFFER_SIZE    1024

#define TYPE_DATA             0x10 // data frame
#define TYPE_ACK              0x20 // ack frame
#define TYPE_SWITCH           0x40 // switch from send to recv mode reuqest frame
#define TYPE_ERR              0x80 // err frame
#define TYPE_RESYNC_REQ       0x08 // re-sync req frame
#define TYPE_RESYNC_ACK       0x04 // re-sync ack frame
#define TYPE_MASK             0xfc

#define ERR_CODE_CRC          0x01
#define ERR_CODE_CHAR_TIMEOUT 0x02
#define ERR_CODE_TYPE         0x03
#define ERR_CODE_MASK         0x03

#define MAX_PAYLOAD_SIZE      255
#define MAX_FRAME_SIZE        260
#define MAX_ERR_COUNT         5

#define CRC_INIT              0x0000

#define BLOCK_TIMEOUT         10000 // 10s
#define BYTE_TIMEOUT          500   // 100ms

#define ASSERT(expr)                                                        \
    if (!(expr)) {                                                          \
        printf("assertion failed: %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        while (1) { }                                                       \
    }

typedef struct {
    uint8_t recv_buf[MAX_PAYLOAD_SIZE];
    uint8_t recv_data_size;
    uint8_t recv_data_off;
    bool_t send_mode;
    bool_t first_init;
    bool_t is_client;                     // 设备角色标识，TRUE为客户端，FALSE为服务端
    stp_string_output_cb string_callback; // 字符串输出回调函数
} uart_pro_st;

static uart_pro_st s_up_data;

static uint32_t up_recv_byte(uint8_t *val);

static inline uint32_t min(uint32_t a, uint32_t b)
{
    return a < b ? a : b;
}

static inline uint32_t max(uint32_t a, uint32_t b)
{
    return a > b ? a : b;
}

static inline bool_t is_send_mode(void)
{
    return s_up_data.send_mode;
}

static inline void set_send_mode(void)
{
    s_up_data.send_mode = TRUE;
}

static inline void set_recv_mode(void)
{
    s_up_data.send_mode = FALSE;
}

static uint16_t up_calc_crc(const uint8_t *data, uint32_t size, uint16_t init)
{
    uint16_t val = init;
    uint16_t poly = 0x1021;
    uint16_t wchar = 0;
    while (size--) {
        wchar = *(data++);
        val ^= (wchar << 8);

        for (int i = 0; i < 8; i++) {
            if (val & 0x8000) {
                val = (val << 1) ^ poly;
            } else {
                val = val << 1;
            }
        }
    }
    return val;
}

static uint32_t up_send_frame(uint8_t type, const uint8_t *data, uint32_t size)
{
    if (size != 0 && data == NULL) {
        return STP_ERR_PARAM;
    }

    uint8_t head[3];

    head[OFF_LEAD] = FRAME_LEAD;
    head[OFF_TYPE] = type;
    head[OFF_LEN] = size;

    uint16_t crc = up_calc_crc(head, 3, CRC_INIT);
    if (size > 0) {
        crc = up_calc_crc(data, size, crc);
    }

    log("send frame: ");
    for (uint32_t i = 0; i < 3; i++) {
        log("%02x", head[i]);
    }
    for (uint32_t i = 0; i < size; i++) {

        log("%02x", data[i]);
    }
    log("%02x", crc >> 8);
    log("%02x", crc & 0xff);
    log("\n");

    for (uint32_t i = 0; i < 3; i++) {
        uint32_t ret = uart_send_byte(head[i]);
        if (ret != STP_OK) {
            return ret;
        }
    }

    for (uint32_t i = 0; i < size; i++) {
        uint32_t ret = uart_send_byte(data[i]);
        if (ret != STP_OK) {
            return ret;
        }
    }

    uint32_t ret = uart_send_byte((uint8_t)(crc >> 8));
    if (ret != STP_OK) {
        return ret;
    }
    ret = uart_send_byte((uint8_t)(crc));
    if (ret != STP_OK) {
        return ret;
    }

    return STP_OK;
}

static uint32_t up_recv_string_frame(void)
{
    uint8_t buf[STRING_BUFFER_SIZE];
    uint32_t buf_idx = 0;
    uint8_t val;
    uint32_t ret;

    // 已经检测到0xFE，继续检测第二字节是否为0xF0
    ret = up_recv_byte(&val);
    if (ret != STP_OK) {
        return ret;
    }

    if (val != STRING_FRAME_LEAD2) {
        // 不是字符串帧，忽略
        return STP_OK;
    }

    log("string frame detected\n");

    // 持续读取字符串内容，直到遇到结束标识或超时
    while (1) {
        ret = up_recv_byte(&val);
        if (ret != STP_OK) {
            // 超时或其他错误，输出已收到的内容
            if (buf_idx > 0 && s_up_data.string_callback != NULL) {
                buf[buf_idx] = '\0';
                s_up_data.string_callback((const char *)buf, buf_idx);
            }
            return STP_OK;
        }

        // 检测结束标识第一字节
        if (val == STRING_FRAME_END1) {
            ret = up_recv_byte(&val);
            if (ret != STP_OK) {
                // 超时，将0xFE作为数据处理
                if (buf_idx >= STRING_BUFFER_SIZE - 1) {
                    // 缓冲区满，输出当前内容并清空
                    buf[buf_idx] = '\0';
                    if (s_up_data.string_callback != NULL) {
                        s_up_data.string_callback((const char *)buf, buf_idx);
                    }
                    buf_idx = 0;
                }
                buf[buf_idx++] = STRING_FRAME_END1;
                if (s_up_data.string_callback != NULL) {
                    buf[buf_idx] = '\0';
                    s_up_data.string_callback((const char *)buf, buf_idx);
                }
                return STP_OK;
            }

            if (val == STRING_FRAME_END2) {
                // 检测到完整结束标识，输出字符串
                if (buf_idx > 0 && s_up_data.string_callback != NULL) {
                    buf[buf_idx] = '\0';
                    s_up_data.string_callback((const char *)buf, buf_idx);
                }
                log("string frame complete\n");
                return STP_OK;
            } else {
                // 不是结束标识，将两个字节都作为数据
                // 检查第一个字节是否会导致缓冲区满
                if (buf_idx >= STRING_BUFFER_SIZE - 1) {
                    // 缓冲区满，输出当前内容并清空
                    buf[buf_idx] = '\0';
                    if (s_up_data.string_callback != NULL) {
                        s_up_data.string_callback((const char *)buf, buf_idx);
                    }
                    buf_idx = 0;
                }
                buf[buf_idx++] = STRING_FRAME_END1;

                // 检查第二个字节是否会导致缓冲区满
                if (buf_idx >= STRING_BUFFER_SIZE - 1) {
                    // 缓冲区满，输出当前内容并清空
                    buf[buf_idx] = '\0';
                    if (s_up_data.string_callback != NULL) {
                        s_up_data.string_callback((const char *)buf, buf_idx);
                    }
                    buf_idx = 0;
                }
                buf[buf_idx++] = val;
            }
        } else {
            // 普通字符，检查缓冲区是否会满
            if (buf_idx >= STRING_BUFFER_SIZE - 1) {
                // 缓冲区满，输出当前内容并清空
                buf[buf_idx] = '\0';
                if (s_up_data.string_callback != NULL) {
                    s_up_data.string_callback((const char *)buf, buf_idx);
                }
                buf_idx = 0;
            }
            buf[buf_idx++] = val;
        }
    }
}

static uint32_t up_recv_byte(uint8_t *val)
{
    uint32_t ret;
    uint32_t start = stp_get_time_ms();
    while (1) {
        uint32_t end = stp_get_time_ms();
        if (end - start >= BYTE_TIMEOUT) {
            log("byte time out\n");
            return STP_ERR_TIMEOUT;
        }

        ret = uart_peek_byte(val);
        switch (ret) {
        case STP_ERR_PEEK_NO_DATA:
            continue;
        default:
            return ret;
        }
    }
}

static uint32_t up_recv_frame(uint8_t *buf)
{
    uint32_t ret;
    uint8_t len;
    // uint32_t start = stp_get_time_ms();
    // peek first byte
    while (1) {
        // uint32_t end = stp_get_time_ms();
        // if (end - start >= BLOCK_TIMEOUT) {
        //     return STP_ERR_TIMEOUT;
        // }

        ret = uart_peek_byte(&buf[OFF_LEAD]);
        switch (ret) {
        case STP_OK:
            if (buf[OFF_LEAD] == FRAME_LEAD) {
                goto MATCH_HEAD;
            } else if (buf[OFF_LEAD] == STRING_FRAME_LEAD1) {
                // 可能是字符串帧，尝试处理
                up_recv_string_frame();
                // 处理完字符串帧后继续等待数据帧
                continue;
            } else {
                continue;
            }
        case STP_ERR_PEEK_NO_DATA:
            continue;
        default:
            log("peed failed: %d\n", ret);
            return ret;
        }
    }

MATCH_HEAD:

    ret = up_recv_byte(&buf[OFF_TYPE]);
    if (ret != STP_OK) {
        return ret;
    }
    ret = up_recv_byte(&buf[OFF_LEN]);
    if (ret != STP_OK) {
        return ret;
    }
    len = buf[OFF_LEN];

    if (len > 0) {
        uint8_t recv_len = 0;

        while (len != recv_len) {
            ret = up_recv_byte(&buf[OFF_PAYLOAD + recv_len]);
            if (ret != STP_OK) {
                return ret;
            }
            recv_len++;
        }
    }

    uint8_t crc1, crc2;
    ret = up_recv_byte(&crc1);
    if (ret != STP_OK) {
        return ret;
    }
    ret = up_recv_byte(&crc2);
    if (ret != STP_OK) {
        return ret;
    }

    uint32_t crc = up_calc_crc(buf, 3 + len, CRC_INIT);

    log("recv frame: ");
    for (uint32_t i = 0; i < len + 3; i++) {
        log("%02x", buf[i]);
    }
    log("%02x", crc1);
    log("%02x", crc2);
    log("\n");

    if ((((uint16_t)crc1 << 8) | crc2) != crc) {
        log("frame crc error\n");
        return STP_ERR_CRC;
    }

    return STP_OK;
}

static void up_reset(void)
{
    // 根据角色设置初始模式
    if (s_up_data.is_client) {
        set_send_mode(); // 客户端初始为发送模式
    } else {
        set_recv_mode(); // 服务端初始为接收模式
    }
    s_up_data.first_init = TRUE;
    s_up_data.recv_data_off = 0;
    s_up_data.recv_data_size = 0;
}

static uint32_t up_transmit(uint8_t type, const uint8_t *data, uint32_t size, uint8_t expect_type, uint8_t *frame_buf)
{
    uint32_t err_count = 0;

    ASSERT(size <= MAX_PAYLOAD_SIZE);
    ASSERT(type == TYPE_DATA || type == TYPE_SWITCH || type == TYPE_ACK);

    uint32_t ret;
    if (s_up_data.first_init) {
        s_up_data.first_init = FALSE;
        if (!is_send_mode() && type == TYPE_ACK) {
            ret = STP_OK;
        } else {
            ret = up_send_frame(type, data, size);
        }
    } else {
        ret = up_send_frame(type, data, size);
    }
    if (ret != STP_OK) {
        return ret;
    }

    while (1) {

        uint8_t *buf = frame_buf;
        ret = up_recv_frame(buf);

        switch (ret) {
        case STP_OK: {
            uint8_t recv_type = buf[OFF_TYPE] & TYPE_MASK;
            if (recv_type & expect_type) {
                return STP_OK;
            }
            switch (recv_type) {
            case TYPE_RESYNC_REQ:
                ret = up_send_frame(TYPE_RESYNC_ACK, NULL, 0);
                up_reset(); // 不再传递参数，由角色决定模式
                return STP_ERR_RESYNC;
            case TYPE_ERR:
                err_count++;
                if (err_count >= MAX_ERR_COUNT) {
                    return ret;
                }
                // resend
                ret = up_send_frame(type, data, size);
                break;
            default:
                ret = up_send_frame(TYPE_ERR | ERR_CODE_TYPE, NULL, 0);
                break;
            }

            if (ret != STP_OK) {
                return ret;
            }
        } break;
        case STP_ERR_TIMEOUT:
            err_count++;
            if (err_count >= MAX_ERR_COUNT) {
                return ret;
            }
            ret = up_send_frame(TYPE_ERR | ERR_CODE_CHAR_TIMEOUT, NULL, 0);
            if (ret != STP_OK) {
                return ret;
            }
            break;
        case STP_ERR_CRC:
            err_count++;
            if (err_count >= MAX_ERR_COUNT) {
                return ret;
            }
            ret = up_send_frame(TYPE_ERR | ERR_CODE_CRC, NULL, 0);
            if (ret != STP_OK) {
                return ret;
            }
            break;
        default:
            return ret;
        }
    }
}

// 完整发送数据
static uint32_t up_send(const uint8_t *data, uint32_t size)
{
    log("up_send\n");
    uint8_t frame_buf[MAX_FRAME_SIZE];
    while (!is_send_mode()) {
        // 当前处于接收模式，接收并丢弃所有收到的数据，直到对方切换为接收模式
        s_up_data.recv_data_size = 0;
        s_up_data.recv_data_off = 0;

        log("drop data, send ack and wait data|switch\n");
        uint32_t ret = up_transmit(TYPE_ACK, NULL, 0, TYPE_DATA | TYPE_SWITCH, frame_buf);
        if (ret != STP_OK) {
            return ret;
        }

        if (frame_buf[OFF_TYPE] & TYPE_DATA) {
            // 收到的是数据帧，丢弃
            continue;
        } else {
            // 收到的是SWITCH帧，切换为接收模式
            set_send_mode();
        }
    }

    while (size > 0) {
        // 分块发送所有数据
        uint32_t send_len = min(size, MAX_PAYLOAD_SIZE);
        log("size: %u, send_len: %u\n", size, send_len);
        log("send data wait ack\n");
        uint32_t ret = up_transmit(TYPE_DATA, data, send_len, TYPE_ACK, frame_buf);
        if (ret != STP_OK) {
            log("up_transmit failed, ret: %u\n", ret);
            return ret;
        }
        size -= send_len;
        data += send_len;
    }
    return STP_OK;
}

// 完整接收数据
static uint32_t up_recv(uint8_t *buf, uint32_t size)
{
    uint8_t frame_buf[MAX_FRAME_SIZE];
    frame_buf[OFF_LEN] = 0;
    if (is_send_mode()) {
        log("now is send mode, send switch frame to notify that we need to switch to recv mode\n");
        ASSERT(s_up_data.recv_data_size == 0);
        // 主动切换到接收模式，直到收到数据帧
        uint32_t ret = up_transmit(TYPE_SWITCH, NULL, 0, TYPE_DATA, frame_buf);
        if (ret != STP_OK) {
            return ret;
        }
        s_up_data.recv_data_size = 0;
        s_up_data.recv_data_off = 0;
        set_recv_mode();
    }

    // copy from cache
    if (s_up_data.recv_data_size > 0) {
        log("cache is not empty, size: %u, off: %u\n", s_up_data.recv_data_size, s_up_data.recv_data_off);
        log_hex("cache: ", &s_up_data.recv_buf[s_up_data.recv_data_off], s_up_data.recv_data_size);

        uint32_t copy_len = min(s_up_data.recv_data_size, size);
        memcpy(buf, &s_up_data.recv_buf[s_up_data.recv_data_off], copy_len);
        size -= copy_len;
        buf += copy_len;
        s_up_data.recv_data_size -= copy_len;
        s_up_data.recv_data_off += copy_len;
    }

    uint32_t frame_len = frame_buf[OFF_LEN];
    while (size > 0) {
        ASSERT(s_up_data.recv_data_size == 0);
        ASSERT(!is_send_mode());

        if (frame_len == 0) {
            uint32_t ret = up_transmit(TYPE_ACK, NULL, 0, TYPE_DATA, frame_buf);
            if (ret != STP_OK) {
                return ret;
            }
            frame_len = frame_buf[OFF_LEN];
            log_hex("recv frame data: ", &frame_buf[OFF_PAYLOAD], frame_len);
        }

        // copy from data frame
        uint32_t copy_len = min(frame_len, size);
        memcpy(buf, &frame_buf[OFF_PAYLOAD], copy_len);
        size -= copy_len;
        frame_len -= copy_len;
        buf += copy_len;
    }

    ASSERT(size == 0);
    if (frame_len > 0) {
        ASSERT(s_up_data.recv_data_size == 0);

        log("frame_len: %u\n", frame_len);

        s_up_data.recv_data_size = frame_len;
        s_up_data.recv_data_off = 0;
        memcpy(s_up_data.recv_buf, &frame_buf[OFF_PAYLOAD + frame_buf[OFF_LEN] - frame_len], frame_len);

        log("current cache, size: %u, off: %u\n", s_up_data.recv_data_size, s_up_data.recv_data_off);
        log_hex("cache: ", &s_up_data.recv_buf[s_up_data.recv_data_off], s_up_data.recv_data_size);
    }

    return STP_OK;
}

static uint32_t up_init(bool_t is_client, stp_string_output_cb string_cb)
{
    memset(&s_up_data, 0, sizeof(s_up_data));
    s_up_data.is_client = is_client;       // 保存角色信息
    s_up_data.string_callback = string_cb; // 保存字符串输出回调函数

    up_reset();

    uint32_t ret = uart_init();
    if (ret != STP_OK) {
        return ret;
    }

    // 客户端主动发送RESYNC_REQ进行握手
    if (is_client) {
        uint8_t buf[MAX_FRAME_SIZE];
        uint32_t err_count = 0;
        while (1) {
            if (err_count > MAX_ERR_COUNT) {
                return STP_ERR_RESYNC;
            }
            ret = up_send_frame(TYPE_RESYNC_REQ, NULL, 0);
            if (ret != STP_OK) {
                err_count++;
                continue;
            }
            ret = up_recv_frame(buf);
            if (ret == STP_OK && (buf[OFF_TYPE] & TYPE_MASK) == TYPE_RESYNC_ACK) {
                break; // 握手成功
            }
            err_count++;
        }
    }
    // 服务端不需要主动发送，等待客户端RESYNC_REQ即可

    return STP_OK;
}

void up_deinit(void)
{
    uart_deinit();
}

// ------------------------------------------------------------- //
uint32_t stp_init(bool_t is_client, stp_string_output_cb string_cb)
{
    return up_init(is_client, string_cb);
}

uint32_t stp_send(const uint8_t *data, uint32_t size)
{
    return up_send(data, size);
}

uint32_t stp_recv(uint8_t *buf, uint32_t size)
{
    return up_recv(buf, size);
}

void stp_deinit(void)
{
    up_deinit();
}

void stp_reset_comm(void)
{
    up_reset();
}

uint32_t stp_send_string(const uint8_t *data, uint32_t size)
{
    if (data == NULL && size != 0) {
        return STP_ERR_PARAM;
    }

    uint32_t ret;

    // 发送起始标识：0xFE 0xF0
    ret = uart_send_byte(STRING_FRAME_LEAD1);
    if (ret != STP_OK) {
        return ret;
    }
    ret = uart_send_byte(STRING_FRAME_LEAD2);
    if (ret != STP_OK) {
        return ret;
    }

    // 发送字符串数据
    for (uint32_t i = 0; i < size; i++) {
        ret = uart_send_byte(data[i]);
        if (ret != STP_OK) {
            return ret;
        }
    }

    // 发送结束标识：0xFE 0xFE
    ret = uart_send_byte(STRING_FRAME_END1);
    if (ret != STP_OK) {
        return ret;
    }
    ret = uart_send_byte(STRING_FRAME_END2);
    if (ret != STP_OK) {
        return ret;
    }

    log("send string frame: size=%u\n", size);

    return STP_OK;
}

__attribute__((weak)) uint32_t stp_get_time_ms(void)
{
    return 0;
}
