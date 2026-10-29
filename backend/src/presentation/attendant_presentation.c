#include "backend/src/use_case/attendant_use_case.h"
#include <stdio.h>
#include <stdlib.h>

int register_new_attendant(char name_attendant[], char last_name_attendant[], char email_attendant[], char password_attendant[],  char id_branch[],char id_supervisor[]) {
    
    if(create_new_attendant(name_attendant, last_name_attendant, email_attendant, password_attendant, id_branch, id_supervisor) == -1) {
        fprintf(stderr, "Erro ao criar atendente.\n");
        return -1;
    }
    return 0;
}