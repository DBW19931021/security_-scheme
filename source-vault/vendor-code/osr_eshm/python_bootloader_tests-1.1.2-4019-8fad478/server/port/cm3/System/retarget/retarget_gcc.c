/// Copyright by Wingsemi LLC © 2023. See LICENSE for details
/// @file       <sys_call.c>
///
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "uart_stdout.h"

#undef errno
extern int errno;

__attribute__((weak)) void _exit(int fd)
{
    (void)fd;
    while (1) {
        ;
    }
}

__attribute__((weak)) int _close(int fd)
{
    (void)fd;
    errno = EBADF;
    return -1;
}

__attribute__((weak)) int _lseek(int file, int offset, int whence)
{
    (void)file;
    (void)offset;
    (void)whence;
    return 0;
}

__attribute__((weak)) int _fstat(int file, struct stat *st)
{
    if ((STDOUT_FILENO == file) || (STDERR_FILENO == file)) {
        st->st_mode = S_IFCHR;
        return 0;
    } else {
        errno = EBADF;
        return -1;
    }
}

__attribute__((weak)) int _isatty(int fd)
{
    (void)fd;
    return 1;
}

__attribute__((weak)) void *_sbrk(int incr)
{
    extern char __heap_start; /* 链接脚本定义的堆起始 */
    extern char __heap_end;   /* 链接脚本定义的堆结束 */

    static char *heap_ptr = NULL;
    char *prev_heap_ptr;
    if (heap_ptr == NULL) {
        heap_ptr = &__heap_start; /* 首次调用初始化 */
    }
    prev_heap_ptr = heap_ptr;
    if (heap_ptr + incr > &__heap_end) {
        errno = ENOMEM;
        return (void *)-1; /* 堆溢出 */
    }
    heap_ptr += incr;
    return (void *)prev_heap_ptr;
}

__attribute__((weak)) int _read(int fd, void *ptr, int len)
{
    if (fd != STDIN_FILENO) {
        return -1;
    }
    ssize_t cnt = 0;
    uint8_t *readbuf = (uint8_t *)ptr;
    for (cnt = 0; cnt < len; cnt++) {
        readbuf[cnt] = UartGetc();
        /* Return partial buffer if we get EOL */
        if (readbuf[cnt] == '\n') {
            return cnt;
        }
    }

    return cnt;
}

__attribute__((weak)) int _write(int fd, const void *ptr, int len)
{
    if (!isatty(fd)) {
        return -1;
    }

    const uint8_t *writebuf = (const uint8_t *)ptr;

    for (int i = 0; i < len; i++) {
        UartPutc(writebuf[i]);
    }

    return len;
}

__attribute__((weak)) int _kill(int pid, int sig)
{
    (void)pid;
    (void)sig;
    errno = EINVAL;
    return -1;
}

__attribute__((weak)) int _getpid(void)
{
    return 1;
}
