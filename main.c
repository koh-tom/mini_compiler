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

    // 各関数のコード生成
    for (int i = 0; code[i]; i++) {
        gen_function(code[i]);
    }

    printf(".section .note.GNU-stack,\"\",@progbits\n");
    return 0;
}
