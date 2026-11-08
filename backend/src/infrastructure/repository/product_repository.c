#include "backend/config/database_connector.h"
#include "backend/src/infrastructure/repository/product_repository.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

int select_all_products(ProdutoItem *produtos, int max_produtos) {
    MYSQL *conn = connect_to_database();
    if (conn == NULL) return -1;

   const char *query = "SELECT id_produto, nome, preco, quantidade, filial FROM vw_busca_produtos order by nome;";

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao buscar produtos: %s\\n", mysql_error(conn));
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

    while ((row = mysql_fetch_row(result)) && count < max_produtos) {
        snprintf(produtos[count].id_produto,
                 sizeof(produtos[count].id_produto), "%s",
                 row[0] != NULL ? row[0] : "");
        snprintf(produtos[count].nome, sizeof(produtos[count].nome), "%s",
                 row[1] != NULL ? row[1] : "");
        snprintf(produtos[count].preco, sizeof(produtos[count].preco), "%s",
                 row[2] != NULL ? row[2] : "");
        snprintf(produtos[count].quantidade,
                 sizeof(produtos[count].quantidade), "%s",
                 row[3] != NULL ? row[3] : "");
        snprintf(produtos[count].filial, sizeof(produtos[count].filial), "%s",
                 row[4] != NULL ? row[4] : "");
        count++;
    }

    mysql_free_result(result);
    mysql_close(conn);

    return count;
}

