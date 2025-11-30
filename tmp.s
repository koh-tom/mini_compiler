.intel_syntax noprefix
.data
.globl y
y:
    .zero 4
.globl x
x:
    .zero 4
.text
.global main
main:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    lea rax, x[rip]
    push rax
    push 3
    pop rdi
    pop rax
    mov [rax], edi
    push rdi
    pop rax
    lea rax, y[rip]
    push rax
    push 4
    pop rdi
    pop rax
    mov [rax], edi
    push rdi
    pop rax
    lea rax, x[rip]
    push rax
    pop rax
    mov eax, [rax]
    cdqe
    push rax
    lea rax, y[rip]
    push rax
    pop rax
    mov eax, [rax]
    cdqe
    push rax
    pop rdi
    pop rax
    add rax, rdi
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
