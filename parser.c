#include "parser.h"

#include "tokenizer.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    node->ty = new_type(TY_INT, NULL);
    return node;
}

LVar *find_lvar(Token *tok) {
    for (LVar *var = locals; var; var = var->next) {
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
            return var;
        }
    }
    return NULL;
}

GVar *find_gvar(Token *tok) {
    for (GVar *var = globals; var; var = var->next) {
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
            return var;
        }
    }
    return NULL;
}

// 構造体のメンバ検索
Member *find_member(Type *ty, Token *tok) {
    for (Member *mem = ty->members; mem; mem = mem->next) {
        if (mem->len == tok->len && !memcmp(mem->name, tok->str, tok->len)) {
            return mem;
        }
    }
    return NULL;
}

// 構造体のタグ
typedef struct Tag Tag;
struct Tag {
    Tag *next;
    char *name;
    int len;
    Type *ty;
};

Tag *structs;

// typedef
typedef struct Typedef Typedef;
struct Typedef {
    Typedef *next;
    char *name;
    int len;
    Type *ty;
};

Typedef *td_list;

Type *find_typedef(Token *tok) {
    for (Typedef *td = td_list; td; td = td->next) {
        if (td->len == tok->len && !memcmp(tok->str, td->name, td->len)) {
            return td->ty;
        }
    }
    return NULL;
}

Tag *find_tag(Token *tok) {
    for (Tag *tag = structs; tag; tag = tag->next) {
        if (tag->len == tok->len && !memcmp(tok->str, tag->name, tag->len)) {
            return tag;
        }
    }
    return NULL;
}

// enum
typedef struct Enumerator Enumerator;
struct Enumerator {
    Enumerator *next;
    char *name;
    int len;
    int val;
};

Enumerator *enumerators;

Enumerator *find_enumerator(Token *tok) {
    for (Enumerator *e = enumerators; e; e = e->next) {
        if (e->len == tok->len && !memcmp(tok->str, e->name, e->len)) {
            return e;
        }
    }
    return NULL;
}

typedef struct EnumTag EnumTag;
struct EnumTag {
    EnumTag *next;
    char *name;
    int len;
    Type *ty;
};

EnumTag *enum_tags;

EnumTag *find_enum_tag(Token *tok) {
    for (EnumTag *tag = enum_tags; tag; tag = tag->next) {
        if (tag->len == tok->len && !memcmp(tok->str, tag->name, tag->len)) {
            return tag;
        }
    }
    return NULL;
}

bool is_typename() {
    if (token->kind == TK_INT || token->kind == TK_CHAR || token->kind == TK_STRUCT || token->kind == TK_VOID || token->kind == TK_ENUM) {
        return true;
    }
    if (token->kind == TK_IDENT) {
        return find_typedef(token) != NULL;
    }
    return false;
}

