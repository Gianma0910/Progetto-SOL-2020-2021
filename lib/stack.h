#include "my_tree.h"
#include <stdbool.h>

typedef struct Stack_node{
    TreeNode* node;
    struct Stack_node* next;
}Stack_node;

#ifndef PROGETTO_STACK_H
#define PROGETTO_STACK_H

void push(Stack_node** top_ref, TreeNode* node);
TreeNode* pop(Stack_node** top_ref);
bool stack_isEmpty(Stack_node* top);

#endif //PROGETTO_STACK_H
