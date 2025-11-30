.intel_syntax noprefix
.global main
main:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    push 0
    pop rax
    mov rax, rbp
    sub rax, 12
    push rax
    push 0
    pop rdi
    pop rax
    imul rdi, 4
    add rax, rdi
    push rax
    push 0
    pop rdi
    pop rax
    mov [rax], edi
    push rdi
    pop rax
    mov rax, rbp
    sub rax, 12
    push rax
    push 1
    pop rdi
    pop rax
    imul rdi, 4
    add rax, rdi
    push rax
    push 1
    pop rdi
    pop rax
    mov [rax], edi
    push rdi
    pop rax
    mov rax, rbp
    sub rax, 12
    push rax
    push 2
    pop rdi
    pop rax
    imul rdi, 4
    add rax, rdi
    push rax
    push 2
    pop rdi
    pop rax
    mov [rax], edi
    push rdi
    pop rax
    mov rax, rbp
    sub rax, 12
    push rax
    push 2
    pop rdi
    pop rax
    imul rdi, 4
    add rax, rdi
    push rax
    pop rax
    mov eax, [rax]
    cdqe
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
