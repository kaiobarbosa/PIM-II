#include "backend/config/database_connector.h"
#include "backend/src/infrastructure/repository/director_repository.h"
#include <stdio.h>

int create_new_address(const char *id, const char *cep, const char *logradouro, const char *numero, const char *complemento, const char *bairro, const char *cidade, const char *estado) {
    MYSQL *conn = connect_to_database();
    if (conn == NULL) {
        fprintf(stderr, "Erro ao conectar ao banco de dados.\n");
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query),  "INSERT INTO endereco (id_endereco, cep, logradouro, numero, complemento, bairro, cidade, estado) VALUES ('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s')", id, cep, logradouro, numero, complemento, bairro, cidade, estado);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao criar endereço: %s\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    mysql_close(conn);
    return 0;
}

int create_branch(const char *id_branch, const char *id_address, const char *id_user) {
    MYSQL *conn = connect_to_database();
    if (conn == NULL) {
        fprintf(stderr, "Erro ao conectar ao banco de dados.\n");
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query),  "INSERT INTO filial (id_filial, id_endereco, id_diretor) VALUES ('%s', '%s', '%s')", id_branch, id_address, id_user);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao criar filial: %s\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    mysql_close(conn);
    return 0;
}