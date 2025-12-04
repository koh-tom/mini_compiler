#ifndef CODEGEN_H
#define CODEGEN_H

#include "parser.h"

// コード生成関数
void gen(Node *node);
void gen_function(Node *fn);
void gen_globals();
void gen_string_literals();

// 左辺値を評価する関数
void gen_lvar(Node *node);

#endif
