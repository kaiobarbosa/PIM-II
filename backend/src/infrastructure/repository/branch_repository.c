#include "backend/config/database_connector.h"
#include "backend/src/infrastructure/repository/director_repository.h"
#include "backend/src/infrastructure/repository/branch_repository.h"
#include <stdio.h>
#include <string.h>

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

int select_all_branches(BranchItem *branches, int max_branches) {
    MYSQL *conn = connect_to_database();
    if (conn == NULL) return -1;

    const char *query =
        "SELECT f.id_filial, e.cep, e.logradouro, e.numero, "
        "COALESCE(e.complemento,''), e.bairro, e.cidade, e.estado "
        "FROM filial f INNER JOIN endereco e ON f.id_endereco=e.id_endereco";
    
    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao buscar filiais: %s\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (result == NULL) {
        mysql_close(conn);
        return 0;
    }

    MYSQL_ROW row;
    int count = 0;

    while ((row = mysql_fetch_row(result)) && count < max_branches) {
        snprintf(branches[count].id_filial,
                 sizeof(branches[count].id_filial), "%s",
                 row[0] != NULL ? row[0] : "");
        snprintf(branches[count].cep, sizeof(branches[count].cep), "%s",
                 row[1] != NULL ? row[1] : "");
        snprintf(branches[count].logradouro,
                 sizeof(branches[count].logradouro), "%s",
                 row[2] != NULL ? row[2] : "");
        snprintf(branches[count].numero, sizeof(branches[count].numero), "%s",
                 row[3] != NULL ? row[3] : "");
        snprintf(branches[count].complemento,
                 sizeof(branches[count].complemento), "%s",
                 row[4] != NULL ? row[4] : "");
        snprintf(branches[count].bairro, sizeof(branches[count].bairro), "%s",
                 row[5] != NULL ? row[5] : "");
        snprintf(branches[count].cidade, sizeof(branches[count].cidade), "%s",
                 row[6] != NULL ? row[6] : "");
        snprintf(branches[count].estado, sizeof(branches[count].estado), "%s",
                 row[7] != NULL ? row[7] : "");
        count++;
    }

    mysql_free_result(result);
    mysql_close(conn);

    return count;
}
