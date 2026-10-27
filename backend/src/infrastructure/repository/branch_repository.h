#ifndef BRANCH_REPOSITORY_H
#define BRANCH_REPOSITORY_H

#include <mysql/mysql.h>

int create_new_address(const char *id, const char *cep, const char *logradouro, const char *numero, const char *complemento, const char *bairro, const char *cidade, const char *estado);

int create_branch(const char *id_branch, const char *id_address, const char *id_user);

#endif
