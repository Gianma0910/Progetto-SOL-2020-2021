#define O_CREATE 1
#define O_LOCK 2
#define O_OPEN 3

#ifndef CLIENT_API_H
#define CLIENT_API_H

int openConnection(const char *sockname, int msec, const struct timespec abstime);
int closeConnection(const char *sockname);
int openFile(const char* pathname, int flags);
int readFile(const char *pathname, viod **buffer, size_t size);
int readNfiles(int N, const char* dirname);
int writeFile(const char *pathname, const char *dirname);
int closeFile(const char *pathname);
int removeFile(const char *pathname);
int appendToFile(const char *pathname, void *buf, size_t size, const char *dirname);
#endif
