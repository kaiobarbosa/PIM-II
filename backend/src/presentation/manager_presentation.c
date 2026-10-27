#include "backend/src/use_case/manager_use_case.h"
#include <stdio.h>
#include <stdlib.h>

int register_new_manager(char name_manager[], char last_name_manager[], char internal_code_manager[], char branch_manager[], char email_manager[], char password_manager[]) {
    
    if (create_manager(name_manager, last_name_manager, internal_code_manager, branch_manager, email_manager, password_manager) == -1) {
        fprintf(stderr, "Erro ao registrar novo gerente.\n");
        return -1;
    }   

    return 0;
}