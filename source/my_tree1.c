#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include "../lib/my_string.h"
#include "../lib/my_tree1.h"

pthread_mutex_t tree_lock = PTHREAD_MUTEX_INITIALIZER;

TreeNode1* makeEmpty(TreeNode1* root){
    pthread_mutex_lock(&tree_lock);

    if(root != NULL){
        pthread_mutex_unlock(&tree_lock);
        makeEmpty(root->left);
        pthread_mutex_unlock(&tree_lock);
        makeEmpty(root->right);
        free(root);
    }

    pthread_mutex_unlock(&tree_lock);

    return NULL;
}

TreeNode1* insertFile(TreeNode1* root, char* pathname, void* buffer){
    pthread_mutex_lock(&tree_lock);

    if(root == NULL){
        root = malloc(sizeof(TreeNode1));
        strncpy(root->pathname, pathname, str_length(pathname));
        root->buffer = buffer;
        root->size = buffer*1024*1024;
        root->left = NULL;
        root->right = NULL;
    }else if(strncmp(pathname, root->pathname, str_length(pathname)) > 0){
        pthread_mutex_unlock(&tree_lock);
        root->right = insertFile(root->right, pathname);
    }else if(strncmp(pathname, root->pathname, str_length(pathname)) < 0){
        pthread_mutex_unlock(&tree_lock);
        root->left = insertFile(root->left, pathname);
    }

    pthread_mutex_unlock(&tree_lock);

    return root;
}

TreeNode1* findFile(TreeNode1* root, char* pathname){
    pthread_mutex_lock(&tree_lock);

    if(root == NULL){
        pthread_mutex_unlock(&tree_lock);
        return NULL;
    }else if(strncmp(pathname, root->pathname, str_length(pathname)) > 0){
        pthread_mutex_unlock(&tree_lock);
        return findFile(root->right, pathname);
    }else if(strncmp(pathname, root->pathname, str_length(pathname)) < 0){
        pthread_mutex_unlock(&tree_lock);
        return findFile(root->left, pathname);
    }else{
        pthread_mutex_unlock(&tree_lock);
        return root;
    }
}

TreeNode1* findFileMin(TreeNode1* root){
    pthread_mutex_lock(&tree_lock);

    if(root == NULL){
        pthread_mutex_unlock(&tree_lock);
        return NULL;
    }else if(root->left == NULL){
        pthread_mutex_unlock(&tree_lock);
        return root;
    }else{
        pthread_mutex_unlock(&tree_lock);
        return findFileMin(root->left);
    }

}

TreeNode1* findFileMax(TreeNode1* root) {
    pthread_mutex_lock(&tree_lock);

    if (root == NULL) {
        pthread_mutex_unlock(&tree_lock);
        return NULL;
    } else if (root->right == NULL){
        pthread_mutex_unlock(&tree_lock);
        return root;
    }else{
        pthread_mutex_unlock(&tree_lock);
        return findFileMax(root->right);
    }
}

TreeNode1* deleteFile(TreeNode1* root, char* pathname){
    pthread_mutex_lock(&tree_lock);

    TreeNode1* temp;

    if(root == NULL){
        return NULL;
    }else if(strncmp(root->pathname, pathname, str_length(pathname)) < 0){
        pthread_mutex_unlock(&tree_lock);
        root->left = deleteFile(root->left, pathname);
    }else if(strncmp(root->pathname, pathname, str_length(pathname)) > 0){
        pthread_mutex_unlock(&tree_lock);
        root->right = deleteFile(root->right, pathname);
    }else if(root->left && root->right){
        pthread_mutex_unlock(&tree_lock);
        temp = findFileMin(root->right);
        strncpy(root->pathname, temp->pathname, str_length(temp->pathname));
        root->buffer = temp->buffer;
        root->size = temp->size;
        root->right = deleteFile(root->right, root->pathname);
    }else{
        temp = root;
        if(root->left == NULL)
            root = root->right;
        else if(root->right == NULL)
            root = root->left;
        free(temp);
    }

    pthread_mutex_unlock(&tree_lock);

    return root;
}
