#ifndef PARSER_H
#define PARSER_H

#include "tokenizer.h"
#include <stddef.h>

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
    ND_FUNCDEF, // 関数定義
    ND_ADDR, // &
    ND_DEREF, // *
    ND_PTR_ADD, // ポインタ + 整数
    ND_PTR_SUB, // ポインタ - 整数
    ND_GVAR, // グローバル変数
} NodeKind;

typedef struct LVar LVar;
typedef struct Node Node;

typedef struct Type Type;

typedef enum {
    TY_INT,
    TY_PTR,
    TY_ARRAY,
    TY_CHAR,
} TypeKind;

struct Type {
    TypeKind kind;
    Type *ptr_to;
    size_t array_size; // 配列のサイズ
};

// 抽象構文木のノードの型
struct Node {
    NodeKind kind;    // ノードの型
    Node *lhs;        // 左辺
    Node *rhs;        // 右辺
    Type *ty;         // 型 (int, int*, ...)
    int val;          // kindがND_NUMの場合の数値
    int offset;       // kindがND_LVARの場合のオフセット
    Node *cond;       // 条件式
    Node *then;       // then節
    Node *els;        // else節
    Node *init;       // kindがND_FORの場合の初期化式
    Node *inc;        // kindがND_FORの場合の増分式
    Node *block;      // kindがND_BLOCKの場合の文のリスト
    Node *next;       // 次の文（ブロック内で使用）
    char *funcname;   // kindがND_FUNCALL/ND_FUNCDEFの場合の関数名
    int funcname_len; // 関数名の長さ
    char *gvar_name;  // kindがND_GVARの場合のグローバル変数名
    int gvar_name_len; // グローバル変数名の長さ
    Node *args;       // kindがND_FUNCCALLの場合の引数（リンクリスト）
    Node *params;     // kindがND_FUNCDEFの場合の仮引数（リンクリスト）
    Node *body;       // kindがND_FUNCDEFの場合の関数本体
    LVar *locals;     // kindがND_FUNCDEFの場合のローカル変数リスト
}; 

//ローカル変数の型
struct LVar {
    LVar *next; // 次の変数かNULL
    char *name; // 変数の名前
    int len;    // 名前の長さ
    int offset; // RBPからのオフセット
    Type *ty;   // 型
    bool is_local; // ローカル変数かどうか
};

typedef struct GVar GVar;

//グローバル変数の型
struct GVar {
    GVar *next; // 次の変数かNULL
    char *name; // 変数の名前
    int len;    // 名前の長さ
    Type *ty;   // 型
};

typedef struct StringLiteral StringLiteral;

// 文字列リテラルの型
struct StringLiteral {
    StringLiteral *next; // 次の文字列リテラルかNULL
    char *str;           // 文字列の内容
    int len;             // 文字列の長さ
    int label_num;       // ラベル番号
};

// パーサー関数
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Type *new_type(TypeKind kind, Type *ptr_to);
Type *expect_type();
int size_of(Type *ty);
void program();
Node *function();
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
