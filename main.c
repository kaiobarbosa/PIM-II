#include <ncurses.h>
#include <stdio.h>
#include "backend/config/database_connector.h"
#include "backend/src/infrasctructure/repository/query_executor.h"
#include "backend/src/use_case/director_use_case.h"

int main() {
    
    printf("Iniciando o sistema...\n");
    int first_use = check_first_use();

    printf("Resultado da verificação de primeiro uso: %d\n", first_use);

    return 0;
}