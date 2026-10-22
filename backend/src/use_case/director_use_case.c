#include "backend/src/infrasctructure/repository/director_repository.h"
#include <stdio.h>

int check_first_use() {
    printf("Verificando se é o primeiro uso do sistema...\n");
    
    // Captura a resposta enviada pelo repositório
    int total_diretores = check_director_exists();

    if (total_diretores == -1) {
        printf("Falha na validação devido a um erro de banco de dados.\n");
        return -1;
    } else if (total_diretores == 0) {
        printf("Nenhum diretor cadastrado. Iniciando fluxo de primeiro acesso...\n");
        return 1; // Indica que é o primeiro uso
    } else {
        printf("Sistema já possui %d diretor(es). Seguindo para login normal...\n", total_diretores);
        return 0; // Indica que não é o primeiro uso
    }
}