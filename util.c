#include "util.h"

// グローバル変数の定義
// 現在着目しているトークン
Token *token;

// 入力文字列
char *user_input;

// パース結果のノード保存
Node *code[100];

// ローカル変数
LVar *locals;

// ラベルカウンタ
int label_count;

// エラー監視用の関数
// printfと同じ引数をとる
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// エラー箇所の報告
void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " "); // pos分だけ空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}
