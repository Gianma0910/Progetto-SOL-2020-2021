#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include "../lib/my_string.h"
#include "../lib/my_tree.h"

pthread_mutex_t tree_lock = PTHREAD_MUTEX_INITIALIZER;

TreeNode* makeEmpty(TreeNode* root){
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

TreeNode* insertFile(TreeNode* root, char* key, void* value){
    pthread_mutex_lock(&tree_lock);

    if(root == NULL){
        root = malloc(sizeof(TreeNode1));
        strncpy(root->key, key, str_length(key));
        root->value = value;
        root->left = NULL;
        root->right = NULL;
    }else if(strncmp(key, root->key, str_length(key)) > 0){
        pthread_mutex_unlock(&tree_lock);
        root->right = insertFile(root->right, key);
    }else if(strncmp(key, root->key, str_length(key)) < 0){
        pthread_mutex_unlock(&tree_lock);
        root->left = insertFile(root->left, key);
    }

    pthread_mutex_unlock(&tree_lock);

    return root;
}

TreeNode* findFile(TreeNode* root, char* key){
    pthread_mutex_lock(&tree_lock);

    if(root == NULL){
        pthread_mutex_unlock(&tree_lock);
        return NULL;
    }else if(strncmp(key, root->key, str_length(key)) > 0){
        pthread_mutex_unlock(&tree_lock);
        return findFile(root->right, key);
    }else if(strncmp(key, root->key, str_length(key)) < 0){
        pthread_mutex_unlock(&tree_lock);
        return findFile(root->left, key);
    }else{
        pthread_mutex_unlock(&tree_lock);
        return root;
    }
}

TreeNode* findFileMin(TreeNode* root){
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

TreeNode* findFileMax(TreeNode* root) {
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

TreeNode* deleteFile(TreeNode* root, char* key){
    pthread_mutex_lock(&tree_lock);

    TreeNode1* temp;

    if(root == NULL){
        return NULL;
    }else if(strncmp(root->key, key, str_length(key)) < 0){
        pthread_mutex_unlock(&tree_lock);
        root->left = deleteFile(root->left, key);
    }else if(strncmp(root->key, key, str_length(key)) > 0){
        pthread_mutex_unlock(&tree_lock);
        root->right = deleteFile(root->right, key);
    }else if(root->left && root->right){
        pthread_mutex_unlock(&tree_lock);
        temp = findFileMin(root->right);
        strncpy(root->key, temp->key, str_length(temp->key));
        root->value = temp->value;
        root->right = deleteFile(root->right, root->key);
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

TreeNode* updateValue(TreeNode* root, char* key, void* new_value){
    pthread_mutex_lock(&tree_lock);

    if(root == NULL){
        pthread_mutex_unlock(&tree_lock);
        return NULL;
    }else if(strncmp(key, root->key, str_length(key)) > 0){
        pthread_mutex_unlock(&tree_lock);
        return findFile(root->right, key);
    }else if(strncmp(key, root->key, str_length(key)) < 0){
        pthread_mutex_unlock(&tree_lock);
        return findFile(root->left, key);
    }else{
        root->value = new_value;
        pthread_mutex_unlock(&tree_lock);
        return root;
    }
}

bool tree_isEmpty(TreeNode* root){
   return root == NULL;
}

void* getValue(TreeNode* root, char* key){
    pthread_mutex_lock(&tree_lock);

    if(root == NULL){
        pthread_mutex_unlock(&tree_lock);
        return NULL;
    }else if(strncmp(key, root->key, str_length(key)) > 0){
        pthread_mutex_unlock(&tree_lock);
        return findFile(root->right, key);
    }else if(strncmp(key, root->key, str_length(key)) < 0){
        pthread_mutex_unlock(&tree_lock);
        return findFile(root->left, key);
    }else{
        root->value = new_value;
        pthread_mutex_unlock(&tree_lock);
        return root->value;
    }
}