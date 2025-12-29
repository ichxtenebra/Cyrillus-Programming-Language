format ELF64 executable 3
entry _start

include 'messages.inc'

segment readable executable

_start:
    mov eax, 1
    mov edi, eax
    lea rsi, [Cyrillus_banner]
    mov edx, Cyrillus_banner_len
    syscall
    mov eax, 60
    xor edi, edi
    syscall
