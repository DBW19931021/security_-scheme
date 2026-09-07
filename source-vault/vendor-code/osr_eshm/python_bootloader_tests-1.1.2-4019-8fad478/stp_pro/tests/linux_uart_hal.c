#include "strans_pro.h"
#include "uart_hal.h"
#include "uart_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <time.h>

static int g_uart_fd = -1;

// 将波特率数值转换为Linux termios波特率
speed_t get_linux_baudrate(int baudrate)
{
    switch (baudrate) {
    case 9600:
        return B9600;
    case 19200:
        return B19200;
    case 38400:
        return B38400;
    case 57600:
        return B57600;
    case 115200:
        return B115200;
    case 230400:
        return B230400;
    case 460800:
        return B460800;
    case 921600:
        return B921600;
    default:
        printf("Warning: Unsupported baudrate %d, using 115200\n", baudrate);
        return B115200;
    }
}

uint32_t uart_init(void)
{
    // 优先使用全局配置，然后是环境变量
    const char* uart_port = g_uart_config.port;
    if (uart_port == NULL) {
        uart_port = getenv("UART_PORT");
        if (uart_port == NULL) {
            uart_port = DEFAULT_UART_PORT;
        }
    }

    printf("Opening Linux UART port: %s (baudrate: %d)\n", uart_port, g_uart_config.baudrate);

    // 打开串口设备
    g_uart_fd = open(uart_port, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (g_uart_fd < 0) {
        printf("Failed to open UART port %s: %s\n", uart_port, strerror(errno));
        return STP_ERR_HAL;
    }

    // 配置串口参数
    struct termios tty;
    if (tcgetattr(g_uart_fd, &tty) != 0) {
        printf("Failed to get UART attributes: %s\n", strerror(errno));
        close(g_uart_fd);
        g_uart_fd = -1;
        return STP_ERR_HAL;
    }

    // 优先使用全局配置中的波特率，然后是环境变量
    speed_t baudrate = get_linux_baudrate(g_uart_config.baudrate);
    const char* baud_str = getenv("UART_BAUDRATE");
    if (g_uart_config.baudrate == DEFAULT_BAUDRATE && baud_str != NULL) {
        // 如果全局配置是默认值且环境变量存在，则使用环境变量
        int env_baud = atoi(baud_str);
        if (env_baud > 0) {
            baudrate = get_linux_baudrate(env_baud);
            printf("Using baudrate from environment: %d\n", env_baud);
        }
    }

    // 设置波特率
    cfsetospeed(&tty, baudrate);
    cfsetispeed(&tty, baudrate);

    // 配置数据格式: 8N1 (8 data bits, no parity, 1 stop bit)
    tty.c_cflag &= ~PARENB;        // 无校验
    tty.c_cflag &= ~CSTOPB;        // 1个停止位
    tty.c_cflag &= ~CSIZE;         // 清除数据位设置
    tty.c_cflag |= CS8;            // 8个数据位
    tty.c_cflag &= ~CRTSCTS;       // 禁用硬件流控制
    tty.c_cflag |= CREAD | CLOCAL; // 启用读取，忽略调制解调器控制线

    // 配置输入模式：原始模式，禁用所有输入处理
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // 禁用软件流控制
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

    // 配置输出模式：原始模式，禁用所有输出处理
    tty.c_oflag &= ~OPOST;

    // 配置本地模式：原始模式，禁用回显和信号处理
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);

    // 配置控制字符：设置为非阻塞读取
    tty.c_cc[VMIN] = 0;  // 读取时不等待字符
    tty.c_cc[VTIME] = 0; // 读取超时为0（立即返回）

    // 应用配置
    if (tcsetattr(g_uart_fd, TCSANOW, &tty) != 0) {
        printf("Failed to set UART attributes: %s\n", strerror(errno));
        close(g_uart_fd);
        g_uart_fd = -1;
        return STP_ERR_HAL;
    }

    // 清空输入输出缓冲区
    tcflush(g_uart_fd, TCIOFLUSH);

    printf("Linux UART initialized successfully: %s at %d baud\n", uart_port, g_uart_config.baudrate);

    return STP_OK;
}

uint32_t uart_send_byte(uint8_t val)
{
    if (g_uart_fd < 0) {
        printf("UART not initialized\n");
        return STP_ERR_HAL;
    }

    ssize_t bytes_written = write(g_uart_fd, &val, 1);
    if (bytes_written < 0) {
        printf("Failed to write byte: %s\n", strerror(errno));
        return STP_ERR_HAL;
    }

    if (bytes_written == 0) {
        printf("No bytes written\n");
        return STP_ERR_HAL;
    }

    // 确保数据被发送到硬件
    if (tcdrain(g_uart_fd) != 0) {
        printf("Failed to drain write buffer: %s\n", strerror(errno));
        return STP_ERR_HAL;
    }

    return STP_OK;
}

uint32_t uart_peek_byte(uint8_t* buf)
{
    if (g_uart_fd < 0) {
        printf("UART not initialized\n");
        return STP_ERR_HAL;
    }

    if (buf == NULL) {
        printf("Buffer pointer is NULL\n");
        return STP_ERR_PARAM;
    }

    ssize_t bytes_read = read(g_uart_fd, buf, 1);
    if (bytes_read < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // 非阻塞读取，无数据可用
            return STP_ERR_PEEK_NO_DATA;
        } else {
            printf("Failed to read byte: %s\n", strerror(errno));
            return STP_ERR_HAL;
        }
    }

    if (bytes_read == 0) {
        // 无数据可用
        return STP_ERR_PEEK_NO_DATA;
    }

    return STP_OK;
}

void uart_deinit(void)
{
    if (g_uart_fd >= 0) {
        close(g_uart_fd);
        g_uart_fd = -1;
        printf("Linux UART deinitialized\n");
    }
}

uint32_t stp_get_time_ms(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        printf("Failed to get time: %s\n", strerror(errno));
        return 0;
    }

    // 转换为毫秒
    return (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}
