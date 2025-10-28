#include <stdio.h>
int isTmax(int x) {
  int y = ~(x + 1);
  printf("%x", y);
  return !(x^y);
}
int main()
{
    printf("%x", isTmax(-1));
}