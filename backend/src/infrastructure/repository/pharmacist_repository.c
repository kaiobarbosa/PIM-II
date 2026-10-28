#include "backend/config/database_connector.h"
#include "backend/src/infrastructure/repository/pharmacist_repository.h"
#include <stdio.h>

int check_id_supervisor(const char *id_supervisor) {
    MYSQL *conn = connect_to_database();
    if (conn == NULL) {
        fprintf(stderr, "Erro ao conectar ao banco de dados.\n");
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query), "SELECT COUNT(*) FROM gerente WHERE id_gerente = '%s'", id_supervisor);
    int count = -1;
    
    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao buscar supervisores: %s\n", mysql_error(conn));
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

int create_pharmacist(const char *id, const char *name, const char *last_name, const char *internal_code, const char *crf, const char *email, const char *password, const char *id_branch, const char *id_supervisor) {
    MYSQL *conn = connect_to_database();
    if (conn == NULL) {
        fprintf(stderr, "Erro ao conectar ao banco de dados.\n");
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query),  "INSERT INTO farmaceutico (id_farmaceutico, nome_farmaceutico, sobrenome_farmaceutico, codigo_interno, crf_farmaceutico, email_farmaceutico, senha_farmaceutico, id_filial, id_gerente) VALUES ('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s')", id, name, last_name, internal_code, crf, email, password, id_branch, id_supervisor);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao criar farmaceutico: %s\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    mysql_close(conn);
    return 0;
}