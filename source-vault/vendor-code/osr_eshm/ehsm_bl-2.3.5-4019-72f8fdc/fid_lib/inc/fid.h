
#ifndef _FID_H_
#define _FID_H_

#include <stdint.h>

#ifndef FID_LEVEL
#error "FID_LEVEL is not defined!"
#endif

// 抗注入攻击等级
#define FID_LEVEL_NONE 0 // 不开启抗注入攻击
#define FID_LEVEL_LOW  1 // 低等级抗注入攻击（开启"控制流完整性"、"全局故障循环"、"双变量"）
#define FID_LEVEL_HIGH 2 // 高等级抗注入攻击，在低等级基础上开启"随机延时""

#if FID_LEVEL == FID_LEVEL_NONE
typedef unsigned int fid_u32_t;

#define FID_U32_VAL(x)   (x)
#define FID_EQ(x, y)     ((x) == (y))
#define FID_NOT_EQ(x, y) ((x) != (y))
#define FID_SET(x, y)    (x = (y))

#else

#define FID_MASK_XOR 0xdeadbeef

typedef struct {
    uint32_t val;
    uint32_t mask;
} fid_u32_t;

#define FID_U32_VAL(x)                         \
    {                                          \
        .val = (x), .mask = (x) ^ FID_MASK_XOR \
    }

// 安全的相等判断，判断结果为真时进行的操作应当是关键操作
#define FID_EQ(x, y)     (fid_delay() && ((x) == (y)) && fid_delay() && !((y) != (x)))

// 安全的不等判断，判断结果为真时的操作应当时非关键操作或者错误处理，为假时进行的操作应当是关键操作，清注意！
#define FID_NOT_EQ(x, y) (!fid_delay() || ((x) != (y)) || !fid_delay() || !((y) == (x)))

// 安全赋值操作
#define FID_SET(x, y)                  \
    x = (y);                           \
    if (fid_delay() && ((x) != (y))) { \
        fid_panic();                   \
    }
#endif

void fid_set_u32(fid_u32_t *x, uint32_t v);
//下面的操作函数会在内部检查mask值
uint32_t fid_get_u32(const fid_u32_t *x);
void fid_inc_u32(fid_u32_t *x, uint32_t v);

// 供用户在异常状态调用的panic全局入口，这个函数会先调用`fid_panic_callback`
void fid_panic(void);

// 如果用户想要处理panic（例如记录、上报），则实现此函数，否则无需实现
void fid_panic_callback(void);

// 供用户使用的delay接口
uint32_t fid_delay(void);

#if FID_LEVEL == FID_LEVEL_HIGH

// 供`fid_delay`调用的随机数获取函数，由用户实现以提供安全的熵源
// 注：开启随机延时能力时才需要实现
void fid_delay_get_random(uint8_t *buf, uint32_t size);
#endif

#endif