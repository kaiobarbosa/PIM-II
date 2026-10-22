#include "backend/config/database_connector.h"
#include "backend/src/infrasctructure/repository/director_repository.h"
#include <stdio.h>

int check_director_exists() {
    MYSQL *conn = connect_to_database();
    if (conn == NULL) {
        fprintf(stderr, "Erro ao conectar ao banco de dados.\n");
        return -1; // Retorna -1 para indicar que houve um erro de conexão
    }

    const char *query = "SELECT COUNT(*) FROM Diretor";
    int count = -1;
    
    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao buscar diretores: %s\n", mysql_error(conn));
    } else {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result != NULL) {
            MYSQL_ROW row = mysql_fetch_row(result);
            
            // Verifica se a linha existe e possui dados na primeira coluna
            if (row && row[0]) {
                sscanf(row[0], "%d", &count); // Converte o resultado de string para int
            }
            mysql_free_result(result);
        }
    }

    mysql_close(conn);
    return count;
}
