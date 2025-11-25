#include <sys/mman.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

unsigned char main_code[] =
    "\x48\xc7\xc0\x2a\x00\x00\x00\xc3";  // mov rax, 42; ret

int main() {
    size_t size = sizeof(main_code);

    void *buf = mmap(NULL, size,
        PROT_READ | PROT_WRITE | PROT_EXEC,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1, 0);

    memcpy(buf, main_code, size);

    int64_t (*func)() = buf;
    int64_t r = func();

    printf("return = %ld\n", r);
}
