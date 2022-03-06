global puts, putc, putd, putx

puts:
	mov rsi, rdi-1
	.loop:
		inc rsi
		cmp byte [rsi], 0
		jne .loop
	mov rax, 1
	sub rsi, rdi
	mov rdx, rsi
	inc rdx
	mov rsi, rdi
	mov rdi, 1
	syscall
	ret

putc:
	mov [buf], rdi

	mov rax, 1
	mov rdi, 1
	mov rsi, buf
	mov rdx, 1
	syscall

	mov rdi, buf
	mov rsi, 1
	call clear_buf

	ret

putx:
    mov rsi, buf

	mov rcx, 16
	mov rax, rdi
	mov r8, 9

	mov byte [rsi], '0'
	mov byte [rsi+1], 'x'

	.loop:
		push rax
	    mov rdx, 0
		div rcx
		mov rbx, [chars+rdx]
		mov [rsi+r8], bl
		dec r8
		pop rax
		shr rax, 4
		jnz .loop

	mov rax, 1
	mov rdi, 1
	mov rdx, 10
	syscall

	mov rdi, buf
	mov rsi, 256
	call clear_buf

	ret

putd:
    mov rsi, buf

	mov rcx, 10
	mov rax, rdi
	mov r8, 20

	cmp rdi, 0
	jl .negative

	.loop:
	    mov rdx, 0
		div rcx
		mov rbx, [chars+rdx]
		mov [rsi+r8], bl
		dec r8
		cmp rax, 0
		jne .loop
	jmp .print

	.negative:
		mov r9, 0
		sub r9, rax
		mov rax, r9
	.negative_loop:
	    mov rdx, 0
		idiv rcx
		mov rbx, [chars+rdx]
		mov [rsi+r8], bl
		dec r8
		cmp rax, 0
		jne .negative_loop
	mov byte [rsi+r8], '-'

	.print:
		mov rax, 1
		mov rdi, 1
		add rsi, r8
		mov rdx, 21
		syscall

	mov rdi, buf
	mov rsi, 256
	call clear_buf

	ret

clear_buf:
	; RDI = BUF
	; RSI = SIZE
	.loop:
		mov qword [rdi], 0
		inc rdi
		dec rsi
		jnz .loop
	ret

segment .data
	chars: db '0123456789abcdef'
segment .bss
	buf: resq 256