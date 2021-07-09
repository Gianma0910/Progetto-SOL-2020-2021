#define _GNU_SOURCE_
#include "lib/my_tree.h"
#include "lib/config_parser.h"
#include "lib/conn.h"
#include "lib/my_string.h"
#include "lib/sorted_list.h"
#include "lib/queue.h"
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <assert.h>
#include <limits.h>
#include <sys/time.h>
#include <stdlib.h>


#define MASTER_WAKEUP_SECONDS 10
#define MASTER_WAKEUP_MS 0

bool soft_close = false;
bool server_running = true;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

Tree* storage_file; //contiene tutti i file associati al loro path assoluto
Tree* opened_file; //memorizza quanti file un determinato client ha aperto
settings configuration_storage = DEFAULT_SETTINGS;
queue *q;
list *fifo_list;
size_t storage_space;
size_t max_storable_files;
size_t fifo_counts = 0;
int pipe_fd[2];
int connected_clients = 0;

typedef struct{
    char* pathname;
    void* buffer;
    size_t size;
    list *pid_list;
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
    file->buffer = 0;
    file->pid_list = list_create();
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
    return list_containsKey(f->pid_list, pid_client);
}

bool file_isOpened(file_s *f){
    return !list_isEmpty(f->pid_list);
}

void file_open(file_s **f, char* pid_client){
    list_insert(&(*f)->pid_list, pid_client);

    int *n = (int *) tree_get_value(opened_file, pid_client);
    *n += 1;
    opened_file = tree_update_value(opened_file, pid_client, n);
}

void file_clientClose(file_s **f, char* pid_client){
    list_remove(&(*f)->pid_list, pid_client, NULL);

    int *n = (int *) tree_get_value(opened_file, pid_client);
    *n -= 1;
    opened_file = tree_update_value(opened_file, pid_client, n);
}

void file_destroy(void *f){
    file_s *file = (file_s *) f;
    free(file->buffer);
    free(file->pathname);
    list_destroy(&file->pid_list, NULL);
    free(file);
}

void initialization_server(char* config_path){
    settings_load(&configuration_storage, config_path);
    storage_file = init_tree(configuration_storage.MAX_STORABLE_FILES);
    opened_file = init_tree(configuration_storage.MAX_STORABLE_FILES);
    q = queue_create();
    max_storable_files = configuration_storage.MAX_STORABLE_FILES;
    storage_space = configuration_storage.MAX_STORAGE_SPACE;
    fifo_list = list_create();
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
    makeEmpty(storage_file->root);
    makeEmpty(opened_file->root);
    close(pipe_fd[0]);
    close(pipe_fd[1]);
    list_destroy(&fifo_list, NULL);
    queue_destroy(&q);
}

