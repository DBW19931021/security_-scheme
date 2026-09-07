//#include <stdio.h>

#include "../crypto_include/crypto_common/utility.h"


#ifdef UTILITY_PRINT_BUF
//#include "xil_printf.h"


void print_buf_U8(const uint8_t *buf, uint32_t byteLen, char *name)
{
    uint32_t i;

    if(NULL != buf)
    {
        (void)printf("\r\n %s: %p\r\n  ",name, buf); //fflush(stdout);
        for(i=0U; i<byteLen; i++)
        {
            //if(i%16 ==0 && i>0)
            //    (void)printf("\r\n");
            //(void)printf("%02x", buf[byteLen-1-i]);
            (void)printf("%02x", buf[i]);
        }

        (void)printf("\r\n");
    }
}

void print_buf_U32(const uint32_t *buf, uint32_t wordLen, char *name)
{
    uint32_t i;

    if(NULL != buf)
    {
        (void)printf("\r\n %s: %p\r\n",name, buf);//fflush(stdout);
        for(i=0U; i<wordLen; i++)
        {
            //if(i%16 ==0 && i>0)
            //    (void)printf("\r\n");
            //(void)printf("%08x", buf[wordLen-1-i]);
            (void)printf("%08x", (unsigned int)buf[i]);//fflush(stdout);
        }

        (void)printf("\r\n");//fflush(stdout);
    }
}

void print_BN_buf_U32(const uint32_t *buf, uint32_t wordLen, char *name)
{
    uint32_t i;

    if(NULL != buf)
    {
        (void)printf("\r\n %p %s: ", buf, name);//fflush(stdout);
        for(i=0U; i<wordLen; i++)
        {
            //if(i%16 ==0 && i>0)
            //    (void)printf("\r\n");
            (void)printf("%08x", (unsigned int)buf[wordLen-1U-i]);
        }
        (void)printf("\r\n");//fflush(stdout);
    }
}
#endif
