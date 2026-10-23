#ifndef DIRECTOR_REPOSITORY_H
#define DIRECTOR_REPOSITORY_H

#include <mysql/mysql.h>

int check_director_exists();

int create_new_director(const char *id, const char *name, const char *last_name, const char *internal_code, const char *email, const char *password);

#endif

