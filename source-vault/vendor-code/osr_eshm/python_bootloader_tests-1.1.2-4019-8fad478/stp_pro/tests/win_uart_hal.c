#include "strans_pro.h"
#include "uart_hal.h"
#include "uart_config.h"
#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>

HANDLE g_hcom = INVALID_HANDLE_VALUE;

// 将字符串转换为宽字符串
WCHAR* str_to_wstr(const char* str)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    WCHAR* wstr = (WCHAR*)malloc(len * sizeof(WCHAR));
    MultiByteToWideChar(CP_UTF8, 0, str, -1, wstr, len);
    return wstr;
}

// 将波特率数值转换为Windows DCB波特率
DWORD get_windows_baudrate(int baudrate)
{
    switch (baudrate) {
    case 9600:
        return CBR_9600;
    case 19200:
        return CBR_19200;
    case 38400:
        return CBR_38400;
    case 57600:
        return CBR_57600;
    case 115200:
        return CBR_115200;
    default:
        printf("Warning: Unsupported baudrate %d, using 115200\n", baudrate);
        return CBR_115200;
    }
}

uint32_t uart_init()
{
    // 构建完整的COM端口路径（如果需要）
    char full_port[64];
    if (strncmp(g_uart_config.port, "COM", 3) == 0 && strlen(g_uart_config.port) <= 4) {
        // 简单的COMx格式，需要转换为\\.\COMx格式
        snprintf(full_port, sizeof(full_port), "\\\\.\\%s", g_uart_config.port);
    } else {
        // 已经是完整路径或其他格式
        strncpy(full_port, g_uart_config.port, sizeof(full_port) - 1);
        full_port[sizeof(full_port) - 1] = '\0';
    }

    // 转换为宽字符串
    WCHAR* wport = str_to_wstr(full_port);

    printf("Opening Windows COM port: %s (baudrate: %d)\n", full_port, g_uart_config.baudrate);

    g_hcom = CreateFileW(wport, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    free(wport);

    if (g_hcom == INVALID_HANDLE_VALUE) {
        printf("Failed to open COM port %s: Error %lu\n", full_port, GetLastError());
        return STP_ERR_HAL;
    }

    DCB params = { 0 };
    params.DCBlength = sizeof(DCB);
    if (!GetCommState(g_hcom, &params)) {
        printf("get COM port state failed\n");
        CloseHandle(g_hcom);
        return 2;
    }

    params.BaudRate = get_windows_baudrate(g_uart_config.baudrate);
    params.ByteSize = 8;
    params.StopBits = ONESTOPBIT;
    params.Parity = NOPARITY;

    if (!SetCommState(g_hcom, &params)) {
        CloseHandle(g_hcom);
        return STP_ERR_HAL;
    }

    // 设置超时参数，使 ReadFile 非阻塞
    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 0;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 0;

    if (!SetCommTimeouts(g_hcom, &timeouts)) {
        printf("Set COM port timeouts failed\n");
        CloseHandle(g_hcom);
        return STP_ERR_HAL;
    }

    printf("Windows UART initialized successfully: %s at %d baud\n", g_uart_config.port, g_uart_config.baudrate);

    return STP_OK;
}

uint32_t uart_send_byte(uint8_t val)
{
    if (g_hcom == INVALID_HANDLE_VALUE) {
        printf("COM port handle error\n");
        return 1;
    }

    DWORD bytes_written = 0;
    while (bytes_written == 0) {
        if (!WriteFile(g_hcom, &val, 1, &bytes_written, NULL)) {
            printf("Write File failed\n");
            return 2;
        }
    }
    return STP_OK;
}

uint32_t uart_peek_byte(uint8_t* buf)
{
    if (g_hcom == INVALID_HANDLE_VALUE) {
        printf("COM port handle error\n");
        return 1;
    }

    DWORD bytes_read = 0;
    if (!ReadFile(g_hcom, buf, 1, &bytes_read, NULL)) {
        printf("Read File failed\n");
        return 2;
    }
    if (bytes_read == 0) {
        return STP_ERR_PEEK_NO_DATA;
    }
    return STP_OK;
}

void uart_deinit(void)
{
    if (g_hcom != INVALID_HANDLE_VALUE) {
        CloseHandle(g_hcom);
        g_hcom = INVALID_HANDLE_VALUE;
        printf("Windows UART deinitialized\n");
    }
}

uint32_t stp_get_time_ms(void)
{
    return GetTickCount();
}
