.intel_syntax noprefix
.global main
main:
    push rbp
    mov rbp, rsp
    sub rsp, 0
.Lbegin0:
    push 3
    pop rax
    mov rsp, rbp
    pop rbp
    ret
    jmp .Lbegin0
.Lend0:
    pop rax
    mov rsp, rbp
    pop rbp
    ret
.section .note.GNU-stack,"",@progbits
