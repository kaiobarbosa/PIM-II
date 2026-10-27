#include "backend/config/database_connector.h"
#include "backend/src/infrastructure/repository/manager_repository.h"
#include <stdio.h>

int check_branch_exists(const char *branch_code) {
    MYSQL *conn = connect_to_database();
    if (conn == NULL) {
        fprintf(stderr, "Erro ao conectar ao banco de dados.\n");
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query), "SELECT COUNT(*) FROM filial WHERE id_filial = '%s'", branch_code);
    int count = -1;
    
    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao buscar filiais: %s\n", mysql_error(conn));
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

int create_manager(const char *id, const char *name, const char *last_name, const char *internal_code, const char *branch, const char *email, const char *password, const char *id_director) {
    MYSQL *conn = connect_to_database();
    if (conn == NULL) {
        fprintf(stderr, "Erro ao conectar ao banco de dados.\n");
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query),  "INSERT INTO gerente (id_gerente, nome_gerente, sobrenome_gerente, codigo_interno, email_gerente, senha_gerente, id_filial, id_diretor) VALUES ('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s')", id, name, last_name, internal_code, email, password, branch, id_director);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao criar gerente: %s\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    mysql_close(conn);
    return 0;
}