void program() {
    globals = NULL;
    td_list = NULL; // typedefリスト初期化
    int i = 0;
    while (!at_eof()) {
        if (consume_keyword(TK_TYPEDEF)) {
            Type *base_ty = expect_type();
            Token *tok = consume_ident();
            if (!tok) error("識別子がありません");

            // 配列の処理
            Type head = {};
            Type *cur = &head;
            while (consume("[")) {
                int len = expect_number();
                expect("]");
                cur->ptr_to = calloc(1, sizeof(Type));
                cur->ptr_to->kind = TY_ARRAY;
                cur->ptr_to->array_size = len;
                cur = cur->ptr_to;
            }
            cur->ptr_to = base_ty;
            Type *ty = head.ptr_to ? head.ptr_to : base_ty;

            expect(";");

            Typedef *td = calloc(1, sizeof(Typedef));
            td->name = tok->str;
            td->len = tok->len;
            td->ty = ty;
            td->next = td_list;
            td_list = td;
            continue;
        }

        // 型を読む
        Type *base_ty = expect_type();

        // 識別子を読む
        Token *tok = consume_ident();
        if (!tok) {
            // 識別子がない場合、struct A { ... }; や enum E { ... }; のようなタグ定義のみの可能性がある
            if ((base_ty->kind == TY_STRUCT || base_ty->kind == TY_ENUM) && consume(";")) {
                continue;
            }
            error("識別子がありません");
        }

        // 次のトークンが '(' なら関数定義、そうでなければグローバル変数定義
        if (consume("(")) {
            // 関数定義
            locals = NULL;

            // 引数リストをパース
            Node head = {};
            Node *cur = &head;
            while (!consume(")")) {
                if (cur != &head) {
                    expect(",");
                }

                Type *ty = expect_type();

                Token *param = consume_ident();
                if (!param) {
                    error("引数名がありません");
                }

                // 引数をローカル変数として登録
                LVar *lvar = calloc(1, sizeof(LVar));
                lvar->next = locals;
                lvar->name = param->str;
                lvar->len = param->len;
                lvar->offset = locals ? locals->offset + 8 : 8;
                lvar->ty = ty;
                lvar->is_local = true;
                locals = lvar;

                // 引数ノードを作成
                Node *node = calloc(1, sizeof(Node));
                node->kind = ND_LVAR;
                node->offset = lvar->offset;
                node->ty = ty;
                cur->next = node;
                cur = cur->next;
            }

            // 関数本体をパース
            Node *body = stmt();

            // 関数定義ノードを作成
            Node *fn = calloc(1, sizeof(Node));
            fn->kind = ND_FUNCDEF;
            fn->funcname = tok->str;
            fn->funcname_len = tok->len;
            fn->params = head.next;
            fn->body = body;
            fn->locals = locals;
            fn->ty = base_ty;

            code[i++] = fn;
        } else {
            // グローバル変数定義
            // 配列の処理
            Type head = {};
            Type *cur = &head;
            while (consume("[")) {
                int len = expect_number();
                expect("]");
                cur->ptr_to = calloc(1, sizeof(Type));
                cur->ptr_to->kind = TY_ARRAY;
                cur->ptr_to->array_size = len;
                cur = cur->ptr_to;
            }
            cur->ptr_to = base_ty;
            Type *ty = head.ptr_to ? head.ptr_to : base_ty;

            expect(";");

            // グローバル変数をリストに追加
            GVar *gvar = calloc(1, sizeof(GVar));
            gvar->next = globals;
            gvar->name = tok->str;
            gvar->len = tok->len;
            gvar->ty = ty;
            globals = gvar;
        }
    }
    code[i] = NULL;
}

Type *new_type(TypeKind kind, Type *ptr_to) {
    Type *ty = calloc(1, sizeof(Type));
    ty->kind = kind;
    ty->ptr_to = ptr_to;
    return ty;
}

int size_of(Type *ty) {
    if (ty->kind == TY_INT) {
        return 4;
    }
    if (ty->kind == TY_PTR) {
        return 8;
    }
    if (ty->kind == TY_ARRAY) {
        return size_of(ty->ptr_to) * ty->array_size;
    }
    if (ty->kind == TY_CHAR) {
        return 1;
    }
    if (ty->kind == TY_STRUCT) {
        return ty->size;
    }
    if (ty->kind == TY_ENUM) {
        return 4;
    }
    if (ty->kind == TY_VOID) {
        return 1; // dummy
    }
    return 8;
}


void enum_members(Type *ty) {
    int val = 0;
    while (!consume("}")) {
        Token *tok = consume_ident();
        if (!tok) error("列挙型メンバ名がありません");

        if (consume("=")) {
            val = expect_number();
        }

        Enumerator *e = calloc(1, sizeof(Enumerator));
        e->name = tok->str;
        e->len = tok->len;
        e->val = val++;
        e->next = enumerators;
        enumerators = e;

        if (consume(",")) {
            if (peek("}")) {
                // trailing comma
            }
        }
    }
}

