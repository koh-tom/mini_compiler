.intel_syntax noprefix
.global main
main:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    push 0
    pop rax
    mov rax, rbp
    sub rax, 8
    push rax
    push 1
    push 2
    push 4
    push 8
    pop r8
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    call alloc4
    push rax
    pop rax
    mov rax, rbp
    sub rax, 8
    push rax
    pop rax
    mov rax, [rax]
    push rax
    push 1
    pop rdi
    pop rax
    imul rdi, 4
    add rax, rdi
    push rax
    pop rax
    mov rax, [rax]
    push rax
    pop rax
    mov rsp, rbp
    pop rbp
    ret
    pop rax
    pop rax
    mov rsp, rbp
    pop rbp
    ret
.section .note.GNU-stack,"",@progbits
