#include <stdio.h>
#include "fid.h"

int main()
{
    fid_u32_t x;
    uint32_t v = 1000;

    fid_set_u32(&x, v);
    fid_inc_u32(&x, 1);
    fid_inc_u32(&x, 2);
    // x.val = 1000;
    v = fid_get_u32(&x);
    printf("v: %d \n", v);

    uint32_t a = 100;
    uint32_t b = 1000;

    if (FID_EQ(a, b)) {
        printf("a b is equal\n");
    } else {
        printf("a b is not equal\n");
    }

    a = 100;
    b = 100;

    if (FID_EQ(a, b)) {
        printf("a b is equal\n");
    } else {
        printf("a b is not equal\n");
    }

    a = 1000;
    printf("b: %d \n", b);
    FID_SET(b, a);
    printf("b: %d \n", b);
}