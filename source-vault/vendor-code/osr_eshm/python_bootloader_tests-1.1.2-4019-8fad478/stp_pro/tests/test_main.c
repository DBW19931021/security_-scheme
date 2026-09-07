#include "strans_pro.h"
#include "uart_config.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// 定义全局配置变量
uart_config_t g_uart_config
    = { .port = DEFAULT_UART_PORT, .baudrate = DEFAULT_BAUDRATE, .is_client = DEFAULT_IS_CLIENT };

uint8_t g_buf[8192];

#if 0
// 需要将uart_pro.c里的 #if 0 打开
void log_hex(const char *prefix, const uint8_t *data, uint32_t size);
#else
#define log_hex(...)
#endif

// 字符串输出回调函数
void string_output_handler(const char *str, uint32_t len)
{
    printf("[STRING] %.*s\n", len, str);
}

// 显示使用帮助
void show_usage(const char *prog_name)
{
    printf("Usage: %s [options]\n", prog_name);
    printf("Options:\n");
    printf("  -p <port>     Serial port device (default: %s)\n", DEFAULT_UART_PORT);
    printf("  -b <baudrate> Baud rate (default: %d)\n", DEFAULT_BAUDRATE);
    printf("  -c            Run as client (default: server)\n");
    printf("  -h            Show this help message\n");
    printf("\n");
    printf("Examples:\n");
#ifdef _WIN32
    printf("  %s -p COM8 -b 57600      # Server on COM8 at 57600 baud\n", prog_name);
    printf("  %s -c -p COM3 -b 115200  # Client on COM3 at 115200 baud\n", prog_name);
#else
    printf("  %s -p /dev/ttyUSB0 -b 57600      # Server on /dev/ttyUSB0 at 57600 baud\n", prog_name);
    printf("  %s -c -p /dev/ttyACM0 -b 115200  # Client on /dev/ttyACM0 at 115200 baud\n", prog_name);
#endif
    printf("\n");
}

// 解析命令行参数
int parse_args(int argc, const char **argv)
{
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            if (i + 1 >= argc) {
                printf("Error: -p option requires a port name\n");
                return -1;
            }
            g_uart_config.port = argv[++i];
        } else if (strcmp(argv[i], "-b") == 0) {
            if (i + 1 >= argc) {
                printf("Error: -b option requires a baudrate value\n");
                return -1;
            }
            g_uart_config.baudrate = atoi(argv[++i]);
            if (g_uart_config.baudrate <= 0) {
                printf("Error: Invalid baudrate %s\n", argv[i]);
                return -1;
            }
        } else if (strcmp(argv[i], "-c") == 0) {
            g_uart_config.is_client = 1;
        } else if (strcmp(argv[i], "-h") == 0) {
            show_usage(argv[0]);
            return 1;
        } else {
            printf("Error: Unknown option %s\n", argv[i]);
            show_usage(argv[0]);
            return -1;
        }
    }
    return 0;
}

uint32_t rand_send(const uint8_t *data, uint32_t size)
{
    printf("total send %u data\n", size);
    uint32_t send_size = 0;
    while (send_size < size) {
        uint32_t rand_size = rand() % (size - send_size) + 1;
        printf(">>> prepare to send %u bytes\n", rand_size);
        uint32_t ret = stp_send(data, rand_size);
        if (ret != STP_OK) {
            return ret;
        }
        send_size += rand_size;
        data += rand_size;
        printf(">>> send %u, left: %u\n", send_size, size - send_size);
    }
    return STP_OK;
}

uint32_t rand_recv(uint8_t *data, uint32_t size)
{
    printf("total recv %u data\n", size);
    uint32_t recv_size = 0;
    while (recv_size < size) {
        uint32_t rand_size = rand() % (size - recv_size) + 1;
        printf(">>> prepare to recv %u bytes\n", rand_size);
        uint32_t ret = stp_recv(data, rand_size);
        if (ret != STP_OK) {
            return ret;
        }
        recv_size += rand_size;
        data += rand_size;
        printf(">>> recv %u, left: %u\n", recv_size, size - recv_size);
        log_hex("g_buf: ", g_buf, recv_size);
    }

    return STP_OK;
}

int main(int argc, const char **argv)
{
    // 解析命令行参数
    int parse_result = parse_args(argc, argv);
    if (parse_result != 0) {
        return parse_result > 0 ? 0 : 1; // 正数表示正常退出（如显示帮助），负数表示错误
    }

    // 显示配置信息
    printf("STP Protocol Test Server/Client\n");
    printf("Configuration:\n");
    printf("  Port: %s\n", g_uart_config.port);
    printf("  Baudrate: %d\n", g_uart_config.baudrate);
    printf("  Role: %s\n", g_uart_config.is_client ? "Client" : "Server");
    printf("\n");

    srand(time(NULL));
    printf("Initializing STP protocol...\n");
    uint32_t ret = stp_init(g_uart_config.is_client, string_output_handler);
    if (ret != STP_OK) {
        printf("STP initialization failed with error code: %u\n", ret);
        return 1;
    }

    const char *tip = "STP protocol initialized successfully\n\n";
    printf("%s", tip);
    stp_send_string((const uint8_t *)tip, strlen(tip));

    uint32_t i = 0;

    while (1) {
        uint8_t len_buf[2];
        ret = rand_recv(len_buf, 2);
        if (ret != STP_OK) {
            continue;
        }

        memset(g_buf, 0x5a, sizeof(g_buf));
        uint32_t data_len = ((uint32_t)len_buf[0] << 8) | len_buf[1];
        printf("recv len: %u\n", data_len);

        ret = rand_recv(g_buf, data_len);
        if (ret != STP_OK) {
            continue;
        }

        log_hex("recv data: ", g_buf, data_len);

        // just echo back
        rand_send(g_buf, data_len);

        printf("-----------------------!! command %u over !!--------------------\n", i);
        char str_buf[128];
        sprintf(str_buf, "%u done\n", i + 1);
        stp_send_string((const uint8_t *)str_buf, strlen(str_buf));
        i++;
    }
    return 0;
}
