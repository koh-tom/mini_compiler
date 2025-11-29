#include <stdio.h>

int ret3() {
    return 3;
}

int ret5() {
    return 5;
}

int add2(int x, int y) {
    return x + y;
}

int sub(int x, int y) {
    return x - y;
}

int add6(int a, int b, int c, int d, int e, int f) {
    return a + b + c + d + e + f;
}

int alloc4(int **p, int a, int b, int c, int d) {
    static int arr[4];
    arr[0] = a;
    arr[1] = b;
    arr[2] = c;
    arr[3] = d;
    *p = arr;
    return 0;
}
