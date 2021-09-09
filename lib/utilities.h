#ifndef PROGETTO_UTILITIES_H
#define PROGETTO_UTILITIES_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int unix_socket(char* path);
int socket_bind(int fd_sk, char* path);
int socket_accept(int fd_sk);
size_t readn(int fd, void* buffer, size_t size);
int writen(int fd, void* buffer, size_t size);
int sendInteger(int fd_sk, unsigned long n);
size_t receiveInteger(int fd_sk);
int sendn(int fd_sk, void* message, size_t length);
int sendFile(int fd_sk, const char* pathname);
void receiveFile(int fd_sk, void** buffer, size_t* size);
void sendStr(int to, char* message);
char* receiveStr(int from);
#endif //PROGETTO_UTILITIES_H
