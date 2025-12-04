.intel_syntax noprefix
.text
.global main
main:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    push 0
    pop rax
    mov rax, rbp
    sub rax, 4
    push rax
    push 1
    pop rdi
    pop rax
    mov [rax], edi
    push rdi
    pop rax
    mov rax, rbp
    sub rax, 4
    push rax
    mov rax, rbp
    sub rax, 4
    push rax
    pop rax
    mov eax, [rax]
    cdqe
    push rax
    push 1
    pop rdi
    pop rax
    add rax, rdi
    push rax
    pop rdi
    pop rax
    mov [rax], edi
    push rdi
    pop rax
    mov rax, rbp
    sub rax, 4
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
