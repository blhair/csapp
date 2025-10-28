#include <stdio.h>

int is_little_endian() {
    unsigned int x = 1;
    char *c = (char*)&x;
    return *c == 1;
}

int main() {
    if (is_little_endian()) {
        printf("Little Endian\n");
    } else {
        printf("Big Endian\n");
    }
    return 0;
}