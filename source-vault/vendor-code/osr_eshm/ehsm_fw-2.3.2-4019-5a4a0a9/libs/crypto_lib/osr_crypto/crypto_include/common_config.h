#ifndef COMMON_CONFIG_H
#define COMMON_CONFIG_H


#include <stdint.h>    //including definitions of int32_t, uint32_t, etc.

#include <string.h>    //including definition of NULL




/************************************************************************************
 ******************************    common config    *********************************
 ************************************************************************************/

//print buffer functions
#define UTILITY_PRINT_BUF

#ifdef UTILITY_PRINT_BUF
#include <stdio.h>
#endif

#define UTILITY_SEC


//only one of the following two macro could be enabled
#define MEM_VOLATILE
//#define MEM_VOLATILE volatile


//#define CONFIG_UNIT_TEST

//C keyword static, keep it by default, or define it empty just for some testing.
#ifdef CONFIG_UNIT_TEST
#define FLAG_STATIC
#else
#define FLAG_STATIC   static   //default
#endif

#define CRYPTO_BASE_ADDR 0

#define SUPPORT_STATIC_ANALYSIS
#endif
