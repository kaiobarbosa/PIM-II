#ifndef BRANCH_REPOSITORY_H
#define BRANCH_REPOSITORY_H

#include <mysql/mysql.h>

// 1. Criamos um "molde" para guardar os dados da filial que vêm do banco
typedef struct {
    char id_filial[7];
    char cep[9];
    char cidade[51];
    char estado[51];
} BranchItem;

int create_new_address(const char *id, const char *cep, const char *logradouro, const char *numero, const char *complemento, const char *bairro, const char *cidade, const char *estado);
int create_branch(const char *id_branch, const char *id_address, const char *id_user);

// 2. Assinatura da nova função
// Ela vai retornar o número de filiais encontradas, e vai preencher o array 'branches'
int select_all_branches(BranchItem *branches, int max_branches);

#endif
