#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "backend/src/utils/verify_user_informations.h"
#include "backend/src/domain/validation/password_crypt.h"
#include "backend/src/infrastructure/repository/login_repository.h"

int login_use_case(char *email, char *password) {

    if (verify_email_format(email) == 0) {
        fprintf(stderr, "Formato de email inválido.\n");
        return -1; 
    }

    char hash_armazenado[64] = ""; 
    char perfil_usuario[20] = "";  

    int usuario_encontrado = get_user_by_email(email, hash_armazenado, perfil_usuario);

    if (usuario_encontrado == 0) {
        fprintf(stderr, "Usuário não encontrado.\n");
        return -1; 
    }

    if (password_verify(password, hash_armazenado) == 1) {
        
        printf("Acesso liberado! Autenticado como: %s\n", perfil_usuario);
        return 1; 

    } else {
        
        fprintf(stderr, "Senha incorreta.\n");
        return 0; 
        
    }
}
