section .bss
    mem_size equ 30000
    mem resw mem_size
    
    
section .text
global _start

extern _ExitProcess@4

_start:
    xor edi, edi

fill_array:
    mov [mem + edi * 2], di
    inc edi
    cmp edi, mem_size
    jl fill_array

    xor edi, edi

end:
    mov eax, 60
    xor edi, edi
    syscall

