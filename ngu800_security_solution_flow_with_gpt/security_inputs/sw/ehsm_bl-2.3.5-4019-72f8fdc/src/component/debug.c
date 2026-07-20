/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include <stdarg.h>

#include "debug.h"
#include "uart.h"
#include "sysreg.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define PRINTF_PRECISION

#define LOG_SIZE 512

#define ZEROPAD (1 << 0) /* pad with zero */
#define SIGN    (1 << 1) /* unsigned/signed long */
#define PLUS    (1 << 2) /* show plus */
#define SPACE   (1 << 3) /* space if plus */
#define LEFT    (1 << 4) /* left justified */
#define SPECIAL (1 << 5) /* 0x */
#define LARGE   (1 << 6) /* use 'ABCDEF' instead of 'abcdef' */

#define _ISDIGIT(c) ((unsigned)((c) - '0') < 10)
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

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/
static void debug_uart_puts(uint8_t *log, uint32_t size)
{
    uint32_t i;

    for (i = 0; i < size; i++) {
        uart_putc(log[i]);
    }
}

#ifdef PRINTF_LONGLONG
static inline int divide(long long *n, int base)
#else
static inline int divide(long *n, int base)
#endif /* PRINTF_LONGLONG */
{
    int res;

    /* optimized for processor which does not support divide instructions. */
    if (base == 10) {
#ifdef PRINTF_LONGLONG
        res = (int)(((unsigned long long)*n) % 10U);
        *n = (long long)(((unsigned long long)*n) / 10U);
#else
        res = (int)(((unsigned long)*n) % 10U);
        *n = (long)(((unsigned long)*n) / 10U);
#endif
    } else {
#ifdef PRINTF_LONGLONG
        res = (int)(((unsigned long long)*n) % 16U);
        *n = (long long)(((unsigned long long)*n) / 16U);
#else
        res = (int)(((unsigned long)*n) % 16U);
        *n = (long)(((unsigned long)*n) / 16U);
#endif
    }

    return res;
}

static char *print_number(char *buf, char *end,
#ifdef PRINTF_LONGLONG
    long long num,
#else
    long num,
#endif /* PRINTF_LONGLONG */
    int base, int s,
#ifdef PRINTF_PRECISION
    int precision,
#endif /* PRINTF_PRECISION */
    int type)
{
    char c, sign;
#ifdef PRINTF_LONGLONG
    char tmp[32];
#else
    char tmp[16];
#endif /* PRINTF_LONGLONG */

#ifdef PRINTF_PRECISION
    int precision_bak = precision;
#endif
    const char *digits;
    static const char small_digits[] = "0123456789abcdef";
    static const char large_digits[] = "0123456789ABCDEF";
    register int i;
    register int size;

    size = s;

    digits = (type & LARGE) ? large_digits : small_digits;
    if (type & LEFT)
        type &= ~ZEROPAD;

    c = (type & ZEROPAD) ? '0' : ' ';

    /* get sign */
    sign = 0;
    if (type & SIGN) {
        if (num < 0) {
            sign = '-';
            num = -num;
        } else if (type & PLUS)
            sign = '+';
        else if (type & SPACE)
            sign = ' ';
    }

#ifdef RT_PRINTF_SPECIAL
    if (type & SPECIAL) {
        if (base == 16)
            size -= 2;
        else if (base == 8)
            size--;
    }
#endif /* RT_PRINTF_SPECIAL */

    i = 0;
    if (num == 0)
        tmp[i++] = '0';
    else {
        while (num != 0)
            tmp[i++] = digits[divide(&num, base)];
    }

#ifdef PRINTF_PRECISION
    if (i > precision)
        precision = i;
    size -= precision;
#else
    size -= i;
#endif /* PRINTF_PRECISION */

    if (!(type & (ZEROPAD | LEFT))) {
        if ((sign) && (size > 0))
            size--;

        while (size-- > 0) {
            if (buf < end)
                *buf = ' ';
            ++buf;
        }
    }

    if (sign) {
        if (buf < end) {
            *buf = sign;
        }
        --size;
        ++buf;
    }

#ifdef RT_PRINTF_SPECIAL
    if (type & SPECIAL) {
        if (base == 8) {
            if (buf < end)
                *buf = '0';
            ++buf;
        } else if (base == 16) {
            if (buf < end)
                *buf = '0';
            ++buf;
            if (buf < end) {
                *buf = type & LARGE ? 'X' : 'x';
            }
            ++buf;
        }
    }
#endif /* RT_PRINTF_SPECIAL */

    /* no align to the left */
    if (!(type & LEFT)) {
        while (size-- > 0) {
            if (buf < end)
                *buf = c;
            ++buf;
        }
    }

#ifdef PRINTF_PRECISION
    while (i < precision--) {
        if (buf < end)
            *buf = '0';
        ++buf;
    }

    /* put number in the temporary buffer */
    while (i-- > 0 && (precision_bak != 0)) {
        if (buf < end)
            *buf = tmp[i];
        ++buf;
    }
#endif /* PRINTF_PRECISION */

    while (size-- > 0) {
        if (buf < end)
            *buf = ' ';
        ++buf;
    }

    return buf;
}

