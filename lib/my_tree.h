#include <stdbool.h>

typedef struct TreeNode{
    char* key;
    void *value;
    struct TreeNode* left;
    struct TreeNode* right;
}TreeNode;

#ifndef PROGETTO_MY_TREE_H
#define PROGETTO_MY_TREE_H

TreeNode* makeEmpty(TreeNode* root);
TreeNode* insertFile(TreeNode* root, char* key, void* value);
TreeNode* findFile(TreeNode* root, char* key);
TreeNode* deleteFile(TreeNode* root, char* key);
TreeNode* updateValue(TreeNode* root, char* key, void* new_value);
bool tree_isEmpty(TreeNode* root);
void* getValue(TreeNode* root, char* key);

#endif //PROGETTO_MY_TREE_H
