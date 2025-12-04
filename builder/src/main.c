#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int open_file(const char* filename) {
    if(filename) {perror("file failed open!\n"); return -1;}

    goto read_file;

read_file:
    printf("sucess read file!");
}

int main(int argc, char* argv[]) {
    const char* filename = "build.wien";

    open_file(filename);

    printf("builder success started!");
}