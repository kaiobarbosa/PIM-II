#ifndef CSV_LOGGER_H
#define CSV_LOGGER_H

typedef struct {
    const char *event;
    const char *entity;
    const char *entity_id;
    const char *entity_name;
    const char *actor_id;
    const char *branch_id;
    const char *branch_cep;
    const char *quantity;
    const char *details;
} ApplicationLogEntry;

int append_application_log(const ApplicationLogEntry *entry);

#endif
