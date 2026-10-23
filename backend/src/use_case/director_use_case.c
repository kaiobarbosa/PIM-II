#include "backend/src/infrasctructure/repository/director_repository.h"
#include "backend/src/domain/director_domain.h"
#include "backend/src/utils/generate_id.h"
#include <stdio.h>
#include <stdlib.h>

int check_first_use() {
    
    int number_directors = check_director_exists();

    if (number_directors == -1) {
        return -1;
    } else if (number_directors == 0) {
        return 1; 
    } else {
        return 0; 
    }
}

int create_director(char name_director[], char last_name_director[], char internal_code_director[], char email_director[], char password_director[]) {
    
    char id_director[7];
    generate_id(id_director);

    if (verify_null_fields(name_director, last_name_director, internal_code_director, email_director, password_director) == 0) {
        return -1; 
    }else if (verify_internal_code_format(internal_code_director) == 0) {
        return -1; 
    }else if (verify_email_format(email_director) == 0) {
        return -1; 
    }

    return 0;

}