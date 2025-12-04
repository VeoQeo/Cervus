// read from file

/**
 * @brief main module for start binary with wien script builder
 * @version v0.1
 * @author Wienton Dev Corp
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// header
#include "include/protocol.h"
#include "directives/print/print.h"
#include "directives/sanitaizer/sanitaizer.h"
#include "directives/forloop/forloop.h"
#include "directives/ifelse/ifelse.h"
#include "directives/variable/var.h"
#include "directives/cmd/cmd.h"
#include "directives/arrays/array.h"

#define WIEN_SCRIPT_PATH "build.wien"
#define BUFFER_SIZE 1024

int read_from_file(const char* path_file) {

    FILE* fp = fopen(path_file, "rb");

    if(!fp){perror("error open file!"); return -1;}

    printf("sucess file opened! file: %s", path_file);

}

int main(int argc, char* argvp[]) {

    printf("init success\n");

    read_from_file(WIEN_SCRIPT_PATH);

}