#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/un.h>
#include <sys/time.h>
#include <stdbool.h>
#include "../lib/my_string.h"
#include "../lib/conn.h"
#include "../lib/file_reader.h"
#include "../lib/client_api.h"

bool running = true;
int fd_sk = 0;
char* current_sock = NULL;
bool connected = false;

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void exit_function(){
    if(connected){
        closeConnection(current_sock);
    }
}

int openConnection(const char* sockname, int msec, const struct timespec abstime){
    struct timeval now;
    gettimeofday(&now, NULL);
    abstime.tv_sec = now.tv_sec+msec;
    fd_sk = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un sa;
    strcpy(sa.sun_path, sockname);
    sa.sun_family = AF_UNIX;
    pthread_t tid;
    pthread_create(&tid, NULL, &stop, &abstime);

    while(run){
        int status = connect(fd_sk, (struct sockaddr*) &sa, sizeof(sa));
        if(status == 0){
            char* status_connection = receiveStr(fd_sk);
            if(!str_equals(status_connection, "Not connected to socket")){
                errno = EBADF;
                pthread_join(tid, NULL);
                return -1;
            }else{
                current_sock = str_create(sockname);
                char* pid_client = str_long_toStr(getpid());
                sendn(fd_sk, pid_client, str_length(pid_client));
                atexit(exit_function);
                connected = true;
                return 0;
            }
            usleep(msec*1000);
        }
        pthread_join(tid, NULL);
        errno = EBADF;
        return -1;
    }
}

static void *stop(void *arg){
    struct timespec timeToWait = (*(struct timespec) *arg);
    pthread_mutex_lock(&lock);
    pthread_cond_timedwait(&cond, &lock, timeToWait);
    run = false;
    pthread_mutex_unlock(&lock);

    return;
}

int closeConnection(const char* sockname){
    if(sockname == NULL){
        errno = EINVAL;
        return -1;
    }

    if(!str_equals(current_sock, sockname)){
        errno = EINVAL;
        return -1;
    }

    char* pid_client = str_long_toStr(getpid());
    char* command = str_concat("cl:", pid_client);

    sendStr(fd_sk, command);
    free(pid_client);
    free(command);

    char* answer = receiveStr(fd_sk);

    if(str_equals(answer, "Unsuccess")){
        errno = ENOENT;
        return -1;
    }

    if(close(fd_sk) == 0){
        connected = false;
        free(current_sock);
        return 0;
    }else{
        errno = ENOENT;
        return -1;
    }
}

int openFile(const char* pathname, int flags){
    if(pathname == NULL){
        errno = EINVAL;
        return -1;
    }

    char* answer;
    char* pid_client = str_long_toStr(getpid());

    if(flags == O_CREATE){
        char* abs_path = realpath(pathname, NULL);
        if(abs_path == NULL){
            errno = EPERM;
            return -1;
        }else{
            FILE* file = fopen(abs_path, "rb");
            size_t file_size = file_getsize(file);

            char* command = str_concatn("c:", abs_path, ":", pid_client, NULL);
            sendStr(fd_sk, command);
            sendInteger(fd_sk, file_size);

            answer = receiveStr(fd_sk);

            if(str_equals(answer, "Storage full of memory")){
                while(!str_equals(receiveStr(fd_sk), "End of file from storage")){
                    char* s = receiveStr(fd_sk);
                    printf("Il file %s è stato espulso dallo storage\n", strrchr(s, '/')+1);
                    free(s);
                }
                answer = receiveStr(fd_sk);
            }else if(!str_equals(answer, "Success")){
                free(command);
                free(abs_path);
                free(pid_client);
                errno = EINTR;
                return -1;
            }

            free(cmd);
            free(abs_path);

            return 0;
        }
    }else if(flags == O_LOCK){
        errno = EACCES;
        return -1;
    }else if(flags == O_OPEN){
        char* command = str_concatn("o:", pathname, ":", pid_client, NULL);

        sendStr(fd_sk, command);
        answer = receiveStr(fd_sk);

        if(!str_equals(answer, "Success")){
            errno = EINTR;
            return -1;
        }
        free(command);
        return 0;
    }else{
        errno = EINVAL;
        return -1;
    }

    free(pid_client);
}

int readFile(const char* pathname, void **buf, size_t size){
    if(pathname == NULL){
        errno = EINVAL;
        return -1;
    }

    char* pid_client = str_long_toStr(getpid());
    char* command = str_concatn("r:", pathname, ":", pid_client, NULL);

    sendStr(fd_sk, command);
    answer = receiveStr(fd_sk);

    if(str_equals(answer, "Already exists")){
        receiveFile(fd_sk, buf, &size);
        free(command);
        free(pid_client);

        return 0;
    }else{
        free(pid_client);
        free(command);
        errno = EPERM;
        return -1;
    }
}

int readNfiles(int N, const char* dirname){
    char* dir = NULL;

    if(dirname != NULL){
        if(!str_endsWith(dirname, "/")){
            dir = str_concat(dirname, "/");
        }else{
            dir = str_create(dirname);
        }
    }

    char* number_file_to_read = str_long_toStr(N);
    char* command = str_concat("rn:", number_file_to_read);

    sendStr(fd_sk, command);

    char* answer = receiveStr(fd_sk);

    if(!str_equals(answer, "No problem of request")){
        errno = EPERM;
        return -1;
    }

    size_t size;
    void* buffer;

    if(dir != NULL){
        while(!str_equals(receiveStr(fd_sk), "End of file from storage")){
            char* file_path = receiveStr(fd_sk);
            char* file_name = strrchr(file_path, '/')+1;
            char* path = str_concat(dir, file_name);

            FILE* file = fopen(path, "wb");
            if(file == NULL){
                errno = EINVAL;
                return -1;
            }
            fwrite(buffer, sizeof(char), size, file);
            fclose(file);

            free(buffer);
            free(path);
            free(file_path);
        }
    }else{
        while(!str_equals(receiveStr(fd_sk), "End of file from storage")){
            char* file_path = receiveStr(fd_sk);
            free(file_path);
            receiveFile(fd_sk, &buffer, &size);
            free(buffer);
        }
    }

    free(command);
    free(dir);
    free(number_file_to_read);

    return 0;
}

