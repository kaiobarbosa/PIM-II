#include "backend/config/env_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void load_env_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Aviso: Arquivo %s nao encontrado. Tentando usar variaveis de ambiente do sistema.\n", filename);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;

        char *delimiter = strchr(line, '=');
        if (delimiter != NULL) {
            *delimiter = '\0'; 
            char *key = line;
            char *value = delimiter + 1;

            value[strcspn(value, "\r\n")] = '\0';

            setenv(key, value, 1);
        }
    }
    fclose(file);
}
