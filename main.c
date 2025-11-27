#include "codegen.h"
#include "parser.h"
#include "tokenizer.h"
#include "util.h"

#include <stdio.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        error("引数の個数が正しくありません\n");
        return 1;
    }

    // トークナイズする
    user_input = argv[1];
    token = tokenize();
    program();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // プロローグ
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    int stack_size = locals ? locals->offset : 0;
    // スタックサイズを16の倍数に丸める
    if (stack_size == 0) {
        stack_size = 8;
    } else {
        stack_size = (stack_size + 15) / 16 * 16;
    }
    printf("    sub rsp, %d\n", stack_size);

    // 抽象構文木を下りながらコードを生成する
    for (int i = 0; code[i]; i++) {
        gen(code[i]);
        // スタックトップに式全体の値が残っているのでそれを返り値としてRAXに格納する
        printf("    pop rax\n");
    }

    // エピローグ
    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");
    printf(".section .note.GNU-stack,\"\",@progbits\n");
    return 0;
}
