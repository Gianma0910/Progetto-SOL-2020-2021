#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include "lib/client_api.h"
#include "lib/conn.h"
#include "lib/file_reader.h"
#include "lib/my_string.h"

char* sockname = NULL;
int time_sleep = 0;
bool p_op = false;
struct timespec abstime;

void sendfile_toServer(const char* backup_folder, char* file){
    int errcode;

    if(openFile(file, O_CREATE) != 0){
        errcode = errno;
        printf("%d\n", errcode);
    }
    if(p_op)
        printf("Sending %s...\n", file);
    if(writeFile(file, backup_folder) != 0){
        fprintf(stderr, "writeFile: Not possible to send file %s to server\n", file);
        errcode = errno;
        printf("%d\n", errcode);
    }else if(p_op){
        printf("File %s sent successfully\n", strrchr(file, '/')+1);
    }

    char* file_path = realpath(file, NULL);
    closeFile(file_path);
    free(file_path);
}

void print_possible_command(){
    printf("\t-h \t\tPrints this helper.\n");
    printf("\t-f <sockname> \tSets socket name to <sockname>. \033[0;31m This option must be set once and only once. \033[0m\n");
    printf("\t-p \t\t\tIf set, every operation will printed to stdout. \033[0;31m This option must be set once and only once. \033[0m\n");
    printf("\t-t <time>\t\tSets the waiting time (in milliseconds) between requests. Default is 0.\n");
    printf("\t-a <time>\t\tSets the time (in seconds) after which the application will stop attempting to connect to server. Default value is 5 seconds. \033[0;31m This option must be set once and only once. \033[0m\n");
    printf("\t-w <dirname> <num_file>\t\tSends the content of directory <dirname> to the server. if <num_file> is not specified the client hasn't limit of file to send, else client must send <num_file> to server.\n");
    printf("\t-W [file1, file2,...]\t\tSends the files passed as argument to the server.\n");
    printf("\t-D <dirname>\t\tWrites into directory <dirname> all the files expelled by the server. \033[0;31m This option must be set once and only once. \033[0m\n");
    printf("\t-d <dirname>\t\tWrites into directory <dirname> the file from the server. If not specified, files read from server will be lost. \033[0;31m This option must be set once and only once. \033[0m\n");
    printf("\t-R <num_file>\t\tReads <num_file> files from server. If <num_file> is not specified, reads all the files from the server. \033[0;31m This option must be set once and only once. \033[0m\n");
    printf("\t-r [file1, file2,...]\t\tRead the files specified in the argument list from the server.\n");
    printf("\t-l [file1, file2,...]\t\tLocks all the files given in the file list.\n");
    printf("\t-u [file1, file2,...]\t\tUnlocks all the files given in the file list.\n");
    printf("\t-c [file1, file2,...]\t\tDeletes fomr the server all the file given in the list, if they exist.\n");
    printf("\n");
}

