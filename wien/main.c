#include <stdio.h>   // Для FILE, fopen, fclose, printf, perror, EOF
#include <stdlib.h>  // Для EXIT_FAILURE, EXIT_SUCCESS, system
#include <string.h>  // Для strchr, strncpy, strcmp, strlen, strcspn, strcat
#include <ctype.h>   // Для isspace

// Если wien.h не определит MAX_LINE_LENGTH, используем дефолт
#ifndef MAX_LINE_LENGTH
#define MAX_LINE_LENGTH 256
#endif
#ifndef MAX_SOURCES
#define MAX_SOURCES 32 // Максимальное количество исходных файлов
#endif
#ifndef MAX_SOURCE_PATH_LENGTH
#define MAX_SOURCE_PATH_LENGTH 64 // Максимальная длина пути к одному исходному файлу
#endif


// Структура для хранения настроек сборщика
typedef struct {
    char project_name[MAX_LINE_LENGTH];
    char source_dir[MAX_LINE_LENGTH];
    char build_dir[MAX_LINE_LENGTH];
    char compiler[MAX_LINE_LENGTH];
    char cflags[MAX_LINE_LENGTH];
    char ldflags[MAX_LINE_LENGTH];
    char target[MAX_LINE_LENGTH];

    // Для списка исходных файлов
    char sources[MAX_SOURCES][MAX_SOURCE_PATH_LENGTH];
    int num_sources;
} WienConfig;

// Вспомогательная функция для удаления пробелов в начале и в конце строки
char* trim_whitespace(char* str) {
    char* end;

    // Обрезать в начале
    while (isspace((unsigned char)*str)) str++;

    if (*str == 0) // Вся строка - пробелы
        return str;

    // Обрезать в конце
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    // Добавить нулевой символ к новой конечной позиции
    *(end + 1) = 0;

    return str;
}

// Функция для парсинга файла конфигурации сборщика
int parse_config_file(const char* filename, WienConfig* config) {
    FILE* fp = NULL;
    char line[MAX_LINE_LENGTH];
    int line_num = 0;
    int result = 0; // 0 - успех, -1 - ошибка

    // Инициализируем конфиг нулями
    memset(config, 0, sizeof(WienConfig));

    fp = fopen(filename, "r"); // "r" для текстового файла
    if (fp == NULL) {
        perror("Error opening config file");
        return -1;
    }

    printf("Successfully opened config file: %s\n", filename);

    while (fgets(line, sizeof(line), fp) != NULL) {
        line_num++;
        char* current_line = line;
        
        // Обрезаем символ новой строки в конце, если он есть
        current_line[strcspn(current_line, "\n")] = 0;

        // Игнорируем пустые строки и комментарии (начинающиеся с #)
        if (strlen(current_line) == 0 || current_line[0] == '#') {
            continue;
        }

        char* delimiter = strchr(current_line, '=');
        if (delimiter == NULL) {
            fprintf(stderr, "Warning: Skipping malformed line %d in config (no '='): %s\n", line_num, current_line);
            continue;
        }

        *delimiter = '\0'; // Разделяем ключ и значение, обрезая '='
        char* key = trim_whitespace(current_line);
        char* value = trim_whitespace(delimiter + 1);

        // Обрабатываем найденные ключ-значение пары
        if (strcmp(key, "PROJECT_NAME") == 0) {
            strncpy(config->project_name, value, sizeof(config->project_name) - 1);
            config->project_name[sizeof(config->project_name) - 1] = '\0'; 
        } else if (strcmp(key, "SOURCE_DIR") == 0) {
            strncpy(config->source_dir, value, sizeof(config->source_dir) - 1);
            config->source_dir[sizeof(config->source_dir) - 1] = '\0';
        } else if (strcmp(key, "BUILD_DIR") == 0) {
            strncpy(config->build_dir, value, sizeof(config->build_dir) - 1);
            config->build_dir[sizeof(config->build_dir) - 1] = '\0';
        } else if (strcmp(key, "COMPILER") == 0) {
            strncpy(config->compiler, value, sizeof(config->compiler) - 1);
            config->compiler[sizeof(config->compiler) - 1] = '\0';
        } else if (strcmp(key, "CFLAGS") == 0) {
            strncpy(config->cflags, value, sizeof(config->cflags) - 1);
            config->cflags[sizeof(config->cflags) - 1] = '\0';
        } else if (strcmp(key, "LDFLAGS") == 0) {
            strncpy(config->ldflags, value, sizeof(config->ldflags) - 1);
            config->ldflags[sizeof(config->ldflags) - 1] = '\0';
        } else if (strcmp(key, "TARGET") == 0) {
            strncpy(config->target, value, sizeof(config->target) - 1);
            config->target[sizeof(config->target) - 1] = '\0';
        } else if (strcmp(key, "SOURCES") == 0) {
            // Парсим список исходных файлов, разделенных пробелами
            char* token = strtok(value, " ");
            config->num_sources = 0;
            while (token != NULL && config->num_sources < MAX_SOURCES) {
                strncpy(config->sources[config->num_sources], token, MAX_SOURCE_PATH_LENGTH - 1);
                config->sources[config->num_sources][MAX_SOURCE_PATH_LENGTH - 1] = '\0';
                config->num_sources++;
                token = strtok(NULL, " ");
            }
        }
        else {
            fprintf(stderr, "Warning: Unknown config key '%s' on line %d\n", key, line_num);
        }
    }

    if (ferror(fp)) {
        perror("Error reading from config file");
        result = -1;
    }

    fclose(fp);
    printf("Finished parsing config file: %s\n", filename);
    return result;
}

