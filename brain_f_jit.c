#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define STACK_MAX_SIZE 10000

typedef void (*CodeFunction)();
typedef struct
{
    int items[STACK_MAX_SIZE];
    int top;
} Stack;

void initStack(Stack *stack)
{
    stack->top = -1;
}

void push(Stack *stack, int item)
{
    stack->items[++stack->top] = item;
}

void pop(Stack *stack)
{
    stack->top--;
}

short line_array[300000] = {};
int line_index = 0;

void generate_header(FILE *out)
{
    fprintf(out, "section .bss\n");
    fprintf(out, "    mem_size equ 30000\n");
    fprintf(out, "    mem resw mem_size\n\n");
    fprintf(out, "section .text\n");
    fprintf(out, "    global _start\n\n");
    fprintf(out, "_start:\n");
    fprintf(out, "    lea rsi, [mem]\n");
}

void generate_footer(FILE *out)
{
    fprintf(out, "\n    ; Exit program\n");
    fprintf(out, "    mov rax, 60         ; sys_exit\n");
    fprintf(out, "    mov rdi, 0\n");
    fprintf(out, "    syscall\n");
}

int execute_command(const char *command, const char *error_message)
{
    int ret = system(command);
    if (ret == -1)
    {
        perror(error_message);
    }

    return ret;
}

int execute_machine_code(const char *filename)
{
    int file = open(filename, O_RDONLY);
    if (file == -1)
    {
        perror("Error opening file");
        return 1;
    }

    struct stat st;
    if (fstat(file, &st) == -1)
    {
        perror("Error getting file size");
        return 1;
    }

    void *code = mmap(NULL, st.st_size,
                      PROT_READ | PROT_WRITE | PROT_EXEC,
                      MAP_PRIVATE, file, 0);

    close(file);

    if (code == MAP_FAILED)
    {
        perror("Error mapping memory");
        return 1;
    }

    CodeFunction func = (CodeFunction)code;
    func();

    munmap(code, st.st_size);
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE *file_in;
    FILE *file_out;
    char *file_out_name = "out.asm";
    const char *filename = argv[1];
    int ch, loop_position = 0, label_count = 0;

    Stack stack;
    initStack(&stack);

    file_in = fopen(filename, "r");
    if (!file_in)
    {
        printf("Error opening file for reading");
        return 1;
    }

    file_out = fopen(file_out_name, "w");
    if (!file_out)
    {
        printf("Error opening file for writing");
        return 1;
    }

    generate_header(file_out);

    while ((ch = fgetc(file_in)) != EOF)
    {
        switch (ch)
        {
        case '>':
            fprintf(file_out, "    add rsi, 2\n");
            break;
        case '<':
            fprintf(file_out, "    sub rsi, 2\n");
            break;
        case '+':
            fprintf(file_out, "    inc word [rsi]\n");
            break;
        case '-':
            fprintf(file_out, "    dec word [rsi]\n");
            break;
        case '.':
            fprintf(file_out, "    ; Print character\n");
            fprintf(file_out, "    mov rax, 1          ; sys_write\n");
            fprintf(file_out, "    mov rdi, 1          ; stdout\n");
            fprintf(file_out, "    mov rdx, 1          ; length\n");
            fprintf(file_out, "    push rsi\n");
            fprintf(file_out, "    mov rsi, rsi        ; character address\n");
            fprintf(file_out, "    syscall\n");
            fprintf(file_out, "    pop rsi\n");
            break;
        case ',':
            fprintf(file_out, "    ; Read character\n");
            fprintf(file_out, "    mov rax, 0          ; sys_read\n");
            fprintf(file_out, "    mov rdi, 0          ; stdin\n");
            fprintf(file_out, "    push rsi\n");
            fprintf(file_out, "    mov rsi, rsi        ; buffer address\n");
            fprintf(file_out, "    mov rdx, 1          ; length\n");
            fprintf(file_out, "    syscall\n");
            fprintf(file_out, "    pop rsi\n");
            break;
        case '[':
            push(&stack, loop_position);
            fprintf(file_out, "loop_%d:\n", loop_position);
            label_count++;
            break;
        case ']':
            label_count--;
            fprintf(file_out, "    cmp word [rsi], 0\n");
            fprintf(file_out, "    jne loop_%d\n", stack.items[stack.top]);
            fprintf(file_out, "end_loop_%d:\n", stack.items[stack.top]);
            pop(&stack);
            break;
        }
        loop_position++;
    }

    generate_footer(file_out);

    fclose(file_in);
    fclose(file_out);

    char command[1024];

    execute_command("nasm -f elf64 out.asm -o out.o", "nasm failed");
    execute_command("ld out.o -o out", "ld failed");

    execute_machine_code("./out");
    execute_command("rm out.o out.asm", "Failed to remove files");

    if (label_count != 0)
    {
        printf("Error: unmatched brackets\n");
        return 1;
    }

    return 0;
}