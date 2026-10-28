#include "backend/src/use_case/pharmacist_use_case.h"
#include <stdio.h>
#include <stdlib.h>

int register_new_pharmacist(char name_pharmacist[], char last_name_pharmacist[], char internal_code_pharmacist[], char crf[],char email_pharmacist[], char password_pharmacist[],  char id_branch[],char id_supervisor[]) {
    
    if(create_new_pharmacist(name_pharmacist, last_name_pharmacist, internal_code_pharmacist, crf, email_pharmacist, password_pharmacist, id_branch, id_supervisor) == -1) {
        fprintf(stderr, "Erro ao criar farmaceutico.\n");
        return -1;
    }
    return 0;
}