#include "../lib/conn.h"
#include "../lib/file_reader.h"
#include <stdio.h>

int unix_socket(char* path){
    int fd_sk = socket(AF_UNIX, SOCKSTREAM, 0);
    if(fd_sk == -1){
        fprintf(stderr, "socket creation error\n");
        return errno;
    }
    unlink(path);

    return fd_sk;
}

int socket_bind(int fd_sk; char* path){
    struct sockaddr_un sa;
    strcpy(sa.sun_path, path);
    sa.sun_family = AF_UNIX;

    if((bind(fd_sk, (const struct sockaddr *) &sa, sizeof(sa))) == -1){
        fprintf(stderr, "cannot bind\n");
        if(errno == EADDRINUSE){
            fprintf(stderr, "address already in use\n");
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

size_t readn(int fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    void* bufptr = buf;
    while(left>0) {
        if ((r = (int) read(fd ,bufptr, left)) == -1) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (r == 0) return 0;   // EOF
        left    -= r;
        bufptr  += r;
    }
    return size;
}


size_t writen(int fd, void *buf, size_t size){
    size_t left = size;
    int r;
    void* bufptr = buf;

    while(left > 0){
        if((r = (int) write(fd. bufptr, left)) == -1){
            if(errno == EINTR) continue;
            return -1;
        }
        if(r == 0) return 0;
        left -= r;
        bufptr +=r;
    }

    return 1;
}

int sendInteger(int fd_sk, size_t n){
    if(writen(fd_sk, &n, sizeof(unsigned long)) == -1){
        fprintf(stderr, "An error occured on sending message lenght\n");
        return errno;
    }

    return 0;
}

size_t receiveInteger(int fd_sk){
    size_t n = 0;

    if(readn(fd_sk, &n, sizeof(unsigned long)) == -1){
        fprintf(stderr, "An error occured on reading message lenght\n");
        return errno;
    }

    return n;
}

int sendn(int fd_sk, void *msg, size_t lenght){
    if(sendInteger(fd_sk, lenght) != 0){
        fprintf(stderr, "Error on sending lenght of message\n");
        return errno;
    }

    if(writen(fd_sk, msg, lenght) == -1){
        fprintf(stderr, "An error occured on sending message\n");
        return errno;
    }

    return 0;
}

int sendFile(int fd_sk, const char *pathname){
    FILE *file = fopen(pathname, "rb");
    if(file == NULL){
            return -1;
    }

    size_t fileSize = file_getsize(file);

    if(sendInteger(fd_sk, fileSize) != 0){
        fprintf(stderr, "Error on sending dimension of file");
        return errno;
    }
    void *fileContent = file_readAll(file);
    if(fileContent == NULL){
        fclose(file);
        return -1;
    }

    if(writen(fd_sk, fileContent, fileSize) == -1){
        fprintf(stderr, "An error occured on sending file\n");
        return errno;
    }

    free(fileContent);

    return 0;
}

void receiveFile(int fd_sk, void** buff, size_t* lenght){
    size_t size = receiveInteger(fd_sk);
    *lenght = size;

    *buff = malloc(size * sizeof(char));
    if(*buff == NULL){
        fprintf(stderr, "Impossible receivem a file: full of memory\n");
        exit(errno);
    }
    if(readn(fd_sk, *buff, size) == -1){
        fprintf(stderr, "An error occured reading file\n");
        exit(errno);
    }
}

void sendStr(int to, char* msg){
    sendn(to, msg, (int)strlen(msg));
}

char* receiveStr(int from){
    size_t lenght = receiveInteger(from) * sizeof(char);

    char* buff = calloc(lenght+1, sizeof(char));
    if(buff == NULL){
        return NULL;
    }
    if(readn(from, buff, lenght) == -1){
        fprintf(stderr, "An error occured on reading message\n");
        return NULL;
    }
    buff[lenght] = '\0';

    return (char*) buff;
}