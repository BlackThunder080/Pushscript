#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define check(condition, msg) if (!(condition)) {fprintf(stderr, "%s\n", msg); exit(1);}

#define OUT *


int StartsWith(const char *a, const char *b)
{
   if(strncmp(a, b, strlen(b)) == 0) return 1;
   return 0;
}


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

void Compile(OP[], int64_t, char[][256], int);
void CompileFile(char[]);


typedef enum OPERATIONS
{
	OP_PUSH,
	OP_PUSH_STRING,
	OP_PUSH_GLOBAL,
	
	OP_ADD,
	OP_MINUS,
	OP_TIMES,
	OP_DIVIDE,

	OP_EQUAL,

	OP_PEEK,
	OP_POKE,

	OP_SYSCALL,
	OP_PUT,
	OP_DUMP,

	OP_EOF,
} OPS;

typedef enum TYPES
{
	INTEGER,
	HEX,
	STRING,
} TYPES;

typedef enum GLOBALS
{
	ARGC,
	ARGV,
	MEM,
} GLOBALS;


int main(int argc, char *argv[])
{
	int run = 0;
	int compiled = 1;

	if (argc < 2)
	{
		fprintf(stderr, "Usage: ./push <file> [-r]\n");
		fprintf(stderr, "	-r		- Run file upon successful compilation\n");
		exit(1);
	}

	for (int i = 2; i < argc; i++)
	{
		if (strcmp(argv[i], "-r") == 0)
			run = 1;
	}

	CompileFile(argv[1]);

	compiled ^= system("nasm out.asm -f elf64");
	compiled ^= system("nasm stdlib.asm -f elf64");
	compiled ^= system("ld out.o stdlib.o");

	if (run && compiled)
		system("./a.out");

	return 0;
}