int main(int argc, char* argv[]){
    char* download_folder;
    char* backup_folder;
    char* my_argv[argc];
    int my_argc = 0;
    int error_code;
    bool found = false;
    bool found_r = false;
    bool found_w = false;

    for(int i = 0; i < argc; i++){
        if(str_startsWith(argv[i], "-f")){
            found = true;
            sockname = ((argv[i])+=2);
            if(openConnection(sockname, 0, abstime) != 0){
                error_code = errno;
                printf("%d\n", error_code);
                exit(errno);
            }
        }else if(str_startsWith(argv[i], "-d")){
            download_folder = ((arg[i])+=2);
        }else if(str_startsWith(argv[i], "-D")){
            backup_folder = ((argv[i])+=2);
        }else if(str_startsWith(argv[i], "-p")){
            p_op = true;
        }else if(str_startsWith(argv[i], "-t")){
            str_toInteger(&time_sleep, (argv[i])+=2);
        }else{
            if(str_startsWith(argv[i], "-r") || str_startsWith(argv[i], "-R")){
                found_r = true;
            }else if(str_startsWith(argv[i], "-w") || str_startsWith(argv[i], "-W")){
                found_w = true;
            }
            my_argv[my_argc] = argv[i];
            my_argc++;
        }
    }

    if(!found){
        perror("Socket option -f <sockname> not specified\n");
        perror("Insert the server to connect to\n");
        return -1;
    }

    if(!found_r && download_folder != NULL){
        perror("Option -d must be used with option -r or -R");
    }

    if(!found_w && backup_folder != NULL){
        perror("Option -D must be used with option -w or -W");
    }

    int option;
    while((opt = getopt(my_argc, my_argv, ":h:w:W:r:R:c:")) != -1){
        switch (opt) {
            case 'h':{
                print_possible_command();
                break;
            }
            case 'w': {
                char** array = NULL;
                char** files = NULL;
                int num_files = -1;
                int count;
                int n = str_split(&array, optarg, ",");
                if(n > 2){
                    printf("Too much argument for the command -w dirname[,n=x]\n");
                    break;
                }
                if(n == 2){
                    if(str_toInteger(&num_files, array[1]) != 0){
                        printf("%s Is not a number\n", optarg);
                        break;
                    }
                }

                count = file_nscanAllDir(&files, array[0], num_files);
                for(int i = 0; i < count; i++){
                    sendfile_toServer(backup_folder, files[i]);
                    usleep(time_sleep*1000);
                }
                str_clearArray(&array, n);
                str_clearArray(&files, count);
                break;
            }
            case 'W': {
                char** files = NULL;
                int n = str_split(&files, optarg, ",");
                for(int i = 0; i < n; i++){
                    sendfile_toServer(backup_folder, files[i]);
                    usleep(time_sleep*1000);
                }
                str_clearArray(&files, n);
                break;
            }
            case 'r': {
                char **files = NULL;
                int n = str_split(&files, optarg, ",");
                void* buffer;
                size_t size;
                for(int i = 0; i < n; i++){
                    if(openFile(files[i], O_OPEN) != 0){
                        if(readFile(files[i], &buffer, &size) != 0){
                            printf("ReadFile: Error in file %s\n", files[i]);
                            error_code = errno;
                            printf("%d", error_code);
                        }else{
                            char* file_name = strrchr(files[i], '/')+1;
                            if(download_folder != NULL){
                                if(!str_endsWith(download_folder, "/")){
                                    download_folder = str_concat(download_folder, "/");
                                }
                                char* path = str_concatn(download_folder, file_name, NULL);
                                FILE *file = fopen(path, "wb");
                                if(file == NULL){
                                    printf("Error in path %s\n", path);
                                    download_folder = NULL;
                                }else{
                                        if(fwrite(buffer, sizeof(char), size, file) == 0){
                                            printf("Error in writing to path %s, other file will be ignored\n", path);
                                            download_folder = NULL;
                                        }else if(p_op){
                                            printf("Write into the directory %s\n", file_name);
                                            printf("%s\n\n", path);
                                        }
                                        fclose(file);
                                }
                                free(path);
                            }else if(p_op){
                                printf("File received %s\n", file_name);
                            }
                            free(buffer);
                        }
                        if(closeFile(files[i]) != 0){
                            printf("CloseFile: error in closing the file %s\n", files[i]);
                            error_code = errno;
                            printf("%d\n", error_code);
                        }
                    }else{
                        printf("OpenFile: Error in opening the file %s\n", files[i]);
                        error_code = errno;
                        printf("%d\n", error_code);
                    }
                    usleep(time_sleep*1000);
                }
                str_clearArray(&files, n);
                break;
            }
            case 'R': {
                int n = 0;
                if(optarg != NULL){
                    optarg++;
                    if(str_toInteger(&n, optarg) != 0){
                        printf("%s is not a number\n", optarg);
                        break;
                    }
                }
                if(readNfiles(n, download_folder) != 0){
                    error_code = errno;
                    printf("%d\n", error_code);
                }else if(p_op){
                    printf("Received %d file\n", n);
                }
                break;
            }
            case 'c':{
                char** files = NULL;
                int n = str_split(&files, optarg, ",");
                for(int i = 0; i < n; i++){
                    if(removeFile(files[i]) != 0){
                        error_code = errno;
                        printf("RemoveFile: file error %s\n", files[i]);
                        printf("%d\n", error_code);
                    }else if(p_op){
                        printf("File %s successfully removed\n\n", files[i]);
                    }
                    usleep(time_sleep*1000);
                }
                str_clearArray(&files, n);
                break;
            }
            default:{
                printf("Option not supported\n");
                break;
            }
        }

    }

    if(closeConnection(sockname) != 0){
        error_code = errno;
        printf("%d\n", error_code);
    }

    return 0;
}