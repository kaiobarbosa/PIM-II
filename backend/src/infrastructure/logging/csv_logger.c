#define _POSIX_C_SOURCE 200809L

#include "backend/src/infrastructure/logging/csv_logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

#define DEFAULT_APPLICATION_LOG_PATH "documentation/application_logs.csv"
#define APPLICATION_LOG_HEADER \
    "timestamp,event,entity,entity_id,entity_name,actor_id,branch_id,branch_cep,quantity,details\r\n"

static int write_csv_field(FILE *file, const char *value) {
    const unsigned char *character = (const unsigned char *)
        (value != NULL ? value : "");

    if (fputc('"', file) == EOF) return -1;
    while (*character != '\0') {
        if (*character == '"' && fputc('"', file) == EOF) return -1;
        if (fputc(*character++, file) == EOF) return -1;
    }
    return fputc('"', file) == EOF ? -1 : 0;
}

static int make_timestamp(char *timestamp, size_t capacity) {
    time_t now = time(NULL);
    struct tm local_time;
    char offset_timestamp[40];
    size_t length;

    if (now == (time_t)-1 || localtime_r(&now, &local_time) == NULL ||
        strftime(offset_timestamp, sizeof(offset_timestamp),
                 "%Y-%m-%dT%H:%M:%S%z", &local_time) == 0) {
        return -1;
    }

    length = strlen(offset_timestamp);
    if (length >= 5 &&
        (offset_timestamp[length - 5] == '+' ||
         offset_timestamp[length - 5] == '-')) {
        int written = snprintf(timestamp, capacity, "%.*s%c%c%c:%c%c",
                               (int)(length - 5), offset_timestamp,
                               offset_timestamp[length - 5],
                               offset_timestamp[length - 4],
                               offset_timestamp[length - 3],
                               offset_timestamp[length - 2],
                               offset_timestamp[length - 1]);
        return written >= 0 && written < (int)capacity ? 0 : -1;
    }

    {
        struct tm utc_time;
        if (gmtime_r(&now, &utc_time) == NULL ||
            strftime(timestamp, capacity, "%Y-%m-%dT%H:%M:%SZ",
                     &utc_time) == 0) {
            return -1;
        }
    }
    return 0;
}

static int lock_file(int file_descriptor, short lock_type) {
    struct flock lock = {0};
    lock.l_type = lock_type;
    lock.l_whence = SEEK_SET;
    return fcntl(file_descriptor,
                 lock_type == F_UNLCK ? F_SETLK : F_SETLKW, &lock);
}

int append_application_log(const ApplicationLogEntry *entry) {
    const char *path = getenv("APP_LOG_CSV");
    const char *fields[10];
    char timestamp[40];
    FILE *file;
    int field_index;
    int result = -1;
    long file_size;

    if (entry == NULL || entry->event == NULL || entry->entity == NULL ||
        entry->entity_id == NULL) {
        return -1;
    }
    if (path == NULL || path[0] == '\0')
        path = DEFAULT_APPLICATION_LOG_PATH;
    if (make_timestamp(timestamp, sizeof(timestamp)) != 0) return -1;

    file = fopen(path, "a+");
    if (file == NULL) {
        perror("Não foi possível abrir o CSV de logs");
        return -1;
    }
    if (lock_file(fileno(file), F_WRLCK) != 0) {
        perror("Não foi possível bloquear o CSV de logs");
        fclose(file);
        return -1;
    }

    if (fseek(file, 0, SEEK_END) != 0 || (file_size = ftell(file)) < 0) {
        perror("Não foi possível posicionar o CSV de logs");
        goto cleanup;
    }
    if (file_size == 0 && fputs(APPLICATION_LOG_HEADER, file) == EOF) {
        perror("Não foi possível escrever o cabeçalho do CSV de logs");
        goto cleanup;
    }

    fields[0] = timestamp;
    fields[1] = entry->event;
    fields[2] = entry->entity;
    fields[3] = entry->entity_id;
    fields[4] = entry->entity_name;
    fields[5] = entry->actor_id;
    fields[6] = entry->branch_id;
    fields[7] = entry->branch_cep;
    fields[8] = entry->quantity;
    fields[9] = entry->details;

    for (field_index = 0; field_index < 10; field_index++) {
        if ((field_index > 0 && fputc(',', file) == EOF) ||
            write_csv_field(file, fields[field_index]) != 0) {
            perror("Não foi possível escrever uma linha no CSV de logs");
            goto cleanup;
        }
    }
    if (fputs("\r\n", file) == EOF || fflush(file) != 0 ||
        fsync(fileno(file)) != 0) {
        perror("Não foi possível confirmar a gravação no CSV de logs");
        goto cleanup;
    }
    result = 0;

cleanup:
    lock_file(fileno(file), F_UNLCK);
    if (fclose(file) != 0) {
        perror("Não foi possível fechar o CSV de logs");
        result = -1;
    }
    return result;
}
