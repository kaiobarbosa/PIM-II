#include "backend/config/database_connector.h"
#include <stdio.h>

int create_product(const char *id, const char *nome, const char *marca, const char *descricao, const char *preco, const char *cod_barras, const char *quantidade, const char *filial) {
    MYSQL *conn = connect_to_database();
    if (conn == NULL) {
        fprintf(stderr, "Erro ao conectar ao banco de dados.\n");
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query),  "INSERT INTO produto (id_produto, nome_produto, marca_produto, descricao_produto, preco, codigo_de_barras, quantidade_produto, id_filial) values ('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s')", id, nome, marca, descricao, preco, cod_barras, quantidade, filial);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao criar produto: %s\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    mysql_close(conn);
    return 0;
}