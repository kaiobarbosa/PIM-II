// Agora você inclui os .h em vez do .c
#include "query_executor.h"
#include "../../../config/database_connector.h" // Ajuste o caminho conforme as suas pastas
#include <stdio.h>

void execute_query(MYSQL *conn, const char *query) {
    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro na execução da query: %s\n", mysql_error(conn));
        return;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    
    if (result == NULL) {
        if (mysql_field_count(conn) == 0) {
            printf("Query executada com sucesso. Linhas afetadas: %llu\n", mysql_affected_rows(conn));
        } else {
            fprintf(stderr, "Erro ao resgatar os resultados: %s\n", mysql_error(conn));
        }
        return;
    }

    int num_columns = mysql_num_fields(result);
    MYSQL_ROW row;

    while ((row = mysql_fetch_row(result))) {
        for(int i = 0; i < num_columns; i++) {
            printf("%s\t", row[i] ? row[i] : "NULL");
        }
        printf("\n");
    }

    mysql_free_result(result);
}
