#include "backend/src/use_case/branch_use_case.h"
#include <stdio.h>
#include <stdlib.h>

int register_new_branch(char id_user[], char cep[], char logradouro[], char numero[], char complemento[], char bairro[], char cidade[], char estado[]) {
    
    if (create_new_branch(id_user, cep, logradouro, numero, complemento, bairro, cidade, estado) == -1) {
        fprintf(stderr, "Erro ao registrar nova filial.\n");
        return -1;
    }   

    return 0;
}