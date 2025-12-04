#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define WIEN_SCRIPT_PATH "build.wien"
#define BUFFER_SIZE 1024

char* read_wien_script(const char* path, size_t* out_size) {
    if (!path) {
        fprintf(stderr, "[ERROR] Script path is NULL.\n");
        return NULL;
    }

    FILE* fp = fopen(path, "rb");
    if (!fp) {
        fprintf(stderr, "[ERROR] Failed to open '%s': %s\n", path, strerror(errno));
        return NULL;
    }

    // size buffer
    if (fseek(fp, 0, SEEK_END) != 0) {
        perror("[ERROR] fseek failed");
        fclose(fp);
        return NULL;
    }
    long size = ftell(fp);
    if (size < 0) {
        perror("[ERROR] ftell failed");
        fclose(fp);
        return NULL;
    }
    rewind(fp);

    // memory + 1 for '\0'
    char* buffer = (char*)malloc((size_t)size + 1);
    if (!buffer) {
        fprintf(stderr, "[ERROR] Out of memory reading '%s'\n", path);
        fclose(fp);
        return NULL;
    }

    // Read
    size_t bytes_read = fread(buffer, 1, (size_t)size, fp);
    if (bytes_read != (size_t)size) {
        fprintf(stderr, "[ERROR] Incomplete read of '%s'\n", path);
        free(buffer);
        fclose(fp);
        return NULL;
    }
    buffer[bytes_read] = '\0';

    fclose(fp);

    if (out_size) *out_size = (size_t)size;
    printf("[INFO] Successfully loaded script '%s' (%zu bytes).\n", path, (size_t)size);
    return buffer;
}

int parse_and_execute_wien(const char* source, size_t size) {
    if (!source || size == 0) {
        fprintf(stderr, "[ERROR] Empty script provided to parser.\n");
        return -1;
    }


    printf("[INFO] Script content (first 200 chars):\n%.200s%s\n",
           source, (size > 200) ? "..." : "");

    printf("[INFO] Parsing .wien script — ready for your syntax engine!\n");
    return 0;
}

int main(int argc, char* argv[]) {
    printf("Wien Builder v0.1n");
    printf("Target script: %s\n", WIEN_SCRIPT_PATH);

    size_t script_size = 0;
    char* script = read_wien_script(WIEN_SCRIPT_PATH, &script_size);
    if (!script) {
        fprintf(stderr, "[FATAL] Cannot proceed without build script.\n");
        return EXIT_FAILURE;
    }

    int result = parse_and_execute_wien(script, script_size);

    free(script);

    if (result == 0) {
        printf("\n[SUCCESS] Wien builder finished successfully!\n");
        return EXIT_SUCCESS;
    } else {
        fprintf(stderr, "\n[FAILURE] Build process failed with code %d.\n", result);
        return EXIT_FAILURE;
    }
}