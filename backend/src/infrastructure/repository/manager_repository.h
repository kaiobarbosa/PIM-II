#ifndef MANAGER_REPOSITORY_H
#define MANAGER_REPOSITORY_H

#include <mysql/mysql.h>

int check_branch_exists(const char *branch_code);

int create_manager(const char *id, const char *name, const char *last_name, const char *internal_code, const char *branch, const char *email, const char *password, const char *id_director);


#endif

