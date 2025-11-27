.intel_syntax noprefix
.global main
main:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    push 1
    push 2
    push 3
    push 4
    push 5
    push 6
    push 7
    push 8
    push 9
    pop r9
    pop r8
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    sub rsp, 8
    call add6
    add rsp, 8
    push rax
    push 10
    push 11
    push 12
    push 13
    pop r9
    pop r8
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    sub rsp, 8
    call add6
    add rsp, 8
    push rax
    push 14
    push 15
    push 16
    pop r9
    pop r8
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    sub rsp, 8
    call add6
    add rsp, 8
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
