#ifndef BRANCH_DOMAIN_H
#define BRANCH_DOMAIN_H

int verify_null_fields_branch(char id_user[], char cep[], char logradouro[], char numero[], char complemento[], char bairro[], char cidade[], char estado[]);

int validation_cep(const char *cep);

#endif