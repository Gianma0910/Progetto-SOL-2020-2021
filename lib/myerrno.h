#include <errno.h>
#include <stdarg.h>

#define SUCCESS 0
#define FILE_ALREADY_EXIST 1
#define FILE_NOT_FOUND 2
#define FILE_ALREADY_OPENED 3
#define FILE_NOT_OPENED 4
#define FILE_NOT_EMPTY 5
#define STORAGE_EMPTY 6
#define FILE_FOUND_ON_EXIT 7
#define SOCKET_ALREADY_CLOSED 8
#define STORAGE_FULL 9
#define EOS_F 10
#define HASH_NULL_PARAM 11
#define HASH_INSERT_SUCCESS 12
#define HASH_DUPLICATE_KEY 13
#define HASH_KEY_NOT_FOUND 14
#define FILE_TOO_LARGE 15
#define FILE_OPENED 16
#define CONNECTION_TIMED_OUT 17
#define WRONG_SOCKET 18
#define SOCKET_NOT_FOUND 19
#define INVALID_ARG 20
#define FREE_ERROR 21
#define CONNECTION_REFUSED 22
#define CONNECTION_ACCEPTED 23
#define MALLOC_ERROR 24

#ifndef PROGETTO_MYERRNO_H
#define PROGETTO_MYERRNO_H
#define PROGETTO_MYERRNO_H
#define RD    "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

enum Color {RED='r', GREEN='g', YELLOW='y', BLUE='b', MAGENTA='m', CYAN='c', WHITE='w', STANDARD='d'};

static void pcolor(enum Color c, char* s, ...){
    va_list argp;
    va_start(argp, s);
    char* p;
    switch (c) {
        case RED:
            p = str_concat(RD, s);
            break;
        case GREEN:
            p = str_concat(GRN, s);
            break;
        case YELLOW:
            p = str_concat(YEL, s);
            break;
        case BLUE:
            p = str_concat(BLU, s);
            break;
        case MAGENTA:
            p = str_concat(MAG, s);
            break;
        case CYAN:
            p = str_concat(CYN, s);
            break;
        case WHITE:
            p = str_concat(WHT, s);
            break;
        case STANDARD:
            p = str_concat(RESET, s);
            break;
    }

    vprintf(p, argp);
    va_end(argp);
    free(p);
    printf(RESET);
}

static void psucc(char* s, ...){
    va_list argp;
    va_start(argp, s);
    char* p = str_concat(GRN, s);
    vprintf(p, argp);
    va_end(argp);
    free(p);
    printf(RESET);
}

static void pwarn(char* s, ...){
    va_list argp;
    va_start(argp, s);
    char* p = str_concat(YEL, s);
    vprintf(p, argp);
    va_end(argp);
    free(p);
    printf(RESET);
}

static void perr(char* s, ...){
    va_list argp;
    va_start(argp, s);
    char* p = str_concat(RD, s);
    vprintf(p, argp);
    va_end(argp);
    free(p);
    printf(RESET);
}

static void pcode(int code, char* file){
    if(code == SUCCESS)
        return;

    if(file == NULL)
        file = "(null)";

    switch (code) {
        case FILE_ALREADY_EXIST:{
            perr("ERROR: File %s is already into the server\n"
                 "Code: FILE_ALREADY_EXIST\n\n", file);
            break;
        }
        case FILE_NOT_FOUND:{
            perr("ERROR: File %s not found into the server\n"
                 "Code: FILE_NOT_FOUND\n\n", file);
            break;
        }
        case FILE_ALREADY_OPENED:{
            printf(YEL "WARNING: File %s is already opened\n"
                 "Code: FILE_ALREADY_OPENED\n\n", file);
            break;
        }
        case FILE_NOT_OPENED:{
            perr("ERROR: File %s is not opened\n"
                 "Writing operations not allowed on closed files\n"
                 "Code: FILE_NOT_OPENED\n\n", file);
            break;
        }
        case FILE_NOT_EMPTY:{
            perr("ERROR: Writing operations not allowed on not empty files\n"
                 "File: %s\n"
                 "Code: FILE_NOT_EMPTY\n\n", file);
            break;
        }
        case STORAGE_EMPTY:{
            printf(YEL "WARNING: Server is empty\n"
                       "Code: STORAGE_EMPTY\n\n");
            break;
        }
        case FILE_FOUND_ON_EXIT:{
            printf(YEL "WARNING: Some file were not closed"
                       "The server will close the files\n"
                       "Code: FILE_FOUND_ON_EXIt\n\n");
            break;
        }
        case SOCKET_ALREADY_CLOSED:{
            perr("ERROR: Socket is already closed\n"
                 "Code: SOCKET_ALREADY_CLOSED\n\n");
            break;
        }
        case FILE_TOO_LARGE:{
            perr("ERROR: File %s is too large\n"
                 "Code: FILE_TOO_LARGE\n\n", file);
            break;
        }
        case FILE_OPENED:{
            printf(YEL "WARNING: Attempted operations (maybe delete ?) on opened file\n"
                       "You must close the file, for security issues\n"
                       "Code: FILE_OPENED\n\n");
            break;
        }
        case WRONG_SOCKET:{
            perr("ERROR: Socket argumento is not valid\n"
                 "Code: WRONG_SOCKET");
            break;
        }
        case SOCKET_NOT_FOUND:{
            perr("ERROR: File %s not found\n"
                 "Code: SOCKET_NOT_FOUND\n\n", file);
            break;
        }
        case INVALID_ARG:{
            perr("ERROR: Not valid argument\n"
                 "Code: INVALID_ARG\n\n");
            break;
        }
        case FREE_ERROR:{
            perr("ERROR: Impossible to free space into the server\n"
                 "Try to close some file\n"
                 "Code: FREE_ERROR\n\n");
            break;
        }
        case CONNECTION_REFUSED:{
            perr("ERROR: Impossible to establish a connection with the server\n"
                 "Code: CONNECTION_REFUSED\n\n");
            break;
        }
        case MALLOC_ERROR:{
            perr("ERROR: A malloc on the server failed\n"
                 "Try to delete some file, and then try again\n"
                 "Code: MALLOC_ERROR\n\n");
            break;
        }
        default:{
            break;
        }
    }

}

#undef GRN;
#undef YEL;
#undef WHT;
#undef RD;
#undef RESET;
#endif //PROGETTO_MYERRNO_H