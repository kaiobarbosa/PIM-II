#include "backend/config/database_connector.h"
#include <stdio.h>

int create_attendant(const char *id, const char *name, const char *last_name, const char *email, const char *password, const char *id_branch, const char *id_supervisor) {
    MYSQL *conn = connect_to_database();
    if (conn == NULL) {
        fprintf(stderr, "Erro ao conectar ao banco de dados.\n");
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query),  "INSERT INTO atendente (id_atendente, nome_atendente, sobrenome_atendente, email_atendente, senha_atendente, id_filial, id_gerente) values  ('%s', '%s', '%s', '%s', '%s', '%s', '%s')", id, name, last_name, email, password, id_branch, id_supervisor);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao criar atendente: %s\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    mysql_close(conn);
    return 0;
}