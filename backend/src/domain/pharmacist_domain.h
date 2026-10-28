#ifndef PHARMACIST_DOMAIN_H
#define PHARMACIST_DOMAIN_H

int verify_null_fields_pharmacist(char name[], char last_name[], char internal_code[], char crf[], char email[], char password[], char id_branch[], char id_supervisor[]);

int verify_branch_code(const char *branch_code);

#endif