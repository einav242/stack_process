#include<string>
#include <sys/mman.h>
#define SIZE 1024
using namespace std;

typedef struct Node {
    char data[SIZE];
    struct Node *next;
}Node; 

