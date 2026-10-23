#include "backend/config/database_connector.h"
#include "backend/src/infrastructure/repository/director_repository.h"
#include <stdio.h>

int check_director_exists() {
    MYSQL *conn = connect_to_database();
    if (conn == NULL) {
        fprintf(stderr, "Erro ao conectar ao banco de dados.\n");
        return -1;
    }

    const char *query = "SELECT COUNT(*) FROM Diretor";
    int count = -1;
    
    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao buscar diretores: %s\n", mysql_error(conn));
    } else {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result != NULL) {
            MYSQL_ROW row = mysql_fetch_row(result);
            
            if (row && row[0]) {
                sscanf(row[0], "%d", &count); 
            }
            mysql_free_result(result);
        }
    }

    mysql_close(conn);
    return count;
}

int create_new_director(const char *id, const char *name, const char *last_name, const char *internal_code, const char *email, const char *password) {
    MYSQL *conn = connect_to_database();
    if (conn == NULL) {
        fprintf(stderr, "Erro ao conectar ao banco de dados.\n");
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query),  "INSERT INTO diretor (id_diretor, nome_diretor, sobrenome_diretor, codigo_interno, email_diretor, senha_diretor) VALUES ('%s', '%s', '%s', '%s', '%s', '%s')", id, name, last_name, internal_code, email, password);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao criar diretor: %s\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    mysql_close(conn);
    return 0;
}