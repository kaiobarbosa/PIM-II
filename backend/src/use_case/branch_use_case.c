#include <stdio.h>
#include <string.h>

#include "backend/src/domain/branch_domain.h"
#include "backend/src/utils/generate_id.h"
#include "backend/src/use_case/branch_use_case.h"
#include "backend/src/infrastructure/repository/branch_repository.h"
#include "backend/src/infrastructure/repository/access_control_repository.h"
#include "backend/src/infrastructure/logging/csv_logger.h"

int create_new_branch(char id_user[], char cep[], char logradouro[], char numero[], char complemento[], char bairro[], char cidade[], char estado[]) {
    char profile[20] = "";
    char branch_id[7] = "";
    if (get_user_access_context(id_user, profile, sizeof(profile), branch_id) != 1 ||
        strcmp(profile, "diretor") != 0) {
        fprintf(stderr, "Erro: Apenas o diretor pode cadastrar filiais.\n");
        return -1;
    }

    if (verify_null_fields_branch(id_user, cep, logradouro, numero, complemento, bairro, cidade, estado) == 0) {
        fprintf(stderr, "Erro: Campos nulos detectados.\n");
        return -1;
    }

    if (validation_cep(cep) == 1) {
        fprintf(stderr, "Erro: CEP inválido.\n");
        return -1;
    } 

    char id_branch[7];
    generate_id(id_branch);

    char id_address[7];
    generate_id(id_address);

    if (create_new_address(id_address, cep, logradouro, numero, complemento, bairro, cidade, estado) == -1) {
        fprintf(stderr, "Erro ao criar endereço.\n");
        return -1;
    } else if (create_branch(id_branch, id_address, id_user) == -1) {
        fprintf(stderr, "Erro ao criar filial.\n");
        return -1;
    }

    char address_details[512];
    snprintf(address_details, sizeof(address_details),
             "logradouro=%s; numero=%s; complemento=%s; bairro=%s; cidade=%s; estado=%s",
             logradouro, numero, complemento, bairro, cidade, estado);
    ApplicationLogEntry log_entry = {
        .event = "branch_created",
        .entity = "branch",
        .entity_id = id_branch,
        .entity_name = "Filial",
        .actor_id = id_user,
        .branch_id = id_branch,
        .branch_cep = cep,
        .quantity = "",
        .details = address_details
    };
    if (append_application_log(&log_entry) != 0)
        fprintf(stderr, "Aviso: não foi possível registrar a criação da filial no CSV.\n");
    
    return 0;
}

int select_all_branches_use_case(BranchItem *branches, int max_branches) {
    int count = select_all_branches(branches, max_branches);
    
    if (count == -1) {
        fprintf(stderr, "Erro de comunicação com o banco ao buscar filiais.\n");
    }
    
    return count;
}