// Функция для выполнения команды
void execute_command(const char* command) {
    printf("Executing command: %s\n", command);
    int status = system(command);
    if (status == -1) {
        perror("Error executing command");
    } else {
        printf("Command exited with status %d\n", WEXITSTATUS(status)); // WEXITSTATUS() из <sys/wait.h>
    }
}


int main(void) {
    printf("Wien builder successfully started!\n");

    const char* config_file = "wien.config"; // Имя файла конфигурации
    WienConfig my_config; // Создаем экземпляр нашей структуры конфига

    // 1. Парсим файл конфигурации
    if (parse_config_file(config_file, &my_config) != 0) {
        fprintf(stderr, "Failed to parse config file '%s'. Aborting.\n", config_file);
        return EXIT_FAILURE;
    }

    printf("\n--- Parsed Configuration ---\n");
    printf("Project Name: %s\n", my_config.project_name);
    printf("Source Dir:   %s\n", my_config.source_dir);
    printf("Build Dir:    %s\n", my_config.build_dir);
    printf("Compiler:     %s\n", my_config.compiler);
    printf("CFLAGS:       %s\n", my_config.cflags);
    printf("LDFLAGS:      %s\n", my_config.ldflags);
    printf("Target:       %s\n", my_config.target);
    printf("Sources (%d): \n", my_config.num_sources);
    for (int i = 0; i < my_config.num_sources; i++) {
        printf("  - %s\n", my_config.sources[i]);
    }
    printf("----------------------------\n");

    // 2. Создаем директорию для сборки, если она не существует
    char build_dir_cmd[MAX_LINE_LENGTH * 2];
    snprintf(build_dir_cmd, sizeof(build_dir_cmd), "mkdir -p %s", my_config.build_dir);
    execute_command(build_dir_cmd);

    // 3. Собираем команду для компиляции
    char compile_command[MAX_LINE_LENGTH * 4]; // Достаточно большой буфер для команды
    int offset = 0;

    // Компилятор
    offset += snprintf(compile_command + offset, sizeof(compile_command) - offset, "%s ", my_config.compiler);

    // Флаги CFLAGS
    offset += snprintf(compile_command + offset, sizeof(compile_command) - offset, "%s ", my_config.cflags);

    // Исходные файлы
    for (int i = 0; i < my_config.num_sources; i++) {
        offset += snprintf(compile_command + offset, sizeof(compile_command) - offset, "%s/%s ", my_config.source_dir, my_config.sources[i]);
    }

    // LDFLAGS и выходной файл
    offset += snprintf(compile_command + offset, sizeof(compile_command) - offset, "%s -o %s/%s", my_config.ldflags, my_config.build_dir, my_config.target);

    // 4. Выполняем команду компиляции
    execute_command(compile_command);
    
    printf("\nBuild process finished for project: %s\n", my_config.project_name);

    return EXIT_SUCCESS;
}