void struct_members(Type *ty) {
    Member head = {};
    Member *cur = &head;
    int offset = 0;

    while (!consume("}")) {
        Type *basety = expect_type();
        Token *tok = consume_ident();
        if (!tok) error("メンバ名がありません");

        Type head = {};
        Type *cur_ty = &head;
        while (consume("[")) {
            int len = expect_number();
            expect("]");
            cur_ty->ptr_to = calloc(1, sizeof(Type));
            cur_ty->ptr_to->kind = TY_ARRAY;
            cur_ty->ptr_to->array_size = len;
            cur_ty = cur_ty->ptr_to;
        }
        cur_ty->ptr_to = basety;
        Type *type = head.ptr_to ? head.ptr_to : basety;

        expect(";");

        Member *mem = calloc(1, sizeof(Member));
        mem->name = tok->str;
        mem->len = tok->len;
        mem->ty = type;
        mem->offset = offset;
        offset += size_of(type);
        
        cur->next = mem;
        cur = cur->next;
    }
    ty->members = head.next;
    ty->size = offset;
}

Type *expect_type() {
    Type *ty;
    if (consume_keyword(TK_CHAR)) {
        ty = new_type(TY_CHAR, NULL);
    } else if (consume_keyword(TK_INT)) {
        ty = new_type(TY_INT, NULL);
    } else if (consume_keyword(TK_VOID)) {
        ty = new_type(TY_VOID, NULL);
    } else if (consume_keyword(TK_STRUCT)) {
        Token *tag_tok = consume_ident();
        if (tag_tok) {
             Tag *tag = find_tag(tag_tok);
             if (tag) {
                 ty = tag->ty;
             } else {
                 ty = calloc(1, sizeof(Type));
                 ty->kind = TY_STRUCT;
                 
                 Tag *t = calloc(1, sizeof(Tag));
                 t->name = tag_tok->str;
                 t->len = tag_tok->len;
                 t->ty = ty;
                 t->next = structs;
                 structs = t;
             }
             
             if (consume("{")) {
                 struct_members(ty);
             }
        } else {
            ty = calloc(1, sizeof(Type));
            ty->kind = TY_STRUCT;
            expect("{");
            struct_members(ty);
        }
    } else if (consume_keyword(TK_ENUM)) {
        Token *tag_tok = consume_ident();
        if (tag_tok) {
            EnumTag *tag = find_enum_tag(tag_tok);
            if (tag) {
                ty = tag->ty;
            } else {
                ty = calloc(1, sizeof(Type));
                ty->kind = TY_ENUM;

                EnumTag *t = calloc(1, sizeof(EnumTag));
                t->name = tag_tok->str;
                t->len = tag_tok->len;
                t->ty = ty;
                t->next = enum_tags;
                enum_tags = t;
            }

            if (consume("{")) {
                enum_members(ty);
            }
        } else {
            ty = calloc(1, sizeof(Type));
            ty->kind = TY_ENUM;
            expect("{");
            enum_members(ty);
        }
    } else {
        Token *tok = consume_ident();
        if (tok) {
            ty = find_typedef(tok);
            if (!ty) {
                error_at(tok->str, "型名ではありません");
            }
        } else {
            error("型ではありません");
        }
    }
    while (consume("*")) {
        ty = new_type(TY_PTR, ty);
    }
    return ty;
}

// 関数定義をパース: name(param1, param2, ...) { body }
Node *function() {
    // 関数ごとにローカル変数をリセット
    locals = NULL;

    // 1. 戻り値の型(int)を読む
    Type *ret_ty = expect_type();

    // 2. 関数名を読む
    Token *tok = consume_ident();
    if (!tok) {
        error("関数名がありません");
    }

    // 2. 引数リストをパース
    expect("(");
    Node head = {};
    Node *cur = &head;
    while (!consume(")")) {
        if (cur != &head) {
            expect(",");
        }

        Type *ty = expect_type();

        Token *param = consume_ident();
        if (!param) {
            error("引数名がありません");
        }

        // 引数をローカル変数として登録
        LVar *lvar = calloc(1, sizeof(LVar));
        lvar->next = locals;
        lvar->name = param->str;
        lvar->len = param->len;
        lvar->offset = locals ? locals->offset + 8 : 8;
        lvar->ty = ty;
        locals = lvar;

        // 引数ノードを作成
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;
        node->offset = lvar->offset;
        node->ty = ty;
        cur->next = node;
        cur = cur->next;
    }

    // 3. 関数本体をパース
    Node *body = stmt();

    // 4. 関数定義ノードを作成
    Node *fn = calloc(1, sizeof(Node));
    fn->kind = ND_FUNCDEF;
    fn->funcname = tok->str;
    fn->funcname_len = tok->len;
    fn->params = head.next;
    fn->body = body;
    fn->locals = locals; // この関数のローカル変数を保存
    fn->ty = ret_ty;     // 戻り値の型を保存

    return fn;
}

