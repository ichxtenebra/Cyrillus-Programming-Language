format ELF64 executable 3
entry _start

include 'messages.inc'

segment readable executable

_start:
    push 1
    pop rdi
    lea rsi, [Cyrillus_banner]
    mov edx, Cyrillus_banner_len
    push 1
    pop rax
    syscall
    
    xor edi, edi
    mov eax, 60
    syscall
