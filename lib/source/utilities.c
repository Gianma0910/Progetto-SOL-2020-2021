#include "../utilities.h"
#include "../file_reader.h"

int unix_socket(char* path){
    int fd_sk = socket(AF_UNIX, SOCK_STREAM, 0);
    if(fd_sk == -1){
        fprintf(stderr, "Socket creation error\n");
        return errno;
    }
    unlink(path);
    return fd_sk;
}

int socket_bind(int fd_sk, char* path){
    struct sockaddr_un sa;
    strcpy(sa.sun_path, path);
    sa.sun_family = AF_UNIX;

    if((bind(fd_sk, (const struct sockaddr *) &sa, sizeof(sa))) == -1){
        fprintf(stderr, "Can't bind\n");
        if(errno == EADDRINUSE){
            fprintf(stderr, "Address already in use\n");
        }
        return errno;
    }
    listen(fd_sk, SOMAXCONN);
    return 0;
}

int socket_accept(int fd_sk){
    int fd_c = accept(fd_sk, NULL, 0);
    return fd_c;
}

size_t readn(int fd, void* buffer, size_t size){
    size_t left = size;
    int r;
    void* bufptr = buffer;
    while(left > 0){
        if((r = (int) read(fd, bufptr, left)) == -1){
            if(errno == EINTR) continue;
            return -1;
        }
        if(r == 0) return 0;
        left -= r;
        bufptr += r;
    }
    return size;
}

int writen(int fd, void* buffer, size_t size){
    size_t left = size;
    int r;
    void* bufptr = buf;
    while(left > 0){
        if((r = (int) write(fd, bufptr, left)) == -1){
            if(errno == EINTR) continue;
            return -1;
        }
        if(r == 0) return 0;
        left -= r;
        bufptr += r;
    }
    return 1;
}

int sendInteger(int fd_sk, size_t n){
    if(writen(fd_sk, &n, sizeof(unsigned long)) == -1){
        fprintf(stderr, "An error occured on sending message length\n");
        return errno;
    }

    return 0;
}

size_t receiveInteger(int fd_sk){
    size_t n = 0;
    if(readn(fd_sk, &n, sizeof(unsigned long)) == -1){
        fprintf(stderr, "An error occured reading message length\n");
        return errno;
    }

    return n;
}

int sendn(int fd_sk, void* message, size_t length){
    if(sendInteger(fd_sk, length) != 0){
        fprintf(stderr, "Error on sending the dimension of file\n");
        return errno;
    }

    if(writen(fd_sk, message, length) == -1){
        fprintf(stderr, "An error occured on sending message\n");
        return errno;
    }
    return 0;
}

int sendFile(int fd_sk, const char* pathname){
    FILE* file = fopen(path, "rb");
    if(file == NULL){
        return -1;
    }

    size_t file_size = file_getsize(file);

    if(sendInteger(fd_sk, file_size) != 0){
        fprintf(stderr, "Error on sending the dimension of file\n");
        return errno;
    }

    void* file_content = file_readAll(file);
    if(file_content == NULL){
        fclose(file);
        return -1;
    }

    if(writen(fd_sk, file_content, file_size) == -1){
        fprintf(stderr, "An error occured on sending file\n");
        exit(errno);
    }

    free(file_content);
    return 0;
}

void receiveFile(int fd_sk, void** buffer, size_t* length){
    size_t size = receiveInteger(fd_sk);
    *length = size;

    *buffer = malloc(size*sizeof(char));
    if(*buffer == NULL){
        fprintf(stderr, "Impossible to receive a file: out of memory\n");
        exit(errno);
    }
    if(readn(fd_sk, *buffer, size) == -1){
        fprintf(stderr, "AN error occured reading file\n");
        exit(errno);
    }
}

void sendStr(int to, char* message){
    sendn(to, message, (int) strlen(message));
}

char* receiveStr(int from){
    size_t length = receiveInteger(from) * sizeof(char);

    char* buffer = calloc(length+1, sizeof(char));
    if(buffer == NULL){
        return NULL;
    }
    if(readn(from, buffer, length) == -1){
        fprintf(stderr, "An error occured on reading message\n");
        return NULL;
    }
    buffer[length] = '\0';
    return (char*) buffer;
}