#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>

// トークンの種類
typedef enum {
    TK_RESERVED, // 記号
    TK_NUM,      // 整数
    TK_EOF,      // 終端
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token {
    TokenKind kind; // トークンの型
    Token *next; // 次のトークン
    int val; // kindがTK_NUMの場合の数値
    char *str; // トークンの文字列
};

// 現在着目しているトークン
Token *token;

// エラー監視用の関数
// printfと同じ引数をとる
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// 次のトークンが正規の記号なら、1つ読み進めて真を返す。
// それ以外なら偽を返す。
bool consume(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op){
        return false;
    }
    token = token->next;
    return true;
}

// 次のトークンが正規の記号なら、1つ読み進める。
// それ以外ならエラーを返す。
void expect(char op){
     if (token->kind != TK_RESERVED || token->str[0] != op){
        error("'%c'ではありません", op);
    }
    token = token->next;
}

// 次のトークンが数値なら、1つ読み進めて数値を返す。
// それ以外ならエラーを返す。
int expect_number(){
    if(token->kind != TK_NUM) {
        error("数ではありません。");
    }
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() {
    return token->kind == TK_EOF;
}

// 新しいトークンを生成する
Token *new_token(TokenKind kind, Token *cur, char *str) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

// 入力も文字列pをトークナイズしてそれを返す。
Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (*p == '+' || *p == '-') {
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }
        error("トークンを解析できません。\n");
    }
    new_token(TK_EOF, cur, p);
    return head.next;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }

    // トークナイズする    
    token = tokenize(argv[1]);

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // 式の最初は数値でなければならない
    printf("    mov rax, %d\n", expect_number());

    // +数or-数の並びを消費
    while (!at_eof()) {
        if (consume('+')) {
            printf("    add rax, %d\n", expect_number());
            continue;
        }
        expect('-');
        printf("    sub rax, %d\n", expect_number());
    }

    printf("    ret\n");
    printf(".section .note.GNU-stack,\"\",@progbits\n");
    return 0;
}