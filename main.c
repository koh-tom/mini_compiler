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
    Node *node = expr();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // 抽象構文木を下りながらコードを生成する
    gen(node);

    // スタックトップに式全体の値が残っているのでそれを返り値としてRAXに格納する
    printf("    pop rax\n");
    printf("    ret\n");
    printf(".section .note.GNU-stack,\"\",@progbits\n");
    return 0;
}
