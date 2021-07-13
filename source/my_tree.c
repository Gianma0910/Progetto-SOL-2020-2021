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

void makeEmpty(Tree* tree, void (*delete_value)(void*)){
    pthread_mutex_lock(&tree_lock);

    if(tree->root == NULL){
        pthread_mutex_unlock(&tree_lock);
        return;
    }

    Stack_node* stack;

    push_node(&stack, tree->root);
    while(!stack_empty(stack)){
        TreeNode* node = pop_node(&stack);

        if(node->left != NULL)
            push_node(&stack, node->left);
        if(node->right != NULL)
            push_node(&stack, node->right);

        if(delete_value != NULL){
            delete_value(node);
        }

        free(node);
    }

    pthread_mutex_unlock(&tree_lock);
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

    TreeNode* curr = tree->root;

    while(curr != NULL && strncmp(curr->key, key, str_length(key)) != 0){
        if(strncmp(key, curr->key, str_length(key)) > 0){
            curr = curr->right;
        }else if(strncmp(key, curr->key, str_length(key)) < 0){
            curr = curr->left;
        }
    }

    pthread_mutex_unlock(&tree_lock);
    return curr;
}

TreeNode* tree_find_min(Tree* tree) {
    pthread_mutex_lock(&tree_lock);

    if (tree_isEmpty(tree)) {
        pthread_mutex_unlock(&tree_lock);
        return NULL;
    }

    TreeNode* curr = tree->root;

    while(curr->left != NULL){
        curr = curr->left;
    }

    pthread_mutex_unlock(&tree_lock);
    return curr;
}

TreeNode* tree_find_max(Tree* tree){
    pthread_mutex_lock(&tree_lock);

    if(tree_isEmpty(tree)){
        pthread_mutex_unlock(&tree_lock);
        return NULL;
    }

    TreeNode* curr = tree->root;

    while(curr->right != NULL){
        curr = curr->right;
    }

    pthread_mutex_unlock(&tree_lock);
    return curr;
}

void tree_delete(Tree** tree, char* key, void (*delete_value)(void*)){
    pthread_mutex_lock(&tree_lock);

    TreeNode* node = (*tree)->root;
    TreeNode* parent = NULL;

    while(node != NULL && strncmp(node->key, key, str_length(key)) != 0){
        parent = node;
        if(strncmp(key, node->key, str_length(key)) > 0){
            node = node->right;
        }else{
            node = node->left;
        }
    }

    if(node == NULL){
        pthread_mutex_unlock(&tree_lock);
        return;
    }

    if(node->left == NULL || node->right == NULL){
        TreeNode* new_node;
        if(node->left == NULL)
            new_node = node->right;
        else
            new_node = node->left;

        if(parent == NULL){
            pthread_mutex_unlock(&tree_lock);
            return;
        }


        if(node == parent->left)
            parent->left = new_node;
        else
            parent->right = new_node;

        if(delete_value != NULL){
            delete_value(node);
        }
        free(node);
    }else{
        TreeNode* p = NULL;
        TreeNode* temp;

        temp = node->right;
        while(temp->left != NULL){
            p = temp;
            temp = temp->left;
        }

        if(p != NULL)
            p->left = temp->right;
        else
            node->right = temp->right;

        node->key = temp->key;

        if(delete_value != NULL){
            delete_value(temp);
        }
        free(temp);
    }

    pthread_mutex_unlock(&tree_lock);
    (*tree)->current_size--;
    return;

}

Tree* tree_update_value(Tree* tree, char* key, void* new_value){
    pthread_mutex_lock(&tree_lock);

    if(tree_isEmpty(tree)){
        pthread_mutex_unlock(&tree_lock);
        return NULL;
    }

    TreeNode* node = tree->root;

    while(node != NULL && strncmp(node->key, key, str_length(key)) != 0){
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

    while(node != NULL && strncmp(node->key, key, str_length(key)) != 0){
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
            push_node(&stack, curr);
            function(curr->key, curr->value, &exit, args);
            curr = curr->left;
            num_files++;
        }else{
            if(!stack_empty(stack) && num_files < tree->max_size){
                curr = pop_node(&stack);
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
        tree_iterate(tree, function, args);
        return;
    }
    pthread_mutex_lock(&tree_lock);
    TreeNode* curr = tree->root;
    Stack_node* stack = NULL;
    int num_files = 0;
    bool exit = false;

    while(!exit && num_files < tree->max_size && n > 0){
        if(curr != NULL){
            push_node(&stack, curr);
            function(curr->key, curr->value, &exit, args);
            curr = curr->left;
            num_files++;
            n--;
        }else{
            if(!stack_empty(stack) && n > 0){
                curr = pop_node(&stack);
                curr = curr->right;
            }else{
                exit = 1;
            }
        }
    }

    pthread_mutex_unlock(&tree_lock);
}