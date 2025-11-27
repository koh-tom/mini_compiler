.intel_syntax noprefix
.global main
main:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    call ret5
    push rax
    call ret3
    push rax
    pop rdi
    pop rax
    sub rax, rdi
    push rax
    pop rax
    mov rsp, rbp
    pop rbp
    ret
    pop rax
    pop rax
    pop rax
    mov rsp, rbp
    pop rbp
    ret
.section .note.GNU-stack,"",@progbits
