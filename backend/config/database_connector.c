#include "database_connector.h"
#include <stdio.h>

MYSQL* connect_to_database() {
    MYSQL *conn = mysql_init(NULL);

    if (conn == NULL) {
        fprintf(stderr, "Erro ao inicializar o MySQL\n");
        return NULL;
    }

    if (mysql_real_connect(conn, "localhost", "root", "kaio2106", "pharma_database", 3306, NULL, 0) == NULL) {
        fprintf(stderr, "Erro na conexão: %s\n", mysql_error(conn));
        mysql_close(conn);
        return NULL;
    }

    printf("Conexão com o banco realizada com sucesso!\n");
    return conn;
}
