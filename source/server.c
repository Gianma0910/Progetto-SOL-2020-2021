#define _GNU_SOURCE
#include "lib/config_parser.h"
#include "lib/my_tree1.h"
#include "lib/my_tree2.h"
#include "lib/list1.h"
#include "lib/list2.h"
#include "lib/sorted_list.h"
#include "lib/queue.h"
#include "lib/my_string.h"
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <assert.h>
#include <limits.h>
#include <sys/time.h>
#include <stdlib.h>
#include "lib/conn.h"

#define MASTER_WAKEUP_SECONDS 10
#define MASTER_WAKEUP_MS 0

bool soft_close = false;
bool server_running = true;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

TreeNode1 *storage_file;
TreeNode2 *opened_file;
settings configuration_storage = DEFAULT_SETTINGS;
queue *q;
list1 *fifo_list;
size_t storage_space;
size_t max_storable_files;
size_t fifo_counts = 0;
int pipe_fd[2];
int connected_clients = 0;

typedef struct{
    char* pathname;
    void* buffer;
    size_t size;
    list2 *pid_list;
}file_s;

void print_files(char* key, void* value, bool *exit, void *argv){
    file_s *f = (file_s*) value;
    printf("[");
    if(f->size == 0){
        //...
    }else{
        //...
    }
    printf("]");

    printf("%s: ", (strrchr(key, '/')+1));
    printf("%s\n", key);

    exit = exit;
    argv = argv;
}

static file_s* file_initialization(char* path){
    if(str_isEmpty(path))
        return NULL;

    file_s *file = malloc(sizeof(file_s));
    if(file == NULL){
        fprintf(stderr, "Impossible the file %s\n", (strrchr(path, '/')+1));
        return NULL
    }
    file->pathname = str_create(path);
    file->size = 0;
    file->buffer = NULL;
    file->pid_list = list2_create();
    return file;
}

void file_update(file_s **f, void *new_content, size_t new_size){
    if(new_size == 0){
        free(new_content);
        return;
    }
    free((*f)->buffer);
    storage_space += (*f)->size;

    (*f)->buffer = new_content;
    (*f)->size = new_size;
}

bool file_isEmpty(file_s *f){
    return f->buffer == NULL;
}

bool file_isOpenedBy(file_s *f, char* pid_client){
    return list2_containsKey(f->pid_list, pid_client);
}

bool file_isOpened(file_s *f){
    return !list2_isEmpty(f->pid_list);
}

void file_open(file_s **f, char* pid_client){
    list2_insert(&(*f)->pid_list, pid_client);
}

void file_clientClose(file_s **f, char* pid_client){
    list2_remove(&(*f)->pid_list, pid_client);
}

void file_destroy(void *f){
    file_s *file = (file_s *) f;
    free(file->buffer);
    free(file->pathname);
    list2_destroy(&file->pid_list);
    free(file);
}

void initialization_server(char* config_path){
    settings_load(&configuration_storage, config_path);
    storage_file = NULL;
    opened_file = NULL;
    q = queue_create();
    max_storable_files = configuration_storage.MAX_STORABLE_FILES;
    storage_space = configuration_storage.MAX_STORAGE_SPACE;
    fifo_list = list1_create();
    pipe(pipe_fd);

    if(configuration_storage.MAX_STORAGE_SPACE >= INT_MAX){
        fprintf(stderr, "MAX INT REACHED\n");
        exit(-1);
    }

    sigset_t mask;
    sigfillset(&mask);
    pthread_sigmask(SIG_SETMASK, &mask, NULL);

    struct sigaction s;
    memset(&s, 0, sizeof(s));

    s.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &s, NULL);
}

void close_server(){
    settings_free(&configuration_storage);
    makeEmpty1(storage_file);
    makeEmpty2(opened_file);
    close(pipe_fd[0]);
    close(pipe_fd[1]);
    list1_destroy(&fifo_list);
    queue_destroy(&q);
}

void free_space(int client, char option, size_t file_size){
    node1 *curr = fifo_list->head;

    while(true){
        if(curr == NULL){
            if(configuration_storage.PRINT_LOG == 2){
                printf("FIFO queue reading finished\n\n");
            }
            sendStr(client, "End of file from storage");
            return;
        }

        file_s *file = (file_s*) curr->key;
        assert(file != NULL && findFile(storage_file, file->pathname));

        if(!file_isOpened(file)){
            if(configuration_storage.PRINT_LOG == 2){
                printf("Delete the file %s from the queue\n", (strrchr(file->pathname, '/')+1));
            }

            if(option == 'y'){
                sendStr(client, "Not end of file from storage");
                sendStr(client, file->pathname);
                sendn(client, file->buffer, file->size);
            }

            if(option == 'c'){
                sendStr(client, "Not end of file from storage");
                sendStr(client, file->pathname);
            }

            storage_space += file->size;
            if(storage_space > configuration_storage.MAX_STORAGE_SPACE){
                storage_space = configuration_storage.MAX_STORAGE_SPACE;
            }

            //delete file->path from storage_files
            deleteFile(&storage_file, file->pathname);
            max_storable_files++;
            fifo_counts++;

            char* key = curr->key;
            curr = curr->next;
            list1_remove(&fifo_list, key, NULL);
        }else{
            curr = curr->next;
        }

        if(file_size <= storage_space && max_storable_files > 0){
            sendStr(client, "End of file from storage");
            return;
        }
    }
}

