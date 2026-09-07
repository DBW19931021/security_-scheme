#ifndef UART_CONFIG_H
#define UART_CONFIG_H

// UART配置结构体
typedef struct {
    const char* port; // 串口设备名称
    int baudrate;     // 波特率
    int is_client;    // 是否为客户端角色 (1=客户端, 0=服务端)
} uart_config_t;

// 全局配置变量
extern uart_config_t g_uart_config;

// 默认配置
#ifdef _WIN32
#define DEFAULT_UART_PORT "COM1"
#else
#define DEFAULT_UART_PORT "/dev/ttyACM0"
#endif

#define DEFAULT_BAUDRATE  115200
#define DEFAULT_IS_CLIENT 0 // 默认为服务端

#endif // UART_CONFIG_H
