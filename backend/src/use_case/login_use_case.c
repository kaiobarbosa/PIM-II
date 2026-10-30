#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "backend/src/utils/verify_user_informations.h"
#include "backend/src/domain/validation/password_crypt.h"
#include "backend/src/infrastructure/repository/login_repository.h"

// Adicionamos os ponteiros logged_id e logged_profile para enviar ao frontend
int login_use_case(char *email, char *password, char logged_id[7],
                   char *logged_profile) {

    if (verify_email_format(email) == 0) {
        fprintf(stderr, "Formato de email inválido.\n");
        return -1; 
    }

    char hash_armazenado[64] = ""; 
    
    // Passamos os ponteiros recebidos diretamente para o repositório preencher
    int usuario_encontrado = get_user_by_email(email, hash_armazenado, logged_profile, logged_id);

    if (usuario_encontrado == 0) {
        fprintf(stderr, "Usuário não encontrado.\n");
        return -1; 
    }

    if (password_verify(password, hash_armazenado) == 1) {
        printf("Acesso liberado! Autenticado como: %s (ID: %s)\n",
               logged_profile, logged_id);
        return 1; 
    } else {
        fprintf(stderr, "Senha incorreta.\n");
        return 0; 
    }
}

int get_all_users_info_use_case(UserInfoItem *users_list, int max_users) {
    
    int count = get_all_users_info(users_list, max_users);
    
    if (count == -1) {
        fprintf(stderr, "Erro de comunicação com o banco ao buscar a lista de usuarios.\n");
    }
    
    return count;
}