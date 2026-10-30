#ifndef LOGIN_REPOSITORY_H
#define LOGIN_REPOSITORY_H

#include <mysql/mysql.h>

typedef struct {
    char id_usuario[7];
    char nome[101];
    char filial[51];
    char cargo[20];
} UserInfoItem;

int get_user_by_email(const char *email, char *hash_armazenado, char *perfil_usuario, char id_usuario[7]);
int check_user_id_exist(const char *id_user);

int get_all_users_info(UserInfoItem *users_list, int max_users);

#endif
