#include<string>
#include <sys/mman.h>
#define SIZE 1024
using namespace std;

typedef struct Node {
    char data[SIZE];
    struct Node *next;
}Node; 

// typedef struct mystack
// {
//    Node* top;
//    int size; 
// }mystack;



void pop();
void push(char* value);
char* Top();