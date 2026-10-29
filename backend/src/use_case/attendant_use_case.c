#include <string.h>
#include <stdio.h>

#include "backend/src/domain/attendant_domain.h"
#include "backend/src/utils/verify_user_informations.h"
#include "backend/src/utils/generate_id.h"
#include "backend/src/domain/validation/password_crypt.h"
#include "backend/src/infrastructure/repository/manager_repository.h"
#include "backend/src/infrastructure/repository/pharmacist_repository.h"
#include "backend/src/infrastructure/repository/attendant_repository.h"

int create_new_attendant (char name_attendant[], char last_name_attendant[], char email_attendant[], char password_attendant[], char id_branch[], char id_supervisor[]) {

    if(verify_null_fields_attendant(name_attendant, last_name_attendant, email_attendant, password_attendant, id_branch, id_supervisor) == 0){
        fprintf(stderr, "Erro: Campos nulos detectados.\n");
        return -1;
    }else if(verify_internal_code_format(id_branch) == 0) {
        fprintf(stderr, "Erro: Código da filial inválido.\n");
        return -1;
    }else if(verify_email_format(email_attendant) == 0) {
        fprintf(stderr, "Erro: Formato de e-mail inválido.\n");
        return -1;
    }
    char id_attendant[7];
    generate_id(id_attendant);

    char password_hashed[64];
    if (password_hash(password_attendant, password_hashed, sizeof(password_hashed)) == 0) {
        fprintf(stderr, "Erro ao criptografar a senha do farmacêutico.\n");
        return -1;
    }else if(check_id_supervisor(id_supervisor) == 0) {
        fprintf(stderr, "Erro: Supervisor não encontrado.\n%s", id_supervisor);
        return -1;
    }else if(check_branch_exists(id_branch) == 0) {
        fprintf(stderr, "Erro: Filial não encontrada.\n");
        return -1;
    }else if(create_attendant(id_attendant, name_attendant, last_name_attendant, email_attendant, password_hashed, id_branch, id_supervisor) == -1) {
        fprintf(stderr, "Erro ao criar farmaceutico.\n");
        return -1;
    }
    return 0;
}