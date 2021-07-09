#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../lib/stack.h"

void push(Stack_node** top_ref, TreeNode* node){
    Stack_node* new_tnode = (Stack_node *) malloc(sizeof(Stack_node));

    new_tnode->node = node;
    new_tnode->next = (*top_ref);
    (*top_ref) = new_tnode;
}

TreeNode* pop(Stack_node** top_ref){
    TreeNode* res;
    Stack_node* top;

    if(stack_isEmpty(*top_ref)){
        return NULL;
    }else{
        top = *top_ref;
        res = top->node;
        *top_ref = top->next;
        free(top);
        return res;
    }
}

bool stack_isEmpty(Stack_node* top){
    return top == NULL;
}

