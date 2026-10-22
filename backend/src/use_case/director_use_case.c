#include "backend/src/infrasctructure/repository/director_repository.h"
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

void create_director() {
    printf("Criando o primeiro diretor do sistema...\n");
    

}