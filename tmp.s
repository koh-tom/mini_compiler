.intel_syntax noprefix
.global main
main:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    push 0
    pop rax
    push 10
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
