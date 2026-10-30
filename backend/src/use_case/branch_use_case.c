#include <stdio.h>
#include <string.h>

#include "backend/src/domain/branch_domain.h"
#include "backend/src/utils/generate_id.h"
#include "backend/src/infrastructure/repository/branch_repository.h"

int create_new_branch(char id_user[], char cep[], char logradouro[], char numero[], char complemento[], char bairro[], char cidade[], char estado[]) {
    
    if(verify_null_fields_branch(id_user, cep, logradouro, numero, complemento, bairro, cidade, estado) == 0) {
        fprintf(stderr, "Erro: Campos nulos detectados.\n");
        return -1;
    }

    if(validation_cep(cep) == 1) {
        fprintf(stderr, "Erro: CEP inválido.\n");
        return -1;
    } 

    char id_branch[7];
    generate_id(id_branch);

    char id_address[7];
    generate_id(id_address);

    if(create_new_address(id_address, cep, logradouro, numero, complemento, bairro, cidade, estado) == -1) {
        fprintf(stderr, "Erro ao criar endereço.\n");
        return -1;

    }else if(create_branch(id_branch, id_address, id_user) == -1) {
        fprintf(stderr, "Erro ao criar filial.\n");
        return -1;
    }
    
    return 0;
}

int select_all_branches_use_case(BranchItem *branches, int max_branches) {
    int count = select_all_branches(branches, max_branches);
    
    if (count == -1) {
        fprintf(stderr, "Erro de comunicação com o banco ao buscar filiais.\n");
    }
    
    return count;
}   