#ifndef BRANCH_USE_CASE_H
#define BRANCH_USE_CASE_H

#include "backend/src/infrastructure/repository/branch_repository.h"

int create_new_branch(char id_user[], char cep[], char logradouro[], char numero[], char complemento[], char bairro[], char cidade[], char estado[]);

int select_all_branches_use_case(BranchItem *branches, int max_branches);


#endif