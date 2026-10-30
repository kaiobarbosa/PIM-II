#include <stdio.h>
#include "backend/src/use_case/login_use_case.h"

int login_presentation(char *email, char *password, char logged_id[7],
                       char *logged_profile) {

    int result = login_use_case(email, password, logged_id, logged_profile);

    if (result != 1) {
        fprintf(stderr, "Falha no login. Verifique suas credenciais.\n");
        return 0; 
    }

    return 1;
}

int get_all_users_info_presentation(UserInfoItem *users_list, int max_users) {
    
    int result = get_all_users_info_use_case(users_list, max_users);
    
    if (result == -1) {
        fprintf(stderr, "Erro na camada de apresentacao ao buscar a lista de usuarios.\n");
        return -1;
    }
    
    return result;
}