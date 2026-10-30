#ifndef PHARMACIST_REPOSITORY_H
#define PHARMACIST_REPOSITORY_H

#include <mysql/mysql.h>

int get_supervisor_profile(const char *id_supervisor, char *perfil_retornado);

int create_pharmacist(const char *id, const char *name, const char *last_name, const char *internal_code, const char *crf, const char *email, const char *password, const char *id_branch, const char *id_supervisor, const char *perfil_supervisor);

#endif
