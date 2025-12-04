#include "codegen.h"

#include "util.h"

#include <stdio.h>
#include <string.h>

// x86-64 ABIの引数レジスタ
static char *argreg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

// 文字列リテラルのコード生成
void gen_string_literals() {
    if (!string_literals) {
        return;
    }
    printf(".data\n");
    for (StringLiteral *str = string_literals; str; str = str->next) {
        printf(".LC%d:\n", str->label_num);
        printf("    .string \"");
        for (int i = 0; i < str->len; i++) {
            printf("%c", str->str[i]);
        }
        printf("\"\n");
    }
}

// グローバル変数のコード生成
void gen_globals() {
    if (!globals) {
        return;
    }
    printf(".data\n");
    for (GVar *var = globals; var; var = var->next) {
        printf(".globl %.*s\n", var->len, var->name);
        printf("%.*s:\n", var->len, var->name);
        printf("    .zero %d\n", size_of(var->ty));
    }
    printf(".text\n");
}

// 関数定義のコード生成
void gen_function(Node *fn) {
    // 関数ラベル
    if (strncmp(fn->funcname, "main", 4) == 0 && fn->funcname_len == 4) {
        printf(".global main\n");
    }
    printf("%.*s:\n", fn->funcname_len, fn->funcname);

    // プロローグ
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");

    // ローカル変数用のスタック確保
    int stack_size = fn->locals ? fn->locals->offset : 0;
    if (stack_size == 0) {
        stack_size = 8;
    } else {
        stack_size = (stack_size + 15) / 16 * 16;
    }
    printf("    sub rsp, %d\n", stack_size);

    // 引数をスタックにコピー
    int i = 0;
    for (Node *param = fn->params; param; param = param->next) {
        printf("    mov [rbp-%d], %s\n", param->offset, argreg[i++]);
    }

    // 関数本体
    gen(fn->body);
    printf("    pop rax\n");

    // エピローグ
    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");
}

void gen_lvar(Node *node) {
    if (node->kind == ND_DEREF) {
        gen(node->lhs);
        return;
    }
    if (node->kind != ND_LVAR) {
        error("代入の左辺値が変数ではありません。\n");
    }
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", node->offset);
    printf("    push rax\n");
}

void load(Type *ty) {
    if (ty->kind == TY_ARRAY) {
        return;
    }
    printf("    pop rax\n");
    if (size_of(ty) == 1) {
        // char型: 1バイト読み込み、符号拡張
        printf("    movsx rax, byte ptr [rax]\n");
    } else if (size_of(ty) == 4) {
        // int型: 4バイト読み込み、符号拡張
        printf("    mov eax, [rax]\n");
        printf("    cdqe\n");
    } else {
        // ポインタ型: 8バイト読み込み
        printf("    mov rax, [rax]\n");
    }
    printf("    push rax\n");
}

void store(Type *ty) {
    printf("    pop rdi\n");
    printf("    pop rax\n");
    if (size_of(ty) == 1) {
        // char型: 1バイト書き込み
        printf("    mov [rax], dil\n");
    } else if (size_of(ty) == 4) {
        // int型: 4バイト書き込み
        printf("    mov [rax], edi\n");
    } else {
        // ポインタ型: 8バイト書き込み
        printf("    mov [rax], rdi\n");
    }
    printf("    push rdi\n");
}

