#ifndef PARSER_H
#define PARSER_H

#include "tokenizer.h"

// 抽象構文木のノードの種類
typedef enum {
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_NUM, // 数値
    ND_SEMI, // ;
    ND_EQ,  // ==
    ND_NE,  // !=
    ND_LT,  // <
    ND_LE,  // <=
    ND_ASSIGN, // =
    ND_LVAR, // 変数
    ND_RETURN, // return
    ND_IF, // if
    ND_WHILE, // while
    ND_FOR, // for
    ND_BLOCK, // {}
    ND_FUNCALL, //関数呼出
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの型
struct Node {
    NodeKind kind;    // ノードの型
    Node *lhs;        // 左辺
    Node *rhs;        // 右辺
    int val;          // kindがND_NUMの場合の数値
    int offset;       // kindがND_LVARの場合のオフセット
    Node *cond;       // 条件式
    Node *then;       // then節
    Node *els;        // else節
    Node *init;       // kindがND_FORの場合の初期化式
    Node *inc;        // kindがND_FORの場合の増分式
    Node *block;      // kindがND_BLOCKの場合の文のリスト
    Node *next;       // 次の文（ブロック内で使用）
    char *funcname;   // kindがND_FUNCCALLの場合の関数名
    int funcname_len; // 関数名の長さ
}; 

typedef struct LVar LVar;

//ローカル変数の型
struct LVar {
    LVar *next; // 次の変数かNULL
    char *name; // 変数の名前
    int len;    // 名前の長さ
    int offset; // RBPからのオフセット
};

// パーサー関数
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
void program();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();
LVar *find_lvar(Token *tok);

#endif
