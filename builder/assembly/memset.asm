; Wienton Dev Corp(memset.asm) — memset_zero implementation (x86_64, System V ABI)
; Clears a block of memory to zero.
; void memset_zero(void* ptr, size_t size);

[bits 64]
[default rel]

section .text

align 16
global memset_zero

memset_zero:
    ; rdi = ptr, rsi = size
    test rsi, rsi          ; <-- ИСПРАВЛЕНО: rsi, не rsr!
    jz .done

    push rax
    push rdx

    ; Check 8-byte alignment
    mov rax, rdi
    and rax, 7
    jz .process_qwords

    ; Byte-align first
.byte_align:
    mov byte [rdi], 0
    inc rdi
    dec rsi
    jz .restore

    mov rax, rdi
    and rax, 7
    jnz .byte_align

.process_qwords:
    ; Now 8-byte aligned
    mov rax, 0

    ; Process 8 bytes at a time
.qword_loop:
    cmp rsi, 8
    jl .tail               ; if rsi < 8, go to tail

    mov [rdi], rax
    add rdi, 8
    sub rsi, 8
    jmp .qword_loop

.tail:
    ; Handle remaining bytes (0–7)
    test rsi, rsi
    jz .restore

.byte_loop:
    mov byte [rdi], 0
    inc rdi
    dec rsi
    jnz .byte_loop

.restore:
    pop rdx
    pop rax

.done:
    ret