#ifndef BRANCH_PRESENTATION_H
#define BRANCH_PRESENTATION_H

#include "backend/src/infrastructure/repository/branch_repository.h" // Para enxergar a struct BranchItem

int register_new_branch(char id_user[], char cep[], char logradouro[], char numero[], char complemento[], char bairro[], char cidade[], char estado[]);

int select_all_branches_presentation(BranchItem *branches, int max_branches);

#endif
