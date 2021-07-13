#include <stdbool.h>
#ifndef MY_TREE_H
#define MY_TREE_H

typedef struct TreeNode{
    char* key;
    void *value;
    struct TreeNode* left;
    struct TreeNode* right;
}TreeNode;

typedef struct Tree{
    TreeNode* root;
    unsigned int max_size;
    unsigned int current_size;
}Tree;


void makeEmpty(Tree* tree, void (*delete_value)(void* value));
Tree* init_tree(unsigned int size);
Tree* tree_insert(Tree* tree, char* key, void* value);
void* tree_find(Tree* tree, char* key);
void tree_delete(Tree** tree, char* key, void (*delete_value)(void* value));
Tree* tree_update_value(Tree* tree, char* key, void* new_value);
bool tree_isEmpty(Tree* tree);
void* tree_get_value(Tree* tree, char* key);
void tree_iterate(Tree* tree, void (*function)(char*, void*, bool*, void*), void* args);
void tree_iterateN(Tree* tree, void (*function)(char*, void*, bool*, void*), void* args, int n);
#endif