int get_quantidade_produto_by_id(const char *id_produto, const char *filial) {
    MYSQL *conn = connect_to_database();
    if (conn == NULL) return -1;

    char query[256];
    snprintf(query, sizeof(query), "SELECT quantidade FROM vw_busca_produtos WHERE id_produto = '%s' AND filial = '%s';", id_produto, filial);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao buscar quantidade do produto: %s\\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (result == NULL) {
        mysql_close(conn);
        return -1;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    int quantidade = -1;

    if (row != NULL && row[0] != NULL) {
        quantidade = atoi(row[0]);
    }

    mysql_free_result(result);
    mysql_close(conn);

    return quantidade;
}

int baixa_produto_estoque(const char *id_produto, const char *quantidade,
                          const char *filial) {
    MYSQL *conn = connect_to_database();
    if (conn == NULL) return -1;

    char query[256];
    snprintf(query, sizeof(query),
             "UPDATE produto SET quantidade_produto = quantidade_produto - %s "
             "WHERE id_produto = '%s' AND id_filial = '%s' "
             "AND CAST(quantidade_produto AS UNSIGNED) >= %s",
             quantidade, id_produto, filial, quantidade);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao atualizar quantidade do produto: %s\\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    if (mysql_affected_rows(conn) != 1) {
        mysql_close(conn);
        return -1;
    }

    mysql_close(conn);
    return 0;
}

int select_products_by_name(ProdutoItem *produtos, int max_produtos, const char *nome) {
    MYSQL *conn = connect_to_database();
    if (conn == NULL) return -1;

    char query[512];
    snprintf(query, sizeof(query), "SELECT id_produto, nome, preco, quantidade, filial FROM vw_busca_produtos WHERE nome LIKE '%%%s%%' ORDER BY nome;", nome);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao buscar produtos por nome: %s\\n", mysql_error(conn));
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

    while ((row = mysql_fetch_row(result)) && count < max_produtos) {
        snprintf(produtos[count].id_produto,
                 sizeof(produtos[count].id_produto), "%s",
                 row[0] != NULL ? row[0] : "");
        snprintf(produtos[count].nome, sizeof(produtos[count].nome), "%s",
                 row[1] != NULL ? row[1] : "");
        snprintf(produtos[count].preco, sizeof(produtos[count].preco), "%s",
                 row[2] != NULL ? row[2] : "");
        snprintf(produtos[count].quantidade,
                 sizeof(produtos[count].quantidade), "%s",
                 row[3] != NULL ? row[3] : "");
        snprintf(produtos[count].filial, sizeof(produtos[count].filial), "%s",
                 row[4] != NULL ? row[4] : "");
        count++;
    }

    mysql_free_result(result);
    mysql_close(conn);

    return count;
}

int select_products_by_filial(ProdutoItem *produtos, int max_produtos, const char *filial) {
    MYSQL *conn = connect_to_database();
    if (conn == NULL) return -1;

    char query[512];
    snprintf(query, sizeof(query), "SELECT id_produto, nome, preco, quantidade, filial FROM vw_busca_produtos WHERE filial = '%s' ORDER BY nome;", filial);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao buscar produtos por filial: %s\\n", mysql_error(conn));
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

    while ((row = mysql_fetch_row(result)) && count < max_produtos) {
        snprintf(produtos[count].id_produto,
                 sizeof(produtos[count].id_produto), "%s",
                 row[0] != NULL ? row[0] : "");
        snprintf(produtos[count].nome, sizeof(produtos[count].nome), "%s",
                 row[1] != NULL ? row[1] : "");
        snprintf(produtos[count].preco, sizeof(produtos[count].preco), "%s",
                 row[2] != NULL ? row[2] : "");
        snprintf(produtos[count].quantidade,
                 sizeof(produtos[count].quantidade), "%s",
                 row[3] != NULL ? row[3] : "");
        snprintf(produtos[count].filial, sizeof(produtos[count].filial), "%s",
                 row[4] != NULL ? row[4] : "");
        count++;
    }

    mysql_free_result(result);
    mysql_close(conn);

    return count;
}

int select_products_by_filial_and_name(ProdutoItem *produtos, int max_produtos, const char *filial, const char *nome) {
    MYSQL *conn = connect_to_database();
    if (conn == NULL) return -1;

    char query[512];
    snprintf(query, sizeof(query), "SELECT id_produto, nome, preco, quantidade, filial FROM vw_busca_produtos WHERE filial = '%s' AND nome LIKE '%%%s%%' ORDER BY nome;", filial, nome);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao buscar produtos por filial e nome: %s\\n", mysql_error(conn));
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

    while ((row = mysql_fetch_row(result)) && count < max_produtos) {
        snprintf(produtos[count].id_produto,
                 sizeof(produtos[count].id_produto), "%s",
                 row[0] != NULL ? row[0] : "");
        snprintf(produtos[count].nome, sizeof(produtos[count].nome), "%s",
                 row[1] != NULL ? row[1] : "");
        snprintf(produtos[count].preco, sizeof(produtos[count].preco), "%s",
                 row[2] != NULL ? row[2] : "");
        snprintf(produtos[count].quantidade,
                 sizeof(produtos[count].quantidade), "%s",
                 row[3] != NULL ? row[3] : "");
        snprintf(produtos[count].filial, sizeof(produtos[count].filial), "%s",
                 row[4] != NULL ? row[4] : "");
        count++;
    }

    mysql_free_result(result);
    mysql_close(conn);

    return count;
}

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

int create_med(char id_med[], char principio_ativo[], char forma_farmaceutica[], char concentracao[], char tarja[], bool receita, char tipagem[]){

    MYSQL *conn = connect_to_database();
    if (conn == NULL) {
        fprintf(stderr, "Erro ao conectar ao banco de dados.\n");
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query), "INSERT INTO medicamento (id_medicamento, principio_ativo, forma_farmaceutica, concentracao, tarja, receita, tipagem) values ('%s', '%s', '%s', '%s', '%s', %d, '%s')", id_med, principio_ativo, forma_farmaceutica, concentracao, tarja, receita ? 1 : 0, tipagem);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao criar medicamento: %s\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    mysql_close(conn);
    return 0;
}

int create_prod_med (char id_prod[], char id_med[]){
    MYSQL *conn = connect_to_database();
    if (conn == NULL) {
        fprintf(stderr, "Erro ao conectar ao banco de dados.\n");
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query),  "INSERT INTO produto_medicamento (id_produto, id_medicamento) values ('%s', '%s')", id_prod, id_med);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao criar produto_medicamento: %s\n",
                mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    mysql_close(conn);
    return 0;
}

int create_equip(char id_equipamento[], char modelo[], char fabricante[], char especificacoes[], char garantia[]){

     MYSQL *conn = connect_to_database();
    if (conn == NULL) {
        fprintf(stderr, "Erro ao conectar ao banco de dados.\n");
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query),  "INSERT INTO equipamentos (id_equipamentos, modelo, fabricante, especificacoes, garantia) values('%s', '%s', '%s', '%s', '%s')", id_equipamento, modelo, fabricante, especificacoes, garantia);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao criar equipamento: %s\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    mysql_close(conn);
    return 0;
}