Node *stmt() {
    Node *node;
    if (consume("{")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;

        Node head = {};
        Node *cur = &head;
        while (!consume("}")) {
            cur->next = stmt();
            cur = cur->next;
        }
        node->block = head.next;
        return node;
    } else if (consume_keyword(TK_FOR)) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_FOR;
        expect("(");
        if (!consume(";")) {
            node->init = expr();
            expect(";");
        }
        if (!consume(";")) {
            node->cond = expr();
            expect(";");
        }
        if (!consume(")")) {
            node->inc = expr();
            expect(")");
        }
        node->then = stmt();
        return node;
    } else if (consume_keyword(TK_WHILE)) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        expect("(");
        node->cond = expr();
        expect(")");
        node->then = stmt();
        return node;
    } else if (consume_keyword(TK_IF)) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        expect("(");
        node->cond = expr();
        expect(")");
        node->then = stmt();
        if (consume_keyword(TK_ELSE)) {
            node->els = stmt();
        } else {
            node->els = NULL;
        }
        return node;
    } else if (consume_keyword(TK_RETURN)) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
    } else if (is_typename()) {
        // 変数宣言: int x; または char x; または struct A x; など
        Type *ty = expect_type();

        Token *tok = consume_ident();
        if (!tok) {
            error("変数名がありません");
        }

        Type head = {};
        Type *cur = &head;
        while (consume("[")) {
            int len = expect_number();
            expect("]");
            cur->ptr_to = calloc(1, sizeof(Type));
            cur->ptr_to->kind = TY_ARRAY;
            cur->ptr_to->array_size = len;
            cur = cur->ptr_to;
        }
        cur->ptr_to = ty;
        ty = head.ptr_to ? head.ptr_to : ty;

        // 変数が既に定義されていないか確認
        if (find_lvar(tok)) {
            error("変数が既に定義されています");
        }

        LVar *lvar = calloc(1, sizeof(LVar));
        lvar->next = locals;
        lvar->name = tok->str;
        lvar->len = tok->len;
        int size = size_of(ty);
        lvar->offset = locals ? locals->offset + size : size;
        lvar->ty = ty;
        locals = lvar;

        if (consume("=")) {
            // 初期化式がある場合
            // 変数への代入として処理する
            Node *lhs = calloc(1, sizeof(Node));
            lhs->kind = ND_LVAR;
            lhs->offset = lvar->offset;
            lhs->ty = lvar->ty;

            Node *rhs = expr();

            node = calloc(1, sizeof(Node));
            node->kind = ND_ASSIGN;
            node->lhs = lhs;
            node->rhs = rhs;
        } else {
            // 初期化式がない場合
            // ND_NUM(0)を返して実質スキップ
            node = calloc(1, sizeof(Node));
            node->kind = ND_NUM;
            node->val = 0;
        }
        expect(";");
        return node;
    } else if (consume(";")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_NUM;
        node->val = 0; // 捨てるだけ
        return node;
    } else {
        node = expr();
    }

    expect(";");
    return node;
}

Node *expr() {
    return assign();
}

Node *assign() {
    Node *node = equality();
    if (consume("=")) {
        node = new_node(ND_ASSIGN, node, assign());
    }
    return node;
}

Node *equality() {
    Node *node = relational();

    for (;;) {
        if (consume("==")) {
            node = new_node(ND_EQ, node, relational());
        } else if (consume("!=")) {
            node = new_node(ND_NE, node, relational());
        } else {
            return node;
        }
    }
}

Node *relational() {
    Node *node = add();

    for (;;) {
        if (consume("<")) {
            node = new_node(ND_LT, node, add());
        } else if (consume("<=")) {
            node = new_node(ND_LE, node, add());
        } else if (consume(">")) {
            node = new_node(ND_LT, add(), node);
        } else if (consume(">=")) {
            node = new_node(ND_LE, add(), node);
        } else {
            return node;
        }
    }
}

