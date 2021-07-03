#if !defined(CONN_H)
#define CONN_H

#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int unix_socket(char *path);
int socket_bind(int fd_sk, size_t size);
int socket_accept(int fd_sk);

size_t readn(long fd, void *buf, int size);
size_t writen(long fd, void *buf, int size);
int sendn(int fd_sk, void *msg, size_t lenght);
int sendFile(int fd_sk, const char *pathname);
void receiveFile(int fd_sk, void **buf, size_t *lenght);
void sendStr(int to, char *msg);
char* receiveStr(int from);
int sendInteger(int fd_sk, size_t n);
size_t receiveInteger(int fd_sk);
#endif
