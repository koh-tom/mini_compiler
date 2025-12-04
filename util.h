#ifndef UTIL_H
#define UTIL_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "tokenizer.h"
#include "parser.h"

// グローバル変数
extern Token *token;
extern char *user_input;
extern char *filename;
extern Node *code[100];
extern LVar *locals;
extern int label_count;
extern GVar *globals;
extern StringLiteral *string_literals;

// エラー処理関数
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

// ファイル読み込み関数
char *read_file(char *path);

#endif
