/// Copyright by Wingsemi LLC © 2023. See LICENSE for details
/// @file       <sys_call.c>
///
#ifdef __GNUC__
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "uart_stdout.h"

#undef errno
extern int errno;

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

__attribute__((weak)) int _isatty(int fd)
{
    (void)fd;
    return 1;
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

__attribute__((weak)) void *_sbrk(ptrdiff_t incr)
{
    extern char __heap_start[];
    extern char __heap_end[];
    static char *curbrk = __heap_start;

    if ((curbrk + incr < __heap_start) || (curbrk + incr > __heap_end)) {
        return (void *)(-1);
    }

    curbrk += incr;
    return (void *)(curbrk - incr);
}

__attribute__((weak)) ssize_t _write(int fd, const void *ptr, size_t len)
{
    if (!isatty(fd)) {
        return -1;
    }

    const uint8_t *writebuf = (const uint8_t *)ptr;

    for (size_t i = 0; i < len; i++) {
        UartPutc(writebuf[i]);
    }

    return (ssize_t)len;
}

#elif defined(__ICCRISCV__)

#include <stdint.h>
int __write(int fd, const void *ptr, uint32_t len)
{
    const uint8_t *writebuf = (const uint8_t *)ptr;

    for (uint32_t i = 0; i < len; i++) {
        UartPutc(writebuf[i]);
    }

    return len;
}

#else
#endif