int writeFile(const char* pathname, const char* dirname){
    if(pathname == NULL && dirname != NULL){
        errno = EINVAL;
        return -1;
    }

    if(pathname == NULL){
        errno = EINVAL;
        return -1;
    }

    char* pid_client = str_long_toStr(getpid());
    char* abs_path = realpath(pathname, NULL);

    if(abs_path == NULL){
        errno = EINVAL;
        return -1;
    }

    char* command;

    if(dirname != NULL){
        command = str_concatn("w:", abs_path, ":", pid_client, "?y", NULL);
    }else{
        command = str_concatn("w:", abs_path, ":", pid_client, "?y", NULL);
    }

    sendStr(fd_sk, command);

    if(sendFile(fd_sk, pathname) == -1){
        errno = EACCES;
        return -1;
    }

    free(abs_path);
    free(request);
    free(pid_client);

    char* answer = receiveStr(fd_sk);

    if(str_equals(answer, "Success")){
        return 0;
    }

    if(str_equals(answer, "Storage full of memory")){
        errno = EPERM;
        return -1;
    }

    if(dirname != NULL){
        char* dir = NULL;

        if(!str_endsWith(dirname, "/")){
            dir = str_concat(dirname, "/");
        }else{
            dir = str_create(dirname);
        }
        printf("CAPACITY MISSES: Ricezione file espulsi dal server...\n\n");
        while(!str_equals(receiveStr(fd_sk), "End of file from storage")){
            char* file_path = receiveStr(fd_sk);
            char* file_name = strrchr(file_path, '/')+1;
            char* path = str_concat(dir, file_name);

            void* buffer;
            size_t n;

            receiveFile(fd_sk, &buffer, &n);
            FILE* file = fopen(path, "wb");

            if(file == NULL){
                fprintf(stderr, "Impossibile creare un nuovo file, libera spazio\n");
            }else{
                fwrite(buffer, sizeof(char), n, file);
                fclose(file);
            }
            free(buffer);
            free(file_path);
            free(path);
        }
        free(dir);
    }else{
        receiveStr(fd_sk);
    }

    answer = receiveStr(fd_sk);
    if(!str_equals(answer, "Success")){
        errno = EINTR;
        return -1;
    }
    return 0;
}

int closeFile(const char* pathname){
    if(pathname == NULL){
        errno = EINVAL;
        return -1;
    }

    char* pid_client = str_long_toStr(getpid());
    char* command = str_concatn("cl:", pathname, ":", pid_client, NULL);

    sendStr(fd_sk, command);

    free(command);
    free(pid_client);

    char* answer = receiveStr(fd_sk);
    if(!str_equals(answer, "File closed")){
        errno = EINTR;
        return -1;
    }
    return 0;
}

int removeFile(const char* pathname){
    if(pathname == NULL){
        errno = EINVAL;
        return -1;
    }

    char* command = str_concat("rm:", pathname);
    sendStr(fd_sk, command);

    char* answer = receiveStr(fd_sk);
    if(!str_equals(answer, "File eliminated")){
        errno = EINTR;
        return -1;
    }else
        return 0;
}

int appendToFile(const char* pathname, void* buf, size_t size, const char* dirname){
    char* pid_client = str_long_toStr(getpid());
    char* command;

    if(dirname != NULL){
        command = str_concatn("a:", pathname, ":", pid_client, "?y", NULL);
    }else{
        command = str_concatn("a:", pathname, ":", pid_client, "?n", NULL);
    }

    sendStr(fd_sk, command);
    sendn(fd_sk, buf, size);

    free(pid_client);
    free(command);

    char* answer = receiveStr(fd_sk);
    if(str_equals(answer, "Success")){
        return 0;
    }

    if(!str_equals(answer, "Storage full of memory")){
        errno = EPERM;
        return -1;
    }

    if(dirname != NULL){
        char* dir = NULL;

        if(!str_endsWith(dirname. "/")){
            dir = str_concat(dirname, "/");
        }else{
            dir = str_create(dirname);
        }

        printf("CAPACITY MISSES: Ricezione file espulsi dal server...\n\n");
        while(!str_equals(receiveStr(fd_sk), "End of file from storage")){
            char* file_path = receiveStr(fd_sk);
            char* file_name = strrchr(file_path, '/')+1;
            char* path = str_concat(dir, file_name);

            void* buffer;
            size_t n;
            receiveFile(fd_sk, &buffer, &n);
            FILE* file = fopen(path, "wb");
            if(file == NULL){
                fprintf(stderr, "Impossibile creare un nuovo file\n");
            }else{
                fwrite(buffer, sizeof(char), n, file);
                fclose(file);
            }
            free(buffer);
            free(path);
            free(file_path);
        }
        free(dir);
    }else{
        receiveStr(fd_sk);
    }

    answer = receiveStr(fd_sk);
    if(!str_equals(answer, "Success")){
        errno = EINTR;
        return -1;
    }
    return 0;
}