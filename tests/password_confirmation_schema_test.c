#include <stdio.h>

#include "backend/config/database_connector.h"

static int query_has_fields(MYSQL *conn, const char *query,
                            unsigned int expected_fields) {
    MYSQL_RES *result;
    int valid;

    if (mysql_query(conn, query)) return 0;
    result = mysql_store_result(conn);
    if (result == NULL) return 0;
    valid = mysql_num_fields(result) == expected_fields;
    mysql_free_result(result);
    return valid;
}

int main(void) {
    MYSQL *conn = connect_to_database();
    if (conn == NULL) {
        fprintf(stderr, "FAIL: could not connect to configured MySQL database.\n");
        return 2;
    }

    if (!query_has_fields(
            conn,
            "SELECT id_recuperacao, email_usuario, codigo_verificacao, "
            "criado_em FROM recuperacao_senha LIMIT 0",
            4)) {
        fprintf(stderr, "FAIL: recuperacao_senha table/schema is unavailable.\n");
        mysql_close(conn);
        return 1;
    }
    if (!query_has_fields(
            conn,
            "SELECT id_usuario, email, perfil FROM vw_login_usuarios LIMIT 0",
            3)) {
        fprintf(stderr, "FAIL: vw_login_usuarios view is unavailable.\n");
        mysql_close(conn);
        return 1;
    }

    mysql_close(conn);
    puts("PASS: MySQL exposes the confirmation table and login view schemas.");
    return 0;
}
