#include <stdio.h>

int  sra(int  x,  int  k) { 
//I•  Perform  shift  logically•/ 
    int  xsrl  = (unsigned)  x>>k;
    int w = sizeof(int) << 3;
    printf("%d\n", w);
    int mark = -1;
    mark = mark << w - k - 1;
    printf("%x\n",mark);
    mark = ~((mark & xsrl)-1);
    printf("%x\n",mark);
    return xsrl | mark;
}
int  any_odd_one(unsigned  x)
{
    x &= 0x55555555;
    return !(!x);
}
int odd_ones(unsigned x)
{
    x = (x >> 16) ^ x;
    x = (x >> 8) ^ x;
    x = (x >> 4) ^ x;
    x = (x >> 2) ^ x;
    x = (x >> 1) ^ x;
    return x&1;
}
int main(void)
{
    printf("%x",odd_ones(0xf131));
}