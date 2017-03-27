.globl main
.LC0:
        .string "%ld is a prime factor\n"
factor:
        pushq   %rbp
        movq    %rsp, %rbp
        subq    $32, %rsp
        movq    %rdi, -24(%rbp)
        movq    -24(%rbp), %rax
        sarq    %rax
        movq    %rax, -8(%rbp)
.L3:
        movq    -24(%rbp), %rax
        cqto
        idivq   -8(%rbp)
        movq    %rdx, %rax
        testq   %rax, %rax
        je      .L2
        subq    $1, -8(%rbp)
        jmp     .L3
.L2:
        cmpq    $1, -8(%rbp)
        je      .L4
        movq    -8(%rbp), %rax
        movq    %rax, %rdi
        call    factor
        movq    -24(%rbp), %rax
        cqto
        idivq   -8(%rbp)
        movq    %rax, %rdi
        call    factor
        jmp     .L6
.L4:
        movq    -24(%rbp), %rax
        movq    %rax, %rsi
        movl    $.LC0, %edi
        movl    $0, %eax
        call    printf
.L6:
        nop
        leave
        ret
main:
        pushq   %rbp
        movq    %rsp, %rbp
        movl    $1836311903, %edi
        call    factor
        movl    $0, %eax
        popq    %rbp
        ret