static inline int skip_atoi(const char **s)
{
    register int i = 0;
    while (_ISDIGIT(**s))
        i = i * 10 + *((*s)++) - '0';

    return i;
}

uint32_t debug_vsnprintf(char *buf, uint32_t size, const char *fmt, va_list args)
{
#ifdef PRINTF_LONGLONG
    unsigned long long num;
#else
    uint32_t num;
#endif /* PRINTF_LONGLONG */
    int32_t i, len;
    char *str, *end, c;
    const char *s;

    uint8_t base;        /* the base of number */
    uint8_t flags;       /* flags to print number */
    uint8_t qualifier;   /* 'h', 'l', or 'L' for integer fields */
    int32_t field_width; /* width of output field */

#ifdef PRINTF_PRECISION
    int precision; /* min. # of digits for integers and max for a string */
#endif             /* PRINTF_PRECISION */

    str = buf;
    end = buf + size;

    /* Make sure end is always >= buf */
    if (end < buf) {
        end = ((char *)-1);
        size = (uint32_t)(end - buf);
    }

    for (; *fmt; ++fmt) {
        if (*fmt != '%') {
            if (str < end)
                *str = *fmt;
            ++str;
            continue;
        }

        /* process flags */
        flags = 0;

        while (1) {
            /* skips the first '%' also */
            ++fmt;
            if (*fmt == '-')
                flags |= LEFT;
            else if (*fmt == '+')
                flags |= PLUS;
            else if (*fmt == ' ')
                flags |= SPACE;
            else if (*fmt == '#')
                flags |= SPECIAL;
            else if (*fmt == '0')
                flags |= ZEROPAD;
            else
                break;
        }

        /* get field width */
        field_width = -1;
        if (_ISDIGIT(*fmt))
            field_width = skip_atoi(&fmt);
        else if (*fmt == '*') {
            ++fmt;
            /* it's the next argument */
            field_width = va_arg(args, int);
            if (field_width < 0) {
                field_width = -field_width;
                flags |= LEFT;
            }
        }

#ifdef PRINTF_PRECISION
        /* get the precision */
        precision = -1;
        if (*fmt == '.') {
            ++fmt;
            if (_ISDIGIT(*fmt))
                precision = skip_atoi(&fmt);
            else if (*fmt == '*') {
                ++fmt;
                /* it's the next argument */
                precision = va_arg(args, int);
            }
            if (precision < 0)
                precision = 0;
        }
#endif /* PRINTF_PRECISION */
        /* get the conversion qualifier */
        qualifier = 0;
#ifdef PRINTF_LONGLONG
        if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L')
#else
        if (*fmt == 'h' || *fmt == 'l')
#endif /* PRINTF_LONGLONG */
        {
            qualifier = *fmt;
            ++fmt;
#ifdef PRINTF_LONGLONG
            if (qualifier == 'l' && *fmt == 'l') {
                qualifier = 'L';
                ++fmt;
            }
#endif /* PRINTF_LONGLONG */
        }

        /* the default base */
        base = 10;

        switch (*fmt) {
        case 'c':
            if (!(flags & LEFT)) {
                while (--field_width > 0) {
                    if (str < end)
                        *str = ' ';
                    ++str;
                }
            }

            /* get character */
            c = (uint8_t)va_arg(args, int);
            if (str < end)
                *str = c;
            ++str;

            /* put width */
            while (--field_width > 0) {
                if (str < end)
                    *str = ' ';
                ++str;
            }
            continue;

        case 's':
            s = va_arg(args, char *);
            if (!s)
                s = "(NULL)";

            for (len = 0; (len != field_width) && (s[len] != '\0'); len++)
                ;
#ifdef PRINTF_PRECISION
            if (precision > 0 && len > precision)
                len = precision;
#endif /* PRINTF_PRECISION */

            if (!(flags & LEFT)) {
                while (len < field_width--) {
                    if (str < end)
                        *str = ' ';
                    ++str;
                }
            }

            for (i = 0; i < len; ++i) {
                if (str < end)
                    *str = *s;
                ++str;
                ++s;
            }

            while (len < field_width--) {
                if (str < end)
                    *str = ' ';
                ++str;
            }
            continue;

        case 'p':
            if (field_width == -1) {
                field_width = sizeof(void *) << 1;
                flags |= ZEROPAD;
            }
#ifdef PRINTF_PRECISION
            str = print_number(str, end, (long)va_arg(args, void *), 16, field_width, precision, flags);
#else
            str = print_number(str, end, (long)va_arg(args, void *), 16, field_width, flags);
#endif /* PRINTF_PRECISION */
            continue;

        case '%':
            if (str < end)
                *str = '%';
            ++str;
            continue;

        /* integer number formats - set up the flags and "break" */
        case 'o':
            base = 8;
            break;

        case 'X':
            flags |= LARGE;
            // FALLTHROUGH
        case 'x':
            base = 16;
            break;

        case 'd':
        case 'i':
            flags |= SIGN;
            // FALLTHROUGH
        case 'u':
            break;

        default:
            if (str < end)
                *str = '%';
            ++str;

            if (*fmt) {
                if (str < end)
                    *str = *fmt;
                ++str;
            } else {
                --fmt;
            }
            continue;
        }

#ifdef PRINTF_LONGLONG
        if (qualifier == 'L')
            num = va_arg(args, long long);
        else if (qualifier == 'l')
#else
        if (qualifier == 'l')
#endif /* PRINTF_LONGLONG */
        {
            num = va_arg(args, uint32_t);
            if (flags & SIGN)
                num = (uint32_t)(int32_t)num;
        } else if (qualifier == 'h') {
            num = (uint16_t)va_arg(args, int32_t);
            if (flags & SIGN)
                num = (uint16_t)num;
        } else {
            num = va_arg(args, uint32_t);
            if (flags & SIGN)
                num = (uint32_t)(int32_t)num;
        }
#ifdef PRINTF_PRECISION
        str = print_number(str, end, (long)num, base, field_width, precision, flags);
#else
        str = print_number(str, end, num, base, field_width, flags);
#endif /* PRINTF_PRECISION */
    }

    if (size > 0) {
        if (str < end)
            *str = '\0';
        else {
            end[-1] = '\0';
        }
    }

    return (uint32_t)(str - buf);
}

/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
uint32_t debug_printf(const char *fmt, ...)
{
    va_list args;
    uint32_t length;
    static uint8_t log_buf[LOG_SIZE];
    if (!sysreg_is_log_enable()) {
        return 0;
    }

    va_start(args, fmt);
    length = debug_vsnprintf((char *)log_buf, sizeof(log_buf) - 1, fmt, args);
    if (length > LOG_SIZE - 1)
        length = LOG_SIZE - 1;
    debug_uart_puts(log_buf, length);
    va_end(args);

    return length;
}

void debug_flush(void)
{
    uart_flush();
}
/**
 *
 */
