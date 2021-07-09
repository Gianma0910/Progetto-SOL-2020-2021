#include <stdbool.h>

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

#ifndef PROGETTO_MY_TREE_H
#define PROGETTO_MY_TREE_H

TreeNode* makeEmpty(TreeNode* root);
Tree* init_tree(unsigned int size);
Tree* tree_insert(Tree* tree, char* key, void* value);
void* tree_find(Tree* tree, char* key);
Tree* tree_delete(Tree* tree, char* key);
Tree* tree_update_value(Tree* tree, char* key, void* new_value);
bool tree_isEmpty(Tree* tree);
void* tree_get_value(Tree* tree, char* key);
void tree_iterate(Tree* tree, void (*function)(char*, void*, bool*, void*), void* args);
void tree_iterateN(Tree* tree, void (*function)(char*, void*, bool*, void*), void* args, int n);
#endif //PROGETTO_MY_TREE_H