int create_dermo(char id_dermo[], char tipo_pele[], char volume[], char fator_protecao[], char indicacao[]){

     MYSQL *conn = connect_to_database();
    if (conn == NULL) {
        fprintf(stderr, "Erro ao conectar ao banco de dados.\n");
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query),  "INSERT INTO dermocosmetico (id_dermo, tipo_pele, volume, fator_protecao, indicacao) values('%s', '%s', '%s', '%s', '%s')", id_dermo, tipo_pele, volume, fator_protecao, indicacao);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao criar dermocosmetico: %s\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    mysql_close(conn);
    return 0;
}

int create_prod_equip(char id_prod[], char id_equip[]){

    MYSQL *conn = connect_to_database();
    if (conn == NULL) {
        fprintf(stderr, "Erro ao conectar ao banco de dados.\n");
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query), "INSERT INTO produto_equipamento (id_produto, id_equipamentos) values ('%s', '%s')", id_prod, id_equip);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao relacionar produto e equipamento: %s\n",
                mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    mysql_close(conn);
    return 0;

}

int create_prod_dermo(char id_prod[], char id_dermo[]){

    MYSQL *conn = connect_to_database();
    if (conn == NULL) {
        fprintf(stderr, "Erro ao conectar ao banco de dados.\n");
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query), "INSERT INTO produto_dermo (id_produto, id_dermo) values ('%s', '%s')", id_prod, id_dermo);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao relacionar produto e dermocosmetico: %s\n",
                mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    mysql_close(conn);
    return 0;

}

int create_log_product(char id_log[], char acao[], char id_produto[], char id_funcionario[], char id_filial[]){

    MYSQL *conn = connect_to_database();
    if (conn == NULL) {
        fprintf(stderr, "Erro ao conectar ao banco de dados.\n");
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query),
             "INSERT INTO log_produto (id_log, acao_log, id_produto, funcionario_id, id_filial) "
             "VALUES ('%s', '%s', '%s', '%s', '%s')",
             id_log, acao, id_produto, id_funcionario, id_filial);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao criar log de produto: %s\n",
                mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    mysql_close(conn);
    return 0;

}

int get_produtos_vendidos_by_user_id(const char *id_user,
                                     ProdutoItem *produtos,
                                     int max_produtos) {
    MYSQL *conn = connect_to_database();
    if (conn == NULL) return -1;

    char query[512];
    snprintf(query, sizeof(query),
             "SELECT p.id_produto, p.nome_produto, p.preco, "
             "p.quantidade_produto, l.id_filial, l.data_log "
             "FROM log_produto l "
             "JOIN produto p ON p.id_produto = l.id_produto "
             "WHERE l.funcionario_id = '%s' AND l.acao_log = 'Venda' "
             "ORDER BY l.data_log DESC, l.id_log DESC LIMIT %d",
             id_user, max_produtos);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao buscar produtos vendidos pelo usuário: %s\n", mysql_error(conn));
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

    while ((row = mysql_fetch_row(result)) && count < max_produtos) {
        snprintf(produtos[count].id_produto,
                 sizeof(produtos[count].id_produto), "%s",
                 row[0] != NULL ? row[0] : "");
        snprintf(produtos[count].nome, sizeof(produtos[count].nome), "%s",
                 row[1] != NULL ? row[1] : "");
        snprintf(produtos[count].preco, sizeof(produtos[count].preco), "%s",
                 row[2] != NULL ? row[2] : "");
        snprintf(produtos[count].quantidade,
                 sizeof(produtos[count].quantidade), "%s",
                 row[3] != NULL ? row[3] : "");
        snprintf(produtos[count].filial, sizeof(produtos[count].filial), "%s",
                 row[4] != NULL ? row[4] : "");
        snprintf(produtos[count].data_venda,
                 sizeof(produtos[count].data_venda), "%s",
                 row[5] != NULL ? row[5] : "");
        count++;
    }

    mysql_free_result(result);
    mysql_close(conn);

    return count;
}
