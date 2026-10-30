#include "backend/src/infrastructure/repository/director_repository.h"
#include "backend/src/domain/director_domain.h"
#include "backend/src/utils/generate_id.h"
#include "backend/src/domain/validation/password_crypt.h" 
#include "backend/src/utils/verify_user_informations.h"
#include "backend/src/infrastructure/repository/login_repository.h"

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
    do {
        generate_id(id_director);
    } while (check_user_id_exist(id_director) == 1);

    if (verify_null_fields(name_director, last_name_director, internal_code_director, email_director, password_director) == 0) {
        return -1; 
    }else if (verify_internal_code_format(internal_code_director) == 0) {
        return -1; 
    }else if (verify_email_format(email_director) == 0) {
        return -1; 
    }

    char password_hashed[64]; 

    if (password_hash(password_director, password_hashed, sizeof(password_hashed)) == 0) {
        fprintf(stderr, "Erro ao criptografar a senha do diretor.\n");
        return -1;
    }

    if (create_new_director(id_director, name_director, last_name_director, internal_code_director, email_director, password_hashed) == -1) {
        fprintf(stderr, "Erro ao criar o diretor no banco de dados.\n");
        return -1;
    }else {
        printf("Diretor criado com sucesso!\n");
    }

    return 0;
}