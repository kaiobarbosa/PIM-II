#ifndef LOGIN_USE_CASE_H
#define LOGIN_USE_CASE_H

#include "backend/src/infrastructure/repository/login_repository.h" // Necessário para a struct UserInfoItem

int login_use_case(char *email, char *password, char logged_id[7], char *logged_profile);

int get_all_users_info_use_case(UserInfoItem *users_list, int max_users);

#endif
