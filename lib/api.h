#define O_OPEN 0
#define O_CREATE 1
#define O_LOCK 2

#ifndef PROGETTO_API_H
#define PROGETTO_API_H
int openConnection(const char* sockname, int msec, const struct timespec abstime);
int closeConnection(const char* sockname);
int openFile(const char* pathname, int flags);
int readFile(const char* pathname, void** buffer, size_t* size);
int readNFiles(int N, const char *dirname);
int writeFile(const char* pathname, const charé dirname);
int appendToFile(const char* pathname, void* buffer, size_t size, const char* dirname);
int closeFile(const char* pathname);
int removeFile(const char* pathname);
#endif //PROGETTO_API_H