void Compile(OP *program, int64_t program_length, char strings[][256], int strings_len)
{
	FILE* out = fopen("out.asm", "w");

	fprintf(out, "[BITS 64]\n");
	fprintf(out, "segment .bss\n");
	fprintf(out, "	mem: resq 1024000\n");
	fprintf(out, "	argc: resq 1\n");
	fprintf(out, "	argv: resq 1\n");
	fprintf(out, "segment .text\n");
	fprintf(out, "	extern puts, putc, putd, putx\n");
	fprintf(out, "	global _start\n\n");

	fprintf(out, "_start:\n");
	fprintf(out, "	mov rax, [rsp]\n");
	fprintf(out, "	mov qword [argc], rax\n");
	fprintf(out, "	mov qword [argv], rsp\n");
	fprintf(out, "	add qword [argv], 8\n");

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
			case OP_PUSH_STRING:;
				fprintf(out, "	push str_%ld\n", op.arg + 1);
				break;
			case OP_PUSH_GLOBAL:;
				switch (op.arg)
				{
					case ARGC:;
						fprintf(out, "	push qword [argc]\n");
						break;
					case ARGV:;
						fprintf(out, "	push qword [argv]\n");
						break;
					case MEM:;
						fprintf(out, "	push qword mem\n");
						break;
					default:;
						fprintf(stderr, "Unknown Global Variable `%ld`\n", op.arg);
						exit(1);
				}
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
			case OP_TIMES:;
				fprintf(out, "	pop rax\n");
				fprintf(out, "	pop rbx\n");
				fprintf(out, "	push rax\n");
				fprintf(out, "	mov rax, rbx\n");
				fprintf(out, "	pop rbx\n");
				fprintf(out, "	mul rbx\n");
				fprintf(out, "	push rax\n");
				break;
			case OP_DIVIDE:;
				fprintf(out, "	pop rax\n");
				fprintf(out, "	pop rbx\n");
				fprintf(out, "	push rax\n");
				fprintf(out, "	mov rax, rbx\n");
				fprintf(out, "	pop rbx\n");
				fprintf(out, "	div rbx\n");
				fprintf(out, "	push rbx\n");
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

			case OP_PEEK:;
				fprintf(out, "	pop rax\n");
				fprintf(out, "	push qword [rax]\n");
				break;
			case OP_POKE:;
				fprintf(out, "	pop rax\n");
				fprintf(out, "	pop rbx\n");
				fprintf(out, "	mov [rbx], rax\n");
				break;

			case OP_SYSCALL:;
				char args[][4] = {"rax", "rdi", "rsi", "rdx", "rcx", "r8", "r9"};
				for (int i = 0; i < op.arg; i++)
					fprintf(out, "	pop %s\n", args[op.arg - i - 1]);
				fprintf(out, "	syscall\n");
				break;
			case OP_PUT:;
				fprintf(out, "	pop rdi\n");
				fprintf(out, "	call put%c\n", (char)op.arg);
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
	fprintf(out, "	mov rdi, 0\n");
	fprintf(out, "	syscall\n\n");

	fprintf(out, "segment .data\n");
	for (int i = 0; i < strings_len; i++)
	{
		fprintf(out, "	str_%d: db '%s', 0", i + 1, strings[i]);
	}

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


	for (int i = 0; i < f_size; i++)
	{
		if (buf[i] == '\r')
			buf[i] = '\n';
		if (buf[i] == '\n')
			buf[i] = ' ';
		if (buf[i] == '\t')
			buf[i] = ' ';
	}


	OP program[6400];
	int ip = 0;

	char delim[] = " ";
	char *tok = strtok(buf, delim);

	char strings[1024][256];
	int string_ptr = 0;

	while (tok != NULL)
	{
		if (strcmp(tok, "+") == 0)
			program[ip] = CreateOP(OP_ADD, 0);
		else if (strcmp(tok, "-") == 0)
			program[ip] = CreateOP(OP_MINUS, 0);
		else if (strcmp(tok, "*") == 0)
			program[ip] = CreateOP(OP_TIMES, 0);
		else if (strcmp(tok, "/") == 0)
			program[ip] = CreateOP(OP_DIVIDE, 0);

		else if (strcmp(tok, "==") == 0)
			program[ip] = CreateOP(OP_EQUAL, 0);

		else if (strcmp(tok, "@") == 0)
			program[ip] = CreateOP(OP_PEEK, 0);
		else if (strcmp(tok, "~") == 0)
			program[ip] = CreateOP(OP_POKE, 0);

		else if (StartsWith(tok, "syscall"))
			program[ip] = CreateOP(OP_SYSCALL, atoi(tok + strlen(tok) - 1));
		else if (StartsWith(tok, "put"))
		{
			program[ip] = CreateOP(OP_PUT, tok[3]);
		}
		else if (strcmp(tok, "dump") == 0)
			program[ip] = CreateOP(OP_DUMP, 0);

		else if (strcmp(tok, "argc") == 0)
			program[ip] = CreateOP(OP_PUSH_GLOBAL, ARGC);
		else if (strcmp(tok, "argv") == 0)
			program[ip] = CreateOP(OP_PUSH_GLOBAL, ARGV);
		else if (strcmp(tok, "mem") == 0)
			program[ip] = CreateOP(OP_PUSH_GLOBAL, MEM);

		else if (strspn(tok, "0123456789") == strlen(tok))
			program[ip] = CreateOP(OP_PUSH, atoi(tok));
		else if (tok[0] == '"')
		{
			tok++;
			char str[256] = "";
			do {
				strcat(str, tok);
				strcat(str, " ");
				tok = strtok(NULL, delim);
			} while (tok != NULL && tok[strlen(tok) - 1] != '"');
			strncat(str, tok, strlen(tok) - 1);
			strcpy(strings[string_ptr], str);
			program[ip] = CreateOP(OP_PUSH_STRING, string_ptr++);
		}
		else if (tok[0] == '\'')
		{
			strncpy(tok, tok+1, strchr(tok+1, '\'') - tok+1);
			

			if (tok[0] == '\\')
			{
				if (tok[1] == '\\')
					tok[0] = '\\';
				else if (tok[1] == 'n')
					tok[0] = '\n';
				else
				{
					fprintf(stderr, "Invalid Escape Character '%c'\n", tok[1]);
					exit(1);
				}
				check (strlen(tok) < 4, "Character Literal must be one character long\n")
				tok[1] = '\'';
				tok[2] = '\0';
			}

			check (strlen(tok) == 2 && tok[strlen(tok) - 1] == '\'', "Character Literal must be one character long\n")
			program[ip] = CreateOP(OP_PUSH, tok[0]);
		}

		else
		{
			fprintf(stderr, "Unrecognized Token `%s`\n", tok);
			exit(1);
		}
		ip++;

		tok = strtok(NULL, delim);
	}

	fclose(in);

	Compile(program, ip, strings, string_ptr);
}