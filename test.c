#include <stdio.h>

int main() {
    int uf = 0x800000;
   int EMask = 0x7ffff;
  int MMask = 0x7f80000;
  if((uf&EMask) == EMask||uf==0 || uf == 0x80000000)
  {
    return uf;
  }
  else if ((uf&EMask) == 0)
  {
    int M = uf & MMask;
    M = M << 1;
    printf("%x %x", uf, M);
    return (uf & ~MMask) + (M & MMask) + (M < 0);
  }
  return uf +1;
}