#include "fid.h"

void __attribute__((weak)) fid_panic_callback(void)
{
    return;
}
#if FID_LEVEL == FID_LEVEL_NONE

void fid_panic(void)
{
    fid_panic_callback();
    while (1)
        ;
}

void fid_set_u32(fid_u32_t *x, uint32_t v)
{
    *x = v;
}

uint32_t fid_get_u32(const fid_u32_t *x)
{
    return *x;
}

void fid_inc_u32(fid_u32_t *x, uint32_t v)
{
    *x += v;
}

#else
static void fid_u32_validate(const fid_u32_t *x)
{
    if (x->val != (x->mask ^ FID_MASK_XOR)) {
        fid_panic();
    }
}

void fid_panic_loop(void)
{
    __asm volatile("j fid_panic_loop");
    __asm volatile("j fid_panic_loop");
    __asm volatile("j fid_panic_loop");
    __asm volatile("j fid_panic_loop");
    __asm volatile("j fid_panic_loop");
    __asm volatile("j fid_panic_loop");
    __asm volatile("j fid_panic_loop");
    __asm volatile("j fid_panic_loop");
    __asm volatile("j fid_panic_loop");
}

void fid_panic(void)
{
    fid_panic_callback();
    fid_panic_loop();
}

void fid_set_u32(fid_u32_t *x, uint32_t v)
{
    x->val = v;
    x->mask = v ^ FID_MASK_XOR;
}

uint32_t fid_get_u32(const fid_u32_t *x)
{
    fid_u32_validate(x);
    return x->val;
}

void fid_inc_u32(fid_u32_t *x, uint32_t v)
{
    x->val += v;
    x->mask = x->val ^ FID_MASK_XOR;
    fid_u32_validate(x);
}
#endif

#if FID_LEVEL == FID_LEVEL_HIGH
uint32_t fid_delay(void)
{
    uint8_t delay;
    uint32_t foo = 0;
    volatile uint32_t ret = 0;

    fid_delay_get_random((uint8_t *)&delay, sizeof(delay));

    for (volatile uint32_t i = 0; i < delay; i++) {
        foo++;
    }

    ret = (foo == (uint32_t)delay);

    /* ret is volatile so if it is the return value then the function cannot be
     * optimized
     */
    return ret;
}
#else
uint32_t fid_delay(void)
{
    return 1U;
}
#endif