void free_space(int client, char option, size_t file_size){
    node *curr = fifo_list->head;

    while(true){
        if(curr == NULL){
            if(configuration_storage.PRINT_LOG == 2){
                printf("FIFO queue reading finished\n\n");
            }
            sendStr(client, "End of file from storage");
            return;
        }

        file_s *file = (file_s*) curr->value;
        assert(file != NULL && tree_find(storage_file, file->pathname) != NULL);

        if(!file_isOpened(file)){
            if(configuration_storage.PRINT_LOG == 2){
                printf("Deleting the file %s from the queue\n", (strrchr(file->pathname, '/')+1));
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
            storage_file = tree_delete(storage_file, file->pathname);
            max_storable_files++;
            fifo_counts++;

            char* key = curr->key;
            curr = curr->next;
            list_remove(&fifo_list, key, NULL);
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

    if(tree_find(storage, file_path) == NULL){
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

    file_s *file = tree_get_value(storage, file_path);
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

    if(tree_find(storage_file, file_path) != NULL){
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
        assert(tree_find(opened_file, client_pid) != NULL);

        file_s *file = file_initialization(file_path);

        if(file == NULL){
            sendStr(client, "Malloc error");
            return;
        }
        storage_file = tree_insert(storage_file, file_path, file);
        file_open(&file, client_pid);

        max_storable_files--;
        list_insert(&fifo_list, file_path, file);
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

    if(tree_find(storage_file, file_path) == NULL){
        sendStr(client, "File not exists");
        str_clearArray(&array, n);
        return;
    }
    file_s *file = tree_get_value(storage_file, file_path);

    if(!file_isOpenedBy(file, client_pid)){
        sendStr(client, "File not opened");
        str_clearArray(&array, n);
        return;
    }

    sendStr(client,"Already exists");
    sendn(client, file->buffer, file->size);
    str_clearArray(&array, n);
}

void send_nfiles(char* key, void* value, bool* exit, void* args){
    int client = *((int*) args);
    file_s *file = (file_s *) value;
    sendStr(client, "Not end of file from storage");
    sendStr(client, key);
    sendn(client, file->buffer, file->size);

    exit = exit;
    args = args;
}

void readNFile(int client, char* request){
    if(tree_isEmpty(storage_file)){
        sendStr(client, "Storage empty");
        return;
    }

    int n;
    int res = str_toInteger(&n, request);
    assert(res != -1);

    sendStr(client, "No problem of request");
    tree_iterateN(storage_file, &send_nfiles, (void* ) &client, n);
    sendStr(client, "End of file from storage");
}

void appendFile(int client, char* request) {
    char **array = NULL;
    int n = str_split(&array, request, ":");
    char *file_path = array[0];
    char *pid_client = array[1];
    char option = (array[2])[0];

    assert(option == 'y' || option == 'n');

    void *file_content;
    size_t file_size;
    receiveFile(client, &file_content, &file_size);
    file_s *file = tree_find(storage_file, file_path);

    if ((file->size + file_size) > configuration_storage.MAX_STORAGE_SPACE) {
        if (configuration_storage.PRINT_LOG == 2) {
            fprintf(stderr, "The client %d sent a file, but it is too large\n\n", client);
        }

        sendStr(client, "File too large");
        free(file_content);
        return;
    }

    if (tree_find(storage_file, file_path) == NULL) {
        if (configuration_storage.PRINT_LOG == 2) {
            fprintf(stderr, "The client %d want to execute an operation on not existing file\n", client);
        }

        sendStr(client, "File not exists");
        free(file_content);
        str_clearArray(&array, n);
        return;
    }

    if (file_size > storage_space) {
        if (configuration_storage.PRINT_LOG == 2) {
            fprintf(stderr, "CAPACITY MISSES\n");
        }

        sendStr(client, "Storage full of memory");
        free_space(client, option, file_size);

        if (file_size >= storage_space) {
            if (configuration_storage.PRINT_LOG == 1 || configuration_storage.PRINT_LOG == 2) {
                fprintf(stderr, "Is not possible to free space\n\n");
            }
            free(file_content);
            str_clearArray(&array, n);
            sendStr(client, "Free error");
            return;
        }

        if (configuration_storage.PRINT_LOG == 2) {
            printf("Space freed\n");
        }
    }

    size_t  new_size = file->size + file_size;
    void *new_content = malloc(new_size);
    if(new_content == NULL){
        fprintf(stderr, "Malloc error: impossible to append the new content");
        sendStr(client, "Malloc error");
        return;
    }

    memcpy(new_content, file->buffer, file->size);
    memcpy(new_content + file->size, file_size);

    free(file->buffer);
    file->buffer = new_content;
    file->size = file_size;
    sendStr(client, "Success");

    if(configuration_storage.PRINT_LOG == 1 || configuration_storage.PRINT_LOG == 2){
        printf("Append for client %d\n\n", client);
    }
}

void writeFile(int client, char* request){
    char **array = NULL;
    int n = str_split(&array, request, ":");
    assert(n == 3);
    char* file_path = array[0];
    char* pid_client = array[1];

    char option = (array[2])[0];
    assert(option == 'y' || option == 'n');
    size_t file_size;
    void* file_content = NULL;

    receiveFile(client, &file_content, &file_size);

    if (file_size > configuration_storage.MAX_STORAGE_SPACE) {
        if (configuration_storage.PRINT_LOG == 2) {
            fprintf(stderr, "The client %d sent a file, but it is too large\n\n", client);
        }

        sendStr(client, "File too large");
        free(file_content);
        return;
    }

    if (tree_find(storage_file, file_path) == NULL) {
        if (configuration_storage.PRINT_LOG == 2) {
            fprintf(stderr, "The client %d want to execute an operation on not existing file\n", client);
        }

        sendStr(client, "File not exists");
        free(file_content);
        str_clearArray(&array, n);
        return;
    }

    file_s *file = tree_find(storage, file_path);
    if(!file_isOpenedBy(file, pid_client)){
        if(configuration_storage.PRINT_LOG == 2){
            fprintf(stderr,"The client %d want to execute an operation on a not opened file\n", client);
        }

        sendStr(client, "File is not open");
        free(file_content);
        str_clearArray(&array, n);
        return;
    }else if(!file_isEmpty(file)){
        if(configuration_storage.PRINT_LOG == 2){
            fprintf(stderr, "The client %d want to write on a not empty file\n", client);
        }

        sendStr(client, "File not empty");
        free(file_content);
        str_clearArray(&array, n);
        return;
    }

    if (file_size > storage_space) {
        if (configuration_storage.PRINT_LOG == 2) {
            fprintf(stderr, "CAPACITY MISSES\n");
        }

        sendStr(client, "Storage full of memory");
        free_space(client, option, file_size);

        if (file_size > storage_space) {
            if (configuration_storage.PRINT_LOG == 1 || configuration_storage.PRINT_LOG == 2) {
                fprintf(stderr, "Is not possible to free space\n\n");
            }
            free(file_content);
            str_clearArray(&array, n);
            sendStr(client, "Free error");
            return;
        }

        if (configuration_storage.PRINT_LOG == 2) {
            printf("Space freed\n");
        }
    }

    assert(file->pathname != NULL && !str_isEmpty(file->pathname));
    assert(storage_space <= configuration_storage.MAX_STORAGE_SPACE);

    file_update(&f, file_content, file_size);
    storage_space -= file_size;

    sendStr(client, "Success");

    str_clearArray(&array, n);

    if(configuration_storage.PRINT_LOG == 1 || configuration_storage.PRINT_LOG == 2){
        printf("Write completed\n"
               "Capacity of storage: %zu\n", storage_space);
    }
}

void clear_openedFiles(char* key, void* value, bool* exit, void* client_pid){
    file_s *file = (file_s *) value;
    if(file_isOpenedBy(file, (char *) client_pid)){
        file_clientClose(&file, (char *) client_pid);
    }

    key = key;
    exit = exit;
}

void closeConnection(int client, char* client_pid){
    int nfiles = *((int *)  tree_find(opened_file, client_pid));

    if(nfiles == 0){
        sendStr(client, "Success");
    }else {
        if (configuration_storage.PRINT_LOG == 2) {
            fprintf(stderr, "ATTENTION: the client %d hasn't close some files, closing...\n", client);
        }

        sendStr(client, "File not found on exit");
        //tree_iterate();
        if (configuration_storage.PRINT_LOG == 2) {
            printf("Close completed\n");
        }
    }

    assert((*((int *) tree_get_value(opened_file, client_pid))) == 0);

    tree_delete(opened_file, client_pid);
    if(close(client) != 0){
        fprintf(stderr, "ATTENTION: error in closing the socket with client %d\n", client);
    }else if(configuration_storage.PRINT_LOG == 1 || configuration_storage.PRINT_LOG == 2){
        printf("Client %d disconnected\n\n", client);
    }

    connected_clients--;
}

void closeFile(int client, char* request){
    char** array = NULL;
    int n = str_split(&array, request, ":");
    assert(n == 2);

    char* file_path = array[0];
    char* pid_client = array[1];

    if(tree_find(storage_file, file_path) == NULL){
        if(configuration_storage.PRINT_LOG == 2){
            fprintf(stderr, "The client %d want to close a file that not exists\n", client);
        }

        sendStr(client,"File not found");
        str_clearArray(&array, n);
        return;
    }

    file_s *file = tree_get_value(storage, file_path);
    if(!file_isOpenedBy(file, pid_client)){
        if(configuration_storage.PRINT_LOG == 2){
            fprintf(stderr, "The client %d want to close a file not opened\n", client);
        }

        sendStr(client, "File not opened");
        str_clearArray(&array, n);
        return;
    }

    file_clientClose(&file, pid_client);

    sendStr(client, "Success");
    if(configuration_storage.PRINT_LOG == 1 || configuration_storage.PRINT_LOG == 2){
        printf("File %s closed by %d client\n\n", (strrchr(file_path, '/')+1), client);
    }

    str_clearArray(&array, n);
}

void removeFile(int client, char* request){
    if(tree_find(storage_file, request) == NULL){
        if(configuration_storage.PRINT_LOG == 2){
            fprintf(stderr, "The client %d want to remove a file that not exists\n", client);
        }

        sendStr(client, "File not found");
        return;
    }

    file_s *file = tree_get_value(storage_file, request);
    if(file_isOpened(file)){
        if(configuration_storage.PRINT_LOG == 2){
            fprintf(stderr, "The client %d want to close an opened file", client);
        }

        sendStr(client, "File still opened");
        return;
    }

    storage_space += file->size;
    if(storage_space > configuration_storage.MAX_STORAGE_SPACE){
        storage_space = configuration_storage.MAX_STORAGE_SPACE;
    }

    tree_delete(storage_file, request);
    max_storable_files++;
    sendStr(client, "Success");

    if(configuration_storage.PRINT_LOG == 1 || configuration_storage.PRINT_LOG == 2){
        printf("File %s removed by client %d\n\n", (strrchr(request)+1), client);
    }
}

void* stop_server(void* args){
    sigset_t set;
    int signal_captured;
    int t = -1;

    sigemptyset(&set);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGQUIT);
    sigaddset(&set, SIGHUP);

    if(configuration_storage.PRINT_LOG == 2){
        printf("SIGWAIT Thread started\n\n");
    }

    pthread_sigmask(SIG_SETMASK, &set, NULL);

    if(sigwait(&set, &signal_captured) != 0){
        soft_close = true;
        return NULL;
    }

    if(signal_captured == SIGINT || signal_captured == SIGQUIT){
        server_running = false;
    }else if(signal_captured == SIGHUP || signal_captured == SIGTERM){
        soft_close = true;
    }

    writen(pipe_fd[1], sizeof(int));
    return argv;
}

