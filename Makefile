pushscript: pushscript.c
	gcc pushscript.c -o push -Wall


out:
	nasm out.asm -f elf64
	nasm stdlib.asm -f elf64
	ld out.o stdlib.o