#include <stdio.h>

#define STACK_MAX_SIZE 10000

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

char commands_array[100000] = {};
int commands_index = 0;
int closed_brackets = 0;
int open_brackets = 0;

short line_array[300000] = {};
int line_index = 0;

int main(int argc, char *argv[])
{
    FILE *file;
    // const char *filename = argv[1];
    const char *filename = "hello.bf";
    int ch;

    file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Error opening file: %s\n", filename);
        return 1;
    }

    // save commands to array and count brackets
    while ((ch = fgetc(file)) != EOF)
    {
        commands_array[commands_index++] = ch;
        if (ch == '[')
            open_brackets++;
        else if (ch == ']')
            closed_brackets++;
    }

    if (open_brackets != closed_brackets)
    {
        printf("Error: unmatched brackets\n");
        return 1;
    }

    // execute commands
    Stack stack;
    initStack(&stack);

    for (int i = 0; i < commands_index; i++)
    {
        switch (commands_array[i])
        {
        case '>':
            line_index++;
            break;
        case '<':
            line_index--;
            break;
        case '+':
            line_array[line_index]++;
            break;
        case '-':
            line_array[line_index]--;
            break;
        case '.':
            printf("%c", line_array[line_index]);
            break;
        case ',':
            line_array[line_index] = getchar();
            break;
        case '[':
            push(&stack, i);
            break;
        case ']':
            if (line_array[line_index] != 0)
            {
                i = stack.items[stack.top];
            }
            else
            {
                pop(&stack);
            }
            break;
        }
    }

    fclose(file);
    return 0;
}