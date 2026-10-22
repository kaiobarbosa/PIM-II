#include "backend/src/infrasctructure/repository/director_repository.h"
#include <stdio.h>

int check_first_use() {
    printf("Verificando se é o primeiro uso do sistema...\n");
    
    
    int number_directors = check_director_exists();

    if (number_directors == -1) {
        printf("Falha na validação devido a um erro de banco de dados.\n");
        return -1;
    } else if (number_directors == 0) {
        printf("Nenhum diretor cadastrado. Iniciando fluxo de primeiro acesso...\n");
        return 1; 
    } else {
        printf("Sistema já possui %d diretor(es). Seguindo para login normal...\n", number_directors);
        return 0; 
    }
}