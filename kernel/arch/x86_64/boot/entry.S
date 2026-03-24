.global _start
.extern kernel_main

.section .text.entry
_start:
    lea stack_top(%rip), %rsp
    call kernel_main

.hang:
    hlt
    jmp .hang

.section .bss
.align 16
stack:
    .skip 32768
stack_top:

.section .note.GNU-stack,"",@progbits