void openFile(int client, char* request){
    char **array = NULL;
    int n = str_split(&array, request, ":");
    char* file_path = array[0];
    char* client_pid = array[1];

    if(findFile(storage, file_path) == NULL){
        if(configuration_storage.PRINT_LOG == 2){
            fprintf(stderr, "The client %d want to execute an operation on not existing file\n", client);
        }

        if(configuration_storage.PRINT_LOG == 1){
            fprintf(stderr, "Can't execute the request");
        }

        sendStr(client, "File not found");
        str_clearArray(&array, n);
        return;
    }

    file_s *file = findFile(storage, file_path);
    if(file_isOpenedBy(file, client_pid)){
        if(configuration_storage.PRINT_LOG == 2){
            fprintf(stderr, "The client %d want open file that is already opened\n", client);
        }
        if(configuration_storage.PRINT_LOG == 1){
            fprintf(stderr, "Can't execute the request");
        }
        sendStr(client, "File already opened");
        str_clearArray(&array, n);
        return;
    }

    file_open(&file, client_pid);
    sendStr(client, "Success");

    if(configuration_storage.PRINT_LOG == 1 || configuration_storage.PRINT_LOG == 2){
        printf("Il client %d opened the file %s\n", client, (strrchr(file_path, '/')+1));
    }
    str_clearArray(&array, n);
}

void createFile(int client, char* request){
    size_t file_size = receiveInteger(client);

    char **array = NULL;
    int n = str_split(&array, request, ":");
    char* file_path = array[0];
    char* client_pid = array[1];

    assert(!str_isEmpty(file_path) && file_path != NULL);

    if(findFile(storage_file, file_path) != NULL){
        if(configuration_storage.PRINT_LOG == 2){
            fprintf(stderr, "The client %d want create the file %s, but it's already exists into the server\n", client, (strrchr(file_path, '/')+1));
        }
        if(configuration_storage.PRINT_LOG == 1){
            fprintf(stderr, "Can't execute the request");
        }
        sendStr(client, "File already exists");
    }else if(file_size > configuration_storage.MAX_STORAGE_SPACE){
        if(configuration_storage.PRINT_LOG == 2){
            fprintf(stderr, "The client %d want to put into the server a file too large of size\n", client);
        }
        if(configuration_storage.PRINT_LOG == 1){
            fprintf(stderr, "Can't execute the request");
        }
        sendStr(client, "File too large");
    }else if(max_storable_files == 0){
        if(configuration_storage.PRINT_LOG == 2){
            fprintf(stderr, "CAPACITY MISSES\n");
        }
        sendStr(client, "Storage full of memory");
        free_space(client, 'c', 0);
        if(max_storable_files == 0){
            if(configuration_storage.PRINT_LOG == 2){
                fprintf(stderr, "Impossible to expel file from server\n");
            }
            if(configuration_storage.PRINT_LOG == 1){
                fprintf(stderr, "Can't execute the request");
            }
            sendStr(client, "Storage full of memory");
        }
    }else{
        //da rivedere l'assert
        assert(findFileAndPid(opened_file, file_path, client_pid) != NULL);

        file_s *file = file_initialization(file_path);

        if(file == NULL){
            sendStr(client, "Malloc error");
            return;
        }
        insertFile(storage_file, file_path, file);
        file_open(&file, client_pid);

        max_storable_files--;
        list1_insert(&fifo_list, file_path, file);
        sendStr("Success");
    }
    str_clearArray(&array, n);
}

void readFile(int client, char* request){
    assert(!str_isEmpty(request) && request != NULL);

    char** array = NULL;
    int n = str_split(&array, request, ":");
    char* file_path = array[0];
    char* client_pid = array[1];

    if(findFile(storage_file, file_path) == NULL){
        sendStr(client, "File not exists");
        str_clearArray(&array, n);
        return;
    }
    file_s *file = findFile(storage_file, file_path);

    if(!file_isOpenedBy(file, client_pid)){
        sendStr(client, "File not opened");
        str_clearArray(&array, n);
        return;
    }

    sendStr(client,"Already exists");
    sendn(client, file->buffer, file->size);
    str_clearArray(&array, n);
}
