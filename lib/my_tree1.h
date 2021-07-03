typedef struct TreeNode1{
    char* pathname;
    void *buffer;
    size_t size;
    struct TreeNode1* left;
    struct TreeNode1* right;
}TreeNode1;

#if !defined(MY_TREE1.H)
#define MY_TREE1.H

TreeNode1* makeEmpty(TreeNode1* root);
TreeNode1* insertFile(TreeNode1* root, char* pathname, void* buffer);
TreeNode1* findFile(TreeNode1* root, char* pathname);
TreeNode1* deleteFile(TreeNode1* root, char* pathname);
#endif
