#include <ncurses.h>
#include <stdio.h>
#include <unistd.h>
#include "backend/config/database_connector.h"
#include "backend/src/infrasctructure/repository/query_executor.h"
#include "backend/src/use_case/director_use_case.h"

int main() {
    
    int first_use = 0;

    printf("Iniciando o sistema...\n");
    
    do{
        first_use = check_first_use();

        if (first_use == -1) {
            printf("Falha na conexão com o banco de dados. Tentando novamente em 5 segundos...\n");
            sleep(5); 
        } else break;
        
    } while (first_use == -1);

   if(first_use == 1) {
        printf("Fluxo de primeiro acesso iniciado.\n");
        
    }else{
        printf("Fluxo de login normal iniciado.\n");
       
    }
    return 0;
}