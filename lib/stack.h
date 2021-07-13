#include "my_tree.h"
#include <stdbool.h>

#ifndef STACK_H
#define STACK_H

typedef struct Stack_node{
    TreeNode* node;
    struct Stack_node* next;
}Stack_node;

void push_node(Stack_node** top_ref, TreeNode* node);
TreeNode* pop_node(Stack_node** top_ref);
bool stack_empty(Stack_node* top);

#endif
