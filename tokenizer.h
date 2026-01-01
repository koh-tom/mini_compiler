#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdbool.h>

// トークンの種類
typedef enum {
    TK_RESERVED, // 記号
    TK_IDENT,    // 識別子
    TK_NUM,      // 整数
    TK_STRING,   // 文字列リテラル
    TK_EOF,      // 終端
    TK_SEMI,     // ;
    TK_RETURN,   // return
    TK_IF,       // if
    TK_ELSE,     // else
    TK_WHILE,    // while
    TK_FOR,      // for
    TK_INT,      // int
    TK_SIZEOF,   // sizeof
    TK_CHAR,     // char
    TK_STRUCT,   // struct
    TK_VOID,     // void
    TK_TYPEDEF,  // typedef
    TK_ENUM,     // enum
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token {
    TokenKind kind; // トークンの型
    Token *next;    // 次のトークン
    int val;        // kindがTK_NUMの場合の数値
    char *str;      // トークンの文字列
    int len;        // トークンの長さ
};

// トークナイザ関数
bool consume(char *op);
bool consume_keyword(TokenKind kind);
Token *consume_ident();
void expect(char *op);
int expect_number();
bool at_eof();
bool peek(char *op);
Token *tokenize();

#endif
