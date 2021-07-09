#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include "../lib/my_string.h"
#include "../lib/my_tree.h"
#include "../lib/stack.h"

pthread_mutex_t tree_lock = PTHREAD_MUTEX_INITIALIZER;

Tree* init_tree(unsigned int size){
    Tree* new_tree = (Tree*) malloc(sizeof(Tree));

    new_tree->root = NULL;
    new_tree->max_size = size;
    new_tree->current_size = 0;

    return new_tree;
}

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

Tree* tree_insert(Tree* tree, char* key, void* value){
    pthread_mutex_lock(&tree_lock);

    TreeNode* new_node = (TreeNode *) malloc(sizeof(TreeNode));

    new_node->key = str_create(key);
    new_node->value = value;
    new_node->right = NULL;
    new_node->left = NULL;

    TreeNode* prev = NULL;
    TreeNode* curr = tree->root;

    while(curr != NULL){
        prev = curr;
        if(strncmp(key, curr->key, str_length(key)) > 0){
            curr = curr->right;
        }else if(strncmp(key, curr->key, str_length(key)) < 0){
            curr = curr->left;
        }
    }

    if(prev == NULL){
        tree->root = new_node;
    }else if(strncmp(key, prev->key, str_length(key)) > 0){
        prev->right = new_node;
    }else if(strncmp(key, prev->key, str_length(key)) < 0){
        prev->left = new_node;
    }

    pthread_mutex_unlock(&tree_lock);
    tree->current_size++;
    return tree;
}

void* tree_find(Tree* tree, char* key){
    pthread_mutex_lock(&tree_lock);

    if(tree_isEmpty(tree)){
        pthread_mutex_unlock(&tree_lock);
        return NULL;
    }

    while(tree->root != NULL && strncmp(tree->root->key, key, str_length(key)) == 0){
        if(strncmp(key, tree->root->key, str_length(key)) > 0){
            tree->root = tree->root->right;
        }else if(strncmp(key, tree->root->key, str_length(key)) < 0){
            tree->root = tree->root->left;
        }
    }

    pthread_mutex_unlock(&tree_lock);
    return tree->root;
}

TreeNode* tree_find_min(Tree* tree) {
    pthread_mutex_lock(&tree_lock);

    if (tree_isEmpty(tree)) {
        pthread_mutex_unlock(&tree_lock);
        return NULL;
    }

    while(tree->root->left != NULL){
        tree->root = tree->root->left;
    }

    pthread_mutex_unlock(&tree_lock);
    return tree->root;
}

TreeNode* tree_find_max(Tree* tree){
    pthread_mutex_lock(&tree_lock);

    if(tree_isEmpty(tree)){
        pthread_mutex_unlock(&tree_lock);
        return NULL;
    }

    while(tree->root->right != NULL){
        tree->root = tree->root->right;
    }

    pthread_mutex_unlock(&tree_lock);
    return tree->root;
}

Tree* tree_delete(Tree* tree, char* key){
    pthread_mutex_lock(&tree_lock);

    TreeNode* node = tree->root;
    TreeNode* parent = NULL;

    while(node != NULL){
        if(strncmp(node->key, key, str_length(key)) > 0){
            parent = node;
            node = node->left;
        }else if(strncmp(node->key, key, str_length(key)) < 0)){
            parent = node;
            node = node->right;
        }else{
            if(node->left == NULL && node->right == NULL){
                if(parent->left == NULL)
                    parent->left = NULL;
                else
                    parent->right = NULL;

                node = NULL;

                break;
            }else{
                if(node->left == NULL){
                    if(parent->right == node)
                        parent->right = node->right;
                    else if(parent->left == node)
                        parent->left = node->right;

                    node = NULL;

                    break;
                }else if(node->right == NULL){
                    if(parent->left == node)
                        parent->left = node->left;
                    else if(parent->right == node)
                        parent->right = node->left;

                    node = NULL;
                    break;
                }else{
                    TreeNode* temp = node;
                    TreeNode* px = NULL;
                    temp = temp->right;
                    while(temp->left != NULL){
                        px = temp;
                        temp = temp->left;
                    }
                    node->key = temp->key;
                    if(px->left == temp)
                        px->left = NULL;
                    else if(px->right == temp)
                        px->right = NULL;

                    temp = NULL;
                    break;
                }
            }
        }
    }

    pthread_mutex_unlock(&tree_lock);
    tree->current_size--;
    return tree;
}

Tree* tree_update_value(Tree* tree, char* key, void* new_value){
    pthread_mutex_lock(&tree_lock);

    if(tree_isEmpty(tree)){
        pthread_mutex_unlock(&tree_lock);
        return NULL;
    }

    TreeNode* node = tree->root;

    while(node != NULL && strncmp(node->key, key, str_length(key)) == 0){
        if(strncmp(key, node->key, str_length(key)) > 0){
            node = node->right;
        }else if(strncmp(key, node->key, str_length(key)) < 0){
            node = node->left;
        }
    }

    if(node == NULL){
        pthread_mutex_unlock(&tree_lock);
        return tree;
    }else{
        node->value = new_value;
        pthread_mutex_unlock(&tree_lock);
        return tree;
    }
}

bool tree_isEmpty(Tree* tree){
   return tree->root == NULL;
}

void* tree_get_value(Tree* tree, char* key){
    pthread_mutex_lock(&tree_lock);

    if(tree_isEmpty(tree)){
        pthread_mutex_unlock(&tree_lock);
        return NULL;
    }

    TreeNode* node = tree->root;

    while(node != NULL && strncmp(node->key, key, str_length(key)) == 0){
        if(strncmp(key, node->key, str_length(key)) > 0){
            node = node->right;
        }else if(strncmp(key, node->key, str_length(key)) < 0){
            node = node->left;
        }
    }

    if(node == NULL){
        pthread_mutex_unlock(&tree_lock);
        return NULL;
    }else{
        pthread_mutex_unlock(&tree_lock);
        return node->value;
    }
}

void tree_iterate(Tree* tree, void (*function)(char*, void*, bool*, void*), void* args){
    pthread_mutex_lock(&tree_lock);
    bool exit = false;
    TreeNode* curr = tree->root;
    Stack_node* stack = NULL;
    int num_files = 0;

    while(!exit && num_files < tree->max_size){
        if(curr != NULL){
            push(&stack, curr);
            function(curr->key, curr->value, &exit, args);
            curr = curr->left;
            num_files++;
        }else{
            if(!stack_isEmpty(stack)){
                curr = pop(&stack);
                curr = curr->right;
            }else{
                exit = 1;
            }
        }
    }

    pthread_mutex_unlock(&tree_lock);
}

void tree_iterateN(Tree* tree, void (*function)(char*, void*, bool*, void*), void* args, int n){
    if(n > tree->max_size || n <= 0){
        tree_iterate(table, f, args);
        return;
    }
    pthread_mutex_lock(&tree_lock);
    TreeNode* curr = tree->root;
    Stack_node* stack = NULL;
    int num_files = 0;
    bool exit = false;

    while(!exit && num_files < tree->max_size && n > 0){
        if(curr != NULL){
            push(&stack, curr);
            function(curr->key, curr->value, &exit, args);
            curr = curr->left;
            num_files++;
            n--;
        }else{
            if(!stack_isEmpty(stack)){
                curr = pop(&stack);
                curr = curr->right;
            }else{
                exit = 1;
            }
        }
    }

    pthread_mutex_unlock(&tree_lock);
}