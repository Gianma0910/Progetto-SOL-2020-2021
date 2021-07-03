#include "../lib/list2.h"

typedef struct TreeNode2{
    char* pathname;
    list2 *pid_clients;
    struct TreeNode2* left;
    struct TreeNode2* right;
}treeNode2;

#if !defined(MY_TREE2.H)
#define MY_TREE2.H

TreeNode2* makeEmpty(TreeNode2* root);
TreeNode2* insertFileAndPid(TreeNode2* root, char* pathname, char* pid_client);
TreeNode2* findFileAndPid(TreeNode2* root, char* pathname, char* pid_client);
TreeNode2* deleteFile(TreeNode2* root, char* pathname);
TreeNode2* findFileAndDeletePid(TreeNode2* root, char* pathname, char* pid_client);
#endif
