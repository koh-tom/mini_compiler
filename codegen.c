#include "codegen.h"

#include "util.h"

#include <stdio.h>

// x86-64 ABIの引数レジスタ
static char *argreg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

void gen_lvar(Node *node) {
    if (node->kind != ND_LVAR) {
        error("代入の左辺値が変数ではありません。\n");
    }
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", node->offset);
    printf("    push rax\n");
}

void gen(Node *node) {
    switch (node->kind) {
    case ND_NUM:
        printf("    push %d\n", node->val);
        return;
    case ND_LVAR:
        gen_lvar(node);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        return;
    case ND_ASSIGN:
        gen_lvar(node->lhs);
        gen(node->rhs);
        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
        printf("    push rdi\n");
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
