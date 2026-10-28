#include <string.h>
#include <stdio.h>

#include "backend/src/domain/manager_domain.h"
#include "backend/src/utils/verify_user_informations.h"
#include "backend/src/infrastructure/repository/manager_repository.h"
#include "backend/src/utils/generate_id.h"
#include "backend/src/domain/validation/password_crypt.h"

int create_new_manager (char name_manager[], char last_name_manager[], char internal_code_manager[], char branch_manager[], char email_manager[], char password_manager[], char id_director[]) {
    
    if(verify_null_fields_manager(name_manager, last_name_manager, internal_code_manager, branch_manager, email_manager, password_manager, id_director) == 0) {
        fprintf(stderr, "Erro: Campos nulos detectados.\n");
        return -1;
    }else if(verify_internal_code_format(branch_manager) == 0) {
        fprintf(stderr, "Erro: Código da filial inválido.\n");
        return -1;
    }else if(verify_email_format(email_manager) == 0) {
        fprintf(stderr, "Erro: Formato de e-mail inválido.\n");
        return -1;
    }else if(verify_internal_code_format(internal_code_manager) == 0) {
        fprintf(stderr, "Erro: Código interno inválido.\n");
        return -1;
    }

    char id_manager[7];
    generate_id(id_manager);

    char password_hashed[64]; 
    if (password_hash(password_manager, password_hashed, sizeof(password_hashed)) == 0) {
        fprintf(stderr, "Erro ao criptografar a senha do gerente.\n");
        return -1;
    }

    if(check_branch_exists(branch_manager) == 0) {
        fprintf(stderr, "Erro: Filial não encontrada.\n");
        return -1;
    }else if(create_manager(id_manager, name_manager, last_name_manager, internal_code_manager, branch_manager, email_manager, password_hashed, id_director) == -1) {
        fprintf(stderr, "Erro ao criar gerente.\n");
        return -1;
    }


    return 0;
}