Node *add() {
    Node *node = mul();

    for (;;) {
        if (consume("+")) {
            Node *rhs = mul();
            // ポインタ + 整数 の場合
            if (node->ty && (node->ty->kind == TY_PTR || node->ty->kind == TY_ARRAY)) {
                Node *ptr_add = new_node(ND_PTR_ADD, node, rhs);
                ptr_add->ty = node->ty;
                if (node->ty->kind == TY_ARRAY) {
                    ptr_add->ty = new_type(TY_PTR, node->ty->ptr_to);
                }
                node = ptr_add;
            }
            // 整数 + ポインタ の場合
            else if (rhs->ty && (rhs->ty->kind == TY_PTR || rhs->ty->kind == TY_ARRAY)) {
                Node *ptr_add = new_node(ND_PTR_ADD, rhs, node);
                ptr_add->ty = rhs->ty;
                if (rhs->ty->kind == TY_ARRAY) {
                    ptr_add->ty = new_type(TY_PTR, rhs->ty->ptr_to);
                }
                node = ptr_add;
            }
            // 通常の加算
            else {
                node = new_node(ND_ADD, node, rhs);
                node->ty = new_type(TY_INT, NULL);
            }
        } else if (consume("-")) {
            Node *rhs = mul();
            // ポインタ - 整数 の場合
            if (node->ty && (node->ty->kind == TY_PTR || node->ty->kind == TY_ARRAY)) {
                Node *ptr_sub = new_node(ND_PTR_SUB, node, rhs);
                ptr_sub->ty = node->ty;
                if (node->ty->kind == TY_ARRAY) {
                    ptr_sub->ty = new_type(TY_PTR, node->ty->ptr_to);
                }
                node = ptr_sub;
            }
            // 通常の減算
            else {
                node = new_node(ND_SUB, node, rhs);
                node->ty = new_type(TY_INT, NULL);
            }
        } else {
            return node;
        }
    }
}

Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume("*")) {
            node = new_node(ND_MUL, node, unary());
        } else if (consume("/")) {
            node = new_node(ND_DIV, node, unary());
        } else {
            return node;
        }
    }
}

Node *unary() {
    if (consume("+")) {
        return unary();
    } else if (consume("-")) {
        return new_node(ND_SUB, new_node_num(0), unary());
    } else if (consume_keyword(TK_SIZEOF)) {
        if (consume("(")) {
            if (is_typename()) {
                Type *ty = expect_type();
                expect(")");
                return new_node_num(size_of(ty));
            }
            Node *node = expr();
            expect(")");
            return new_node_num(size_of(node->ty));
        }
        Node *node = unary();
        return new_node_num(size_of(node->ty));
    } else if (consume("&")) {
        Node *operand = unary();
        Node *addr = new_node(ND_ADDR, operand, NULL);
        if (operand->ty) {
            addr->ty = new_type(TY_PTR, operand->ty);
        }
        return addr;
    } else if (consume("*")) {
        Node *operand = unary();
        Node *deref = new_node(ND_DEREF, operand, NULL);
        if (operand->ty && (operand->ty->kind == TY_PTR || operand->ty->kind == TY_ARRAY)) {
            deref->ty = operand->ty->ptr_to;
        }
        return deref;
    }
    return primary();
}

