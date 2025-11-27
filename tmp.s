.intel_syntax noprefix
.global main
main:
    push rbp
    mov rbp, rsp
    sub rsp, 0
.Lbegin0:
    push 2
    push 1
    pop rdi
    pop rax
    sub rax, rdi
    push rax
    pop rax
    cmp rax, 0
    je .Lend0
    push 2
    pop rax
    mov rsp, rbp
    pop rbp
    ret
    jmp .Lbegin0
.Lend0:
    pop rax
    push 3
    pop rax
    mov rsp, rbp
    pop rbp
    ret
    pop rax
    mov rsp, rbp
    pop rbp
    ret
.section .note.GNU-stack,"",@progbits
