#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include "../lib/my_string.h"
#include "../lib/my_tree2.h"

pthread_mutex_t tree_lock = PTHREAD_MUTEX_INITIALIZER;

TreeNode2* makeEmpty(TreeNode2* root){
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

TreeNode2* insertFileAndPid(TreeNode2* root, char* pathname, char* pid_client){
    pthread_mutex_lock(&tree_lock);

    if(root == NULL){
        root = malloc(sizeof(TreeNode2));
        strncpy(root->pathname, pathname, str_length(pathname));
        list_insert(&(root->pid_clients), pid_client);
        root->left = NULL;
        root->right = NULL;
    }else if(strncmp(pathname, root->pathname, str_length(pathname)) > 0){
        pthread_mutex_unlock(&tree_lock);
        root->right = insertFileAndPid(root->right, pathname, pid_client);
    }else if(strncmp(pathname, root->pathname, str_length(pathname)) < 0){
        pthread_mutex_unlock(&tree_lock);
        root->left = insertFileAndPid(root->left, pathname, pid_client);
    }

    pthread_mutex_unlock(&tree_lock);

    return root;
}

TreeNode2* findFileAndPid(TreeNode2* root, char* pathname, char* pid_client){
    pthread_mutex_lock(&tree_lock);

    if(root == NULL){
        pthread_mutex_unlock(&tree_lock);
        return NULL;
    }else if(strncmp(pathname, root->pathname, str_length(pathname)) > 0){
        pthread_mutex_unlock(&tree_lock);
        return findFileAndPid(root->right, pathname, pid_client);
    }else if(strncmp(pathname, root->pathname, str_length(pathname)) < 0){
        pthread_mutex_unlock(&tree_lock);
        return findFileAndPid(root->left, pathname, pid_client);
    }else{
        if(list_containsKey(root->pid_clients, pid_client) == true){
            pthread_mutex_unlock(&tree_lock);
            return root;
        }else{
            pthread_mutex_unlock(&tree_lock);
            return NULL;
        }
    }
}

TreeNode2* findFileMin(TreeNode2* root){
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

TreeNode2* findFileMax(TreeNode2* root) {
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

TreeNode2* deleteFile(TreeNode2* root, char* pathname){
    pthread_mutex_lock(&tree_lock);

    TreeNode2* temp;

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

TreeNode2* findFileAndDeletePid(TreeNode2 *root, char* pathname, char* pid_client){
    pthread_mutex_lock(&tree_lock);

    if(root == NULL){
        pthread_mutex_unlock(&tree_lock);
        return NULL;
    }else if(strncmp(pathname, root->pathname, str_length(pathname)) > 0){
        pthread_mutex_unlock(&tree_lock);
        return findFileAndPid(root->right, pathname, pid_client);
    }else if(strncmp(pathname, root->pathname, str_length(pathname)) < 0){
        pthread_mutex_unlock(&tree_lock);
        return findFileAndPid(root->left, pathname, pid_client);
    }else {
        pthread_mutex_unlock(&tree_lock);
        list_remove(root->pid_clients, pid_client);
        return;
    }
}
