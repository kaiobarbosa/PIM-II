#include <string.h>
#include <stdio.h>

#include "backend/src/domain/manager_domain.h"
#include "backend/src/utils/verify_user_informations.h"
#include "backend/src/infrastructure/repository/manager_repository.h"
#include "backend/src/utils/generate_id.h"
#include "backend/src/domain/validation/password_crypt.h"
#include "backend/src/infrastructure/repository/login_repository.h"
#include "backend/src/infrastructure/repository/access_control_repository.h"
#include "backend/src/infrastructure/repository/branch_repository.h"
#include "backend/src/infrastructure/logging/csv_logger.h"

int create_new_manager (char name_manager[], char last_name_manager[], char internal_code_manager[], char branch_manager[], char email_manager[], char password_manager[], char id_director[]) {
    char supervisor_profile[20] = "";
    char supervisor_branch[7] = "";
    if (get_user_access_context(id_director, supervisor_profile,
                                sizeof(supervisor_profile), supervisor_branch) != 1 ||
        strcmp(supervisor_profile, "diretor") != 0) {
        fprintf(stderr, "Erro: Apenas o diretor pode cadastrar gerentes.\n");
        return -1;
    }
    
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
    do {
        generate_id(id_manager);
    } while (check_user_id_exist(id_manager) == 1);

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

    char branch_cep[9] = "";
    char full_name[102];
    snprintf(full_name, sizeof(full_name), "%s %s", name_manager,
             last_name_manager);
    if (get_branch_cep_by_id(branch_manager, branch_cep,
                             sizeof(branch_cep)) != 1) {
        fprintf(stderr, "Aviso: não foi possível consultar o CEP da filial do gerente.\n");
    }
    ApplicationLogEntry log_entry = {
        .event = "user_created",
        .entity = "user",
        .entity_id = id_manager,
        .entity_name = full_name,
        .actor_id = id_director,
        .branch_id = branch_manager,
        .branch_cep = branch_cep,
        .quantity = "",
        .details = "profile=gerente"
    };
    if (append_application_log(&log_entry) != 0)
        fprintf(stderr, "Aviso: não foi possível registrar a criação do gerente no CSV.\n");

    return 0;
}
