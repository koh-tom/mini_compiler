.intel_syntax noprefix
.data
.LC0:
    .string "abc"
.text
.global main
main:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    push 8
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
