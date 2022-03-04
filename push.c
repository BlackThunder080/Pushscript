#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define OUT *

typedef struct OP
{
	int64_t opcode;
	int64_t arg;
} OP;

OP CreateOP(int64_t opcode, int64_t arg)
{
	OP op;
	op.opcode = opcode;
	op.arg = arg;
	return op;
}

void Compile(OP[], int64_t);
void CompileFile(char[]);


typedef enum OPERATIONS
{
	OP_PUSH,
	
	OP_ADD,
	OP_MINUS,

	OP_EQUAL,

	OP_DUMP,

	OP_EOF,
} OPS;

int main(int argc, char *argv[])
{
	CompileFile("test.push");

	system("nasm out.asm -f elf64");
	system("ld out.o");
	system("./a.out");

	return 0;
}

void Compile(OP *program, int64_t program_length)
{
	FILE* out = fopen("out.asm", "w");

	fprintf(out, "[BITS 64]\n");
	fprintf(out, "segment .text\n");
	fprintf(out, "	global _start\n\n");
		
	fprintf(out, "dump:\n");
	fprintf(out, "	push rax\n");
	fprintf(out, "	push rbx\n");
	fprintf(out, "	push    rbp\n");
	fprintf(out, "	mov     rbp, rsp\n");
	fprintf(out, "	sub     rsp, 64\n");
	fprintf(out, "	mov     QWORD [rbp-56], rdi\n");
	fprintf(out, "	mov     QWORD [rbp-8], 1\n");
	fprintf(out, "	mov     eax, 32\n");
	fprintf(out, "	sub     rax, QWORD [rbp-8]\n");
	fprintf(out, "	mov     BYTE [rbp-48+rax], 10\n");
	fprintf(out, "	.L2:\n");
	fprintf(out, "		mov     rcx, QWORD [rbp-56]\n");
	fprintf(out, "		mov  rdx, -3689348814741910323\n");
	fprintf(out, "		mov     rax, rcx\n");
	fprintf(out, "		mul     rdx\n");
	fprintf(out, "		shr     rdx, 3\n");
	fprintf(out, "		mov     rax, rdx\n");
	fprintf(out, "		sal     rax, 2\n");
	fprintf(out, "		add     rax, rdx\n");
	fprintf(out, "		add     rax, rax\n");
	fprintf(out, "		sub     rcx, rax\n");
	fprintf(out, "		mov     rdx, rcx\n");
	fprintf(out, "		mov     eax, edx\n");
	fprintf(out, "		lea     edx, [rax+48]\n");
	fprintf(out, "		mov     eax, 31\n");
	fprintf(out, "		sub     rax, QWORD [rbp-8]\n");
	fprintf(out, "		mov     BYTE [rbp-48+rax], dl\n");
	fprintf(out, "		add     QWORD [rbp-8], 1\n");
	fprintf(out, "		mov     rax, QWORD [rbp-56]\n");
	fprintf(out, "		mov  rdx, -3689348814741910323\n");
	fprintf(out, "		mul     rdx\n");
	fprintf(out, "		mov     rax, rdx\n");
	fprintf(out, "		shr     rax, 3\n");
	fprintf(out, "		mov     QWORD [rbp-56], rax\n");
	fprintf(out, "		cmp     QWORD [rbp-56], 0\n");
	fprintf(out, "		jne     .L2\n");
	fprintf(out, "		mov     eax, 32\n");
	fprintf(out, "		sub     rax, QWORD [rbp-8]\n");
	fprintf(out, "		lea     rdx, [rbp-48]\n");
	fprintf(out, "		lea     rcx, [rdx+rax]\n");
	fprintf(out, "		mov     rax, QWORD [rbp-8]\n");
	fprintf(out, "		mov     rdx, rax\n");
	fprintf(out, "		mov     rsi, rcx\n");
	fprintf(out, "		mov     edi, 1\n");
	fprintf(out, "		mov     rax, 1\n");
	fprintf(out, "		syscall\n");
	fprintf(out, "		nop\n");
	fprintf(out, "		leave\n");
	fprintf(out, "		pop rbx\n");
	fprintf(out, "		pop rax\n");
	fprintf(out, "		ret\n\n");
	fprintf(out, "_start:\n");

	int64_t ip = 0;
	while (ip < program_length)
	{
		OP op = program[ip];

		if (op.opcode == OP_EOF)
			break;

		switch (op.opcode)
		{
			case OP_PUSH:;
				fprintf(out, "	push %ld\n", op.arg);
				break;
			case OP_ADD:;
				fprintf(out, "	pop rbx\n");
				fprintf(out, "	pop rax\n");
				fprintf(out, "	add rax, rbx\n");
				fprintf(out, "	push rax\n");
				break;
			case OP_MINUS:;
				fprintf(out, "	pop rbx\n");
				fprintf(out, "	pop rax\n");
				fprintf(out, "	sub rax, rbx\n");
				fprintf(out, "	push rax\n");
				break;

			case OP_EQUAL:;
				fprintf(out, "	pop rax\n");
				fprintf(out, "	pop rbx\n");
				fprintf(out, "	mov rcx, 0\n");
				fprintf(out, "	mov rdx, 1\n");
				fprintf(out, "	cmp rax, rbx\n");
				fprintf(out, "	cmove rcx, rdx\n");
				fprintf(out, "	push rcx\n");
				break;

			case OP_DUMP:;
				fprintf(out, "	pop rdi\n");
				fprintf(out, "	call dump\n");
				break;
			default:;
				fprintf(stderr, "Invalid Instruction: %ld\n", op.opcode);
				exit(1);
		}

		ip++;
	}

	fprintf(out, "	mov rax, 60\n");
	fprintf(out, "	mov rdi, 13\n");
	fprintf(out, "	syscall\n");

	fclose(out);
}


void CompileFile(char in_file[])
{
	FILE* in = fopen(in_file, "r");

	fseek(in, 0, SEEK_END);
	size_t f_size = ftell(in);
	rewind(in);

	char buf[f_size];
	fread(buf, sizeof(char), f_size, in);
	buf[f_size] = '\0';

	OP program[6400];
	int ip = 0;

	char delim[] = " ";
	char *tok = strtok(buf, delim);

	while(tok != NULL)
	{

		if (strcmp(tok, "+") == 0)
		{
			program[ip] = CreateOP(OP_ADD, 0);
		}
		else if (strcmp(tok, "dump") == 0)
		{
			program[ip] = CreateOP(OP_DUMP, 0);
		}
		else if (strspn(tok, "0123456789") == strlen(tok))
		{
			program[ip] = CreateOP(OP_PUSH, atoi(tok));
		}
		else
		{
			fprintf(stderr, "\nUnrecognized Token '%s'\n", tok);
			exit(1);
		}
		ip++;

		tok = strtok(NULL, delim);
	}

	fclose(in);


	Compile(program, ip);
}