Node *primary() {
    Node *node;
    Token *tok = consume_ident();
    if (tok) {
        if (consume("(")) {
            node = calloc(1, sizeof(Node));
            node->kind = ND_FUNCALL;
            node->funcname = tok->str;
            node->funcname_len = tok->len;

            // 引数をパース
            Node head = {};
            Node *cur = &head;
            while (!consume(")")) {
                if (cur != &head) {
                    expect(",");
                }
                cur->next = expr();
                cur = cur->next;
            }
            node->args = head.next;
        } else {
            // 変数参照: まずローカル変数を探し、なければグローバル変数を探す
            LVar *lvar = find_lvar(tok);
            if (lvar) {
                node = calloc(1, sizeof(Node));
                node->kind = ND_LVAR;
                node->offset = lvar->offset;
                node->ty = lvar->ty;
            } else {
                GVar *gvar = find_gvar(tok);
                if (gvar) {
                    node = calloc(1, sizeof(Node));
                    node->kind = ND_GVAR;
                    node->gvar_name = gvar->name;
                    node->gvar_name_len = gvar->len;
                    node->ty = gvar->ty;
                } else {
                    Enumerator *en = find_enumerator(tok);
                    if (en) {
                        node = new_node_num(en->val);
                    } else {
                        error_at(tok->str, "未定義の変数です");
                    }
                }
            }
        }
    } else if (consume("(")) {
        node = expr();
        expect(")");
    } else if (token->kind == TK_STRING) {
        // 文字列リテラル
        Token *tok = token;
        token = token->next;

        // 文字列リテラルをリストに追加
        static int label_counter = 0;
        StringLiteral *str_lit = calloc(1, sizeof(StringLiteral));
        str_lit->str = tok->str;
        str_lit->len = tok->len;
        str_lit->label_num = label_counter++;
        str_lit->next = string_literals;
        string_literals = str_lit;

        // 文字列リテラルのラベルのアドレスを返すノードを作成
        node = calloc(1, sizeof(Node));
        node->kind = ND_GVAR;
        char *label = calloc(1, 10);
        sprintf(label, ".LC%d", str_lit->label_num);
        node->gvar_name = label;
        node->gvar_name_len = strlen(label);
        node->ty = new_type(TY_ARRAY, new_type(TY_CHAR, NULL));
        node->ty->array_size = str_lit->len + 1;
    } else {
        node = new_node_num(expect_number());
    }



    while (consume("[")) {
        Node *idx = expr();
        expect("]");
        Type *ptr_ty = node->ty;
        Node *base = node;
        Node *index = idx;

        // node がポインタ/配列でない場合、idx がポインタ/配列かチェック
        // 2[a] のような場合に対応
        if (ptr_ty && ptr_ty->kind != TY_PTR && ptr_ty->kind != TY_ARRAY) {
            if (idx->ty && (idx->ty->kind == TY_PTR || idx->ty->kind == TY_ARRAY)) {
                // 入れ替え: 2[a] -> a[2]
                base = idx;
                index = node;
                ptr_ty = idx->ty;
            }
        }

        if (!ptr_ty) {
            error("添字演算子の対象が不正です");
        }

        if (ptr_ty->kind == TY_ARRAY) {
            ptr_ty = new_type(TY_PTR, ptr_ty->ptr_to);
        } else if (ptr_ty->kind != TY_PTR) {
            error("添字演算子はポインタまたは配列にのみ適用できます");
        }

        // ポインタ演算
        Node *add = new_node(ND_PTR_ADD, base, index);
        add->ty = ptr_ty;
        node = new_node(ND_DEREF, add, NULL);
        node->ty = ptr_ty->ptr_to;
    }
    
    while (true) {
        if (consume(".")) {
            Token *tok = consume_ident();
            if (!tok) error("メンバ名がありません");
            
            Member *mem = find_member(node->ty, tok);
            if (!mem) error("そのようなメンバはありません: %.*s", tok->len, tok->str);
            
            Node *member_node = new_node(ND_MEMBER, node, NULL);
            member_node->member = mem;
            member_node->ty = mem->ty;
            node = member_node;
            continue;
        }
        if (consume("->")) {
             Token *tok = consume_ident();
             if (!tok) error("メンバ名がありません");
             
             if (node->ty->kind != TY_PTR || node->ty->ptr_to->kind != TY_STRUCT) {
                  error("ポインタではありません");
             }
             
             node = new_node(ND_DEREF, node, NULL);
             node->ty = node->lhs->ty->ptr_to;

             Member *mem = find_member(node->ty, tok);
             if (!mem) error("そのようなメンバはありません: %.*s", tok->len, tok->str);

             Node *member_node = new_node(ND_MEMBER, node, NULL);
             member_node->member = mem;
             member_node->ty = mem->ty;
             node = member_node;
             continue;
        }
        break;
    }

    return node;
}
