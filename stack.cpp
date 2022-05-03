#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stack.hpp"


struct Node *top=(struct Node *)mmap(NULL, sizeof(Node), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

void pop()
{
    if (top == NULL)
    {
        printf("\nEMPTY STACK\n");
        return;
    }
    Node *temp = top;
    top = top->next;


    munmap(temp, sizeof(struct Node));

    printf("Node is out\n\n");
}
void push(char *value)
{
    Node *temp;

    temp = (struct Node *)mmap(NULL, sizeof(struct Node), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    for (int i = 0; i < strlen(value); i++)
    {
        temp->data[i] = value[i];
    }
    temp->data[strlen(value)-1]='\n';
    // if (top == NULL)
    // {
    //     temp->next = NULL;
    // }
    // else
    {
        temp->next = top;
    }
    top = temp;
    printf("Node is in\n\n");
}

char *Top()
{
    printf("gg");
    char* ans=(char *)mmap(NULL, sizeof(char), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (top == NULL)
    {
        return NULL;
    }
    
    return top->data;
}
