#include "backend/src/use_case/director_use_case.h"
#include <stdio.h>
#include <stdlib.h>

int register_new_director(char name_director[], char last_name_director[], char internal_code_director[], char email_director[], char password_director[]) {
    
    if (create_director(name_director, last_name_director, internal_code_director, email_director, password_director) == -1) {
        fprintf(stderr, "Erro ao registrar novo diretor.\n");
        return -1;
    }   

    return 0;
}