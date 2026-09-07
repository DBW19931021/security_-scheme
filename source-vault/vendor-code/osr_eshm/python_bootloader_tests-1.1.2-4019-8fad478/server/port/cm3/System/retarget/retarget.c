/*
 *-----------------------------------------------------------------------------
 * The confidential and proprietary information contained in this file may
 * only be used by a person authorised under and to the extent permitted
 * by a subsisting licensing agreement from ARM Limited.
 *
 *            (C) COPYRIGHT 2010-2017  ARM Limited or its affiliates.
 *                ALL RIGHTS RESERVED
 *
 * This entire notice must be reproduced on all copies of this file
 * and copies of this file may only be made by a person if such person is
 * permitted to do so under the terms of a subsisting license agreement
 * from ARM Limited.
 *
 *      SVN Information
 *
 *      Checked In          : $Date: 2013-04-10 15:14:20 +0100 (Wed, 10 Apr 2013) $
 *
 *      Revision            : $Revision: 243501 $
 *
 *      Release Information : CM3DesignStart-r0p0-01rel0
 *-----------------------------------------------------------------------------
 */

#if defined(__CC_ARM)
/******************************************************************************/
/* Retarget functions for ARM DS-5 Professional / Keil MDK                    */
/******************************************************************************/

#include <stdio.h>
#include <time.h>

#include "uart_stdout.h"

#pragma import(__use_no_semihosting_swi)

struct __FILE {
    int handle; /* Add whatever you need here */
};
FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE* f)
{
    return (UartPutc(ch));
}

int fgetc(FILE* f)
{
    return (UartPutc(UartGetc()));
}

int ferror(FILE* f)
{
    /* Your implementation of ferror */
    return EOF;
}

void _ttywrch(int ch)
{
    UartPutc(ch);
}

void _sys_exit(int return_code)
{
label:
    goto label; /* endless loop */
}

#elif defined(__IAR_SYSTEMS_ICC__)
/******************************************************************************/
/* Retarget functions for IAR Systems C Compiler for ARM                      */
/******************************************************************************/
#include <yfuns.h>

extern unsigned char UartGetc(void);
extern unsigned char UartPutc(unsigned char my_ch);

size_t __write(int handle, const unsigned char* buffer, size_t size)
{
    size_t nChars = 0;

    for (/*Empty */; size > 0; --size) {
        UartPutc(*buffer++);
        ++nChars;
    }
    return nChars;
}

size_t __read(int handle, unsigned char* buffer, size_t size)
{
    int nChars = 0;

    /* This template only reads from "standard in", for all other file
     * handles it returns failure. */
    if (handle != _LLIO_STDIN) {
        return _LLIO_ERROR;
    }

    for (/* Empty */; size > 0; --size) {
        /* Get char with echo */
        unsigned char c = UartPutc(UartGetc());
        /* Get char without echo */
        // unsigned char c = UartGetc();
        *buffer++ = c;
        ++nChars;
    }
    return nChars;
}
#else

/******************************************************************************/
/* Retarget functions for GNU Tools for ARM Embedded Processors               */
/******************************************************************************/
#include <stdio.h>
// #include <sys/stat.h>

extern unsigned char UartPutc(unsigned char my_ch);

__attribute__((used)) int _write(int fd, char* ptr, int len)
{
    size_t i;
    for (i = 0; i < len; i++) {
        UartPutc(ptr[i]); // call character output function
    }
    return len;
}

#endif
