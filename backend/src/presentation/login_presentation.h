#ifndef LOGIN_PRESENTATION_H
#define LOGIN_PRESENTATION_H

#include "backend/src/infrastructure/repository/login_repository.h"

int login_presentation(char *email, char *password, char logged_id[7], char *logged_profile);

int get_all_users_info_presentation(UserInfoItem *users_list, int max_users);

#endif