void gen(Node *node) {
    switch (node->kind) {
    case ND_NUM:
        printf("    push %d\n", node->val);
        return;
    case ND_LVAR:
        gen_lvar(node);
        load(node->ty);
        return;
    case ND_GVAR:
        // グローバル変数のアドレスを取得 (RIP相対)
        printf("    lea rax, %.*s[rip]\n", node->gvar_name_len, node->gvar_name);
        printf("    push rax\n");
        load(node->ty);
        return;
    case ND_ASSIGN:
        if (node->lhs->kind == ND_GVAR) {
            // グローバル変数への代入
            printf("    lea rax, %.*s[rip]\n", node->lhs->gvar_name_len, node->lhs->gvar_name);
            printf("    push rax\n");
        } else {
            // ローカル変数への代入
            gen_lvar(node->lhs);
        }
        gen(node->rhs);
        store(node->lhs->ty);
        return;
    case ND_RETURN:
        gen(node->lhs);
        printf("    pop rax\n");
        printf("    mov rsp, rbp\n");
        printf("    pop rbp\n");
        printf("    ret\n");
        return;
    case ND_FUNCALL: {
        // 引数を評価してスタックにプッシュ
        int nargs = 0;
        for (Node *arg = node->args; arg; arg = arg->next) {
            gen(arg);
            nargs++;
        }

        // 引数をレジスタにポップ（逆順）
        for (int i = nargs - 1; i >= 0; i--) {
            printf("    pop %s\n", argreg[i]);
        }

        // RSPを16バイトに丸め込み
        int alignment = (nargs % 2 == 0) ? 8 : 0;
        if (alignment > 0) {
            printf("    sub rsp, %d\n", alignment);
        }

        printf("    mov al, 0\n");
        printf("    call %.*s\n", node->funcname_len, node->funcname);

        if (alignment > 0) {
            printf("    add rsp, %d\n", alignment);
        }

        printf("    push rax\n");
        return;
    }
    case ND_BLOCK:
        for (Node *n = node->block; n; n = n->next) {
            gen(n);
            printf("    pop rax\n");
        }
        return;
    case ND_IF: {
        int label = label_count++;
        gen(node->cond);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lelse%d\n", label);
        gen(node->then);
        printf("    jmp .Lend%d\n", label);
        printf(".Lelse%d:\n", label);
        if (node->els) {
            gen(node->els);
        }
        printf(".Lend%d:\n", label);
        return;
    }
    case ND_FOR: {
        int label = label_count++;
        if (node->init) {
            gen(node->init);
        }
        printf(".Lbegin%d:\n", label);
        if (node->cond) {
            gen(node->cond);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je .Lend%d\n", label);
        }
        gen(node->then);
        if (node->inc) {
            gen(node->inc);
        }
        printf("    jmp .Lbegin%d\n", label);
        printf(".Lend%d:\n", label);
        return;
    }
    case ND_WHILE: {
        int label = label_count++;
        printf(".Lbegin%d:\n", label);
        gen(node->cond);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lend%d\n", label);
        gen(node->then);
        printf("    jmp .Lbegin%d\n", label);
        printf(".Lend%d:\n", label);
        return;
    }
    case ND_ADDR:
        gen_lvar(node->lhs);
        return;
    case ND_DEREF:
        gen(node->lhs);
        load(node->ty);
        return;
    case ND_PTR_ADD:
        gen(node->lhs); // ポインタ
        gen(node->rhs); // 整数
        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    imul rdi, %d\n", size_of(node->lhs->ty->ptr_to));
        printf("    add rax, rdi\n");
        printf("    push rax\n");
        return;
    case ND_PTR_SUB:
        gen(node->lhs); // ポインタ
        gen(node->rhs); // 整数
        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    imul rdi, %d\n", size_of(node->lhs->ty->ptr_to));
        printf("    sub rax, rdi\n");
        printf("    push rax\n");
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->kind) {
    case ND_ADD:
        printf("    add rax, rdi\n");
        break;
    case ND_SUB:
        printf("    sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("    imul rax, rdi\n");
        break;
    case ND_DIV:
        printf("    cqo\n");
        printf("    idiv rdi\n");
        break;
    case ND_EQ:
        printf("    cmp rax, rdi\n");
        printf("    sete al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_NE:
        printf("    cmp rax, rdi\n");
        printf("    setne al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_LT:
        printf("    cmp rax, rdi\n");
        printf("    setl al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_LE:
        printf("    cmp rax, rdi\n");
        printf("    setle al\n");
        printf("    movzb rax, al\n");
        break;
    }

    printf("    push rax\n");
}
