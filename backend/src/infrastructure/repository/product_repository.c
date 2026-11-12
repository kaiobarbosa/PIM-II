#include "backend/config/database_connector.h"
#include "backend/src/infrastructure/repository/product_repository.h"
#include <stdio.h>
#include <stdlib.h>
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

int product_requires_prescription(const char *id_produto) {
    MYSQL *conn;
    char escaped_product_id[13];
    char query[256];
    MYSQL_RES *result;
    MYSQL_ROW row;
    int requires_prescription;

    if (id_produto == NULL || strlen(id_produto) != 6) return -1;
    conn = connect_to_database();
    if (conn == NULL) return -1;

    mysql_real_escape_string(conn, escaped_product_id, id_produto,
                             (unsigned long)strlen(id_produto));
    snprintf(query, sizeof(query),
             "SELECT EXISTS (SELECT 1 FROM produto_medicamento pm "
             "JOIN medicamento m ON m.id_medicamento=pm.id_medicamento "
             "WHERE pm.id_produto='%s' AND m.receita=0)",
             escaped_product_id);
    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao consultar exigencia de receita: %s\n",
                mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    result = mysql_store_result(conn);
    row = result != NULL ? mysql_fetch_row(result) : NULL;
    if (row == NULL || row[0] == NULL) {
        if (result != NULL) mysql_free_result(result);
        mysql_close(conn);
        return -1;
    }
    requires_prescription = strcmp(row[0], "1") == 0;
    mysql_free_result(result);
    mysql_close(conn);
    return requires_prescription;
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

    if (id == NULL || nome == NULL || marca == NULL || descricao == NULL ||
        preco == NULL || cod_barras == NULL || quantidade == NULL ||
        filial == NULL || strlen(id) != 6 || strlen(filial) != 6 ||
        strlen(nome) > 255 || strlen(marca) > 255 ||
        strlen(descricao) > 255 || strlen(preco) > 255 ||
        strlen(cod_barras) > 255 || strlen(quantidade) > 255) {
        mysql_close(conn);
        return -1;
    }

    char escaped_id[13];
    char escaped_name[511];
    char escaped_brand[511];
    char escaped_description[511];
    char escaped_price[511];
    char escaped_barcode[511];
    char escaped_quantity[511];
    char escaped_branch[13];
    char query[4096];

#define ESCAPE_PRODUCT_FIELD(output, input) \
    mysql_real_escape_string(conn, output, input, (unsigned long)strlen(input))
    ESCAPE_PRODUCT_FIELD(escaped_id, id);
    ESCAPE_PRODUCT_FIELD(escaped_name, nome);
    ESCAPE_PRODUCT_FIELD(escaped_brand, marca);
    ESCAPE_PRODUCT_FIELD(escaped_description, descricao);
    ESCAPE_PRODUCT_FIELD(escaped_price, preco);
    ESCAPE_PRODUCT_FIELD(escaped_barcode, cod_barras);
    ESCAPE_PRODUCT_FIELD(escaped_quantity, quantidade);
    ESCAPE_PRODUCT_FIELD(escaped_branch, filial);
#undef ESCAPE_PRODUCT_FIELD

    snprintf(query, sizeof(query),
             "INSERT INTO produto (id_produto, nome_produto, marca_produto, "
             "descricao_produto, preco, codigo_de_barras, "
             "quantidade_produto, id_filial) "
             "VALUES ('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s')",
             escaped_id, escaped_name, escaped_brand, escaped_description,
             escaped_price, escaped_barcode, escaped_quantity, escaped_branch);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao criar produto: %s\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    mysql_close(conn);
    return 0;
}

int create_med(char id_med[], char principio_ativo[], char forma_farmaceutica[], char concentracao[], char tarja[], bool requires_prescription, char tipagem[]){

    MYSQL *conn = connect_to_database();
    if (conn == NULL) {
        fprintf(stderr, "Erro ao conectar ao banco de dados.\n");
        return -1;
    }

    char query[512];
    snprintf(query, sizeof(query), "INSERT INTO medicamento (id_medicamento, principio_ativo, forma_farmaceutica, concentracao, tarja, receita, tipagem) values ('%s', '%s', '%s', '%s', '%s', %d, '%s')", id_med, principio_ativo, forma_farmaceutica, concentracao, tarja, requires_prescription ? 0 : 1, tipagem);

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

static int insert_log_product(MYSQL *conn, const char *id_log,
                              const char *acao, const char *id_produto,
                              const char *id_funcionario,
                              const char *id_filial,
                              const char *quantidade) {
    char escaped_id_log[13];
    char escaped_action[41];
    char escaped_product_id[13];
    char escaped_employee_id[13];
    char escaped_branch_id[13];
    char escaped_quantity[41];
    char quantity_value[48];
    char query[768];

    if (id_log == NULL || acao == NULL || id_produto == NULL ||
        id_funcionario == NULL || id_filial == NULL) {
        return -1;
    }
    if (strlen(id_log) != 6 || strlen(acao) > 20 ||
        strlen(id_produto) != 6 || strlen(id_funcionario) != 6 ||
        strlen(id_filial) != 6 ||
        (quantidade != NULL && strlen(quantidade) > 20)) {
        return -1;
    }

#define ESCAPE_LOG_FIELD(output, input) \
    mysql_real_escape_string(conn, output, input, (unsigned long)strlen(input))
    ESCAPE_LOG_FIELD(escaped_id_log, id_log);
    ESCAPE_LOG_FIELD(escaped_action, acao);
    ESCAPE_LOG_FIELD(escaped_product_id, id_produto);
    ESCAPE_LOG_FIELD(escaped_employee_id, id_funcionario);
    ESCAPE_LOG_FIELD(escaped_branch_id, id_filial);
#undef ESCAPE_LOG_FIELD

    if (quantidade == NULL) {
        snprintf(quantity_value, sizeof(quantity_value), "NULL");
    } else {
        mysql_real_escape_string(conn, escaped_quantity, quantidade,
                                 (unsigned long)strlen(quantidade));
        snprintf(quantity_value, sizeof(quantity_value), "'%s'",
                 escaped_quantity);
    }

    snprintf(query, sizeof(query),
             "INSERT INTO log_produto "
             "(id_log, data_log, quantidade_produto, acao_log, id_produto, "
             "funcionario_id, id_filial) "
             "VALUES ('%s', CURRENT_DATE(), %s, '%s', '%s', '%s', '%s')",
             escaped_id_log, quantity_value, escaped_action,
             escaped_product_id, escaped_employee_id, escaped_branch_id);
    return mysql_query(conn, query) == 0 ? 0 : -1;
}

int create_log_product(char id_log[], char acao[], char id_produto[],
                       char id_funcionario[], char id_filial[],
                       const char *quantidade) {
    MYSQL *conn = connect_to_database();
    if (conn == NULL) {
        fprintf(stderr, "Erro ao conectar ao banco de dados.\n");
        return -1;
    }

    if (insert_log_product(conn, id_log, acao, id_produto, id_funcionario,
                           id_filial, quantidade) != 0) {
        fprintf(stderr, "Erro ao criar log de produto: %s\n",
                mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    mysql_close(conn);
    return 0;
}

int register_product_sale(const char *id_log, const char *id_produto,
                          const char *id_funcionario, const char *id_filial,
                          const char *quantidade) {
    MYSQL *conn = connect_to_database();
    char escaped_product_id[13];
    char escaped_branch_id[13];
    char query[512];
    int result = -1;

    if (conn == NULL) return -1;
    if (id_log == NULL || id_produto == NULL || id_funcionario == NULL ||
        id_filial == NULL || quantidade == NULL || quantidade[0] == '\0' ||
        strlen(id_log) != 6 || strlen(id_produto) != 6 ||
        strlen(id_funcionario) != 6 || strlen(id_filial) != 6 ||
        strlen(quantidade) > 10 ||
        strspn(quantidade, "0123456789") != strlen(quantidade) ||
        atoi(quantidade) <= 0) {
        mysql_close(conn);
        return -1;
    }

    if (mysql_autocommit(conn, 0) != 0) {
        fprintf(stderr, "Erro ao iniciar transação da venda: %s\n",
                mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    mysql_real_escape_string(conn, escaped_product_id, id_produto,
                             (unsigned long)strlen(id_produto));
    mysql_real_escape_string(conn, escaped_branch_id, id_filial,
                             (unsigned long)strlen(id_filial));
    snprintf(query, sizeof(query),
             "UPDATE produto SET quantidade_produto = "
             "CAST(quantidade_produto AS UNSIGNED) - %s "
             "WHERE id_produto = '%s' AND id_filial = '%s' "
             "AND CAST(quantidade_produto AS UNSIGNED) >= %s",
             quantidade, escaped_product_id, escaped_branch_id, quantidade);

    if (mysql_query(conn, query) != 0) {
        fprintf(stderr, "Erro ao atualizar estoque da venda: %s\n",
                mysql_error(conn));
        mysql_rollback(conn);
        goto cleanup;
    }
    if (mysql_affected_rows(conn) != 1) {
        fprintf(stderr,
                "Venda não realizada: estoque insuficiente ou produto inexistente.\n");
        mysql_rollback(conn);
        goto cleanup;
    }

    if (insert_log_product(conn, id_log, "Venda", id_produto,
                           id_funcionario, id_filial, quantidade) != 0) {
        fprintf(stderr, "Erro ao gravar log da venda: %s\n",
                mysql_error(conn));
        mysql_rollback(conn);
        goto cleanup;
    }

    if (mysql_commit(conn) != 0) {
        fprintf(stderr, "Erro ao confirmar venda: %s\n", mysql_error(conn));
        mysql_rollback(conn);
        goto cleanup;
    }
    result = 0;

cleanup:
    mysql_autocommit(conn, 1);
    mysql_close(conn);
    return result;
}

int get_produtos_vendidos_by_user_id(const char *id_user,
                                     ProdutoItem *produtos,
                                     int max_produtos) {
    MYSQL *conn = connect_to_database();
    if (conn == NULL) return -1;

    char query[512];
    snprintf(query, sizeof(query),
             "SELECT p.id_produto, p.nome_produto, p.preco, "
             "l.quantidade_produto, l.id_filial, l.data_log "
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

static void copy_product_value(char *destination, size_t capacity,
                               const char *value) {
    snprintf(destination, capacity, "%s", value != NULL ? value : "");
}

int get_product_details(const char *id_produto, ProductDetails *details) {
    MYSQL *conn;
    MYSQL_RES *result;
    MYSQL_ROW row;
    char escaped_id[13];
    char query[2400];

    if (id_produto == NULL || details == NULL || strlen(id_produto) != 6)
        return -1;
    conn = connect_to_database();
    if (conn == NULL) return -1;
    mysql_real_escape_string(conn, escaped_id, id_produto,
                             (unsigned long)strlen(id_produto));
    snprintf(query, sizeof(query),
        "SELECT p.id_produto,p.nome_produto,p.marca_produto,"
        "p.descricao_produto,p.preco,p.codigo_de_barras,"
        "p.quantidade_produto,p.id_filial,"
        "CASE WHEN pm.id_medicamento IS NOT NULL THEN 2 "
        "WHEN pd.id_dermo IS NOT NULL THEN 1 "
        "WHEN pe.id_equipamentos IS NOT NULL THEN 3 ELSE 0 END,"
        "COALESCE(m.principio_ativo,''),COALESCE(m.forma_farmaceutica,''),"
        "COALESCE(m.concentracao,''),COALESCE(m.tarja,''),"
        "CASE WHEN m.receita=0 THEN 'sim' WHEN m.receita=1 THEN 'nao' ELSE '' END,"
        "COALESCE(m.tipagem,''),COALESCE(d.tipo_pele,''),COALESCE(d.volume,''),"
        "COALESCE(d.fator_protecao,''),COALESCE(d.indicacao,''),"
        "COALESCE(e.modelo,''),COALESCE(e.fabricante,''),"
        "COALESCE(e.especificacoes,''),COALESCE(e.garantia,'') "
        "FROM produto p "
        "LEFT JOIN produto_medicamento pm ON pm.id_produto=p.id_produto "
        "LEFT JOIN medicamento m ON m.id_medicamento=pm.id_medicamento "
        "LEFT JOIN produto_dermo pd ON pd.id_produto=p.id_produto "
        "LEFT JOIN dermocosmetico d ON d.id_dermo=pd.id_dermo "
        "LEFT JOIN produto_equipamento pe ON pe.id_produto=p.id_produto "
        "LEFT JOIN equipamentos e ON e.id_equipamentos=pe.id_equipamentos "
        "WHERE p.id_produto='%s' LIMIT 1", escaped_id);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao consultar detalhes do produto: %s\n",
                mysql_error(conn));
        mysql_close(conn);
        return -1;
    }
    result = mysql_store_result(conn);
    row = result != NULL ? mysql_fetch_row(result) : NULL;
    if (row == NULL) {
        if (result != NULL) mysql_free_result(result);
        mysql_close(conn);
        return 0;
    }

    memset(details, 0, sizeof(*details));
    copy_product_value(details->id_produto, sizeof(details->id_produto), row[0]);
    copy_product_value(details->nome, sizeof(details->nome), row[1]);
    copy_product_value(details->marca, sizeof(details->marca), row[2]);
    copy_product_value(details->descricao, sizeof(details->descricao), row[3]);
    copy_product_value(details->preco, sizeof(details->preco), row[4]);
    copy_product_value(details->codigo_de_barras,
                       sizeof(details->codigo_de_barras), row[5]);
    copy_product_value(details->quantidade, sizeof(details->quantidade), row[6]);
    copy_product_value(details->id_filial, sizeof(details->id_filial), row[7]);
    details->kind = row[8] != NULL ? (ProductKind)atoi(row[8]) :
                                     PRODUCT_KIND_GENERAL;
    copy_product_value(details->principio_ativo,
                       sizeof(details->principio_ativo), row[9]);
    copy_product_value(details->forma_farmaceutica,
                       sizeof(details->forma_farmaceutica), row[10]);
    copy_product_value(details->concentracao,
                       sizeof(details->concentracao), row[11]);
    copy_product_value(details->tarja, sizeof(details->tarja), row[12]);
    copy_product_value(details->receita, sizeof(details->receita), row[13]);
    copy_product_value(details->tipagem, sizeof(details->tipagem), row[14]);
    copy_product_value(details->tipo_pele, sizeof(details->tipo_pele), row[15]);
    copy_product_value(details->volume, sizeof(details->volume), row[16]);
    copy_product_value(details->fator_protecao,
                       sizeof(details->fator_protecao), row[17]);
    copy_product_value(details->indicacao, sizeof(details->indicacao), row[18]);
    copy_product_value(details->modelo, sizeof(details->modelo), row[19]);
    copy_product_value(details->fabricante, sizeof(details->fabricante), row[20]);
    copy_product_value(details->especificacoes,
                       sizeof(details->especificacoes), row[21]);
    copy_product_value(details->garantia, sizeof(details->garantia), row[22]);
    mysql_free_result(result);
    mysql_close(conn);
    return 1;
}

int update_product_details(const ProductDetails *details) {
    MYSQL *conn;
    char escaped[22][513];
    const char *values[22];
    char query[4096];
    int index;

    if (details == NULL || strlen(details->id_produto) != 6 ||
        strlen(details->id_filial) != 6) return -1;
    values[0] = details->id_produto;
    values[1] = details->nome;
    values[2] = details->marca;
    values[3] = details->descricao;
    values[4] = details->preco;
    values[5] = details->codigo_de_barras;
    values[6] = details->quantidade;
    values[7] = details->id_filial;
    values[8] = details->principio_ativo;
    values[9] = details->forma_farmaceutica;
    values[10] = details->concentracao;
    values[11] = details->tarja;
    values[12] = details->receita;
    values[13] = details->tipagem;
    values[14] = details->tipo_pele;
    values[15] = details->volume;
    values[16] = details->fator_protecao;
    values[17] = details->indicacao;
    values[18] = details->modelo;
    values[19] = details->fabricante;
    values[20] = details->especificacoes;
    values[21] = details->garantia;

    conn = connect_to_database();
    if (conn == NULL) return -1;
    for (index = 0; index < 22; index++) {
        mysql_real_escape_string(conn, escaped[index], values[index],
                                 (unsigned long)strlen(values[index]));
    }
    if (mysql_query(conn, "START TRANSACTION")) {
        mysql_close(conn);
        return -1;
    }
    snprintf(query, sizeof(query),
        "UPDATE produto SET nome_produto='%s',marca_produto='%s',"
        "descricao_produto='%s',preco='%s',codigo_de_barras='%s',"
        "quantidade_produto='%s',id_filial='%s' WHERE id_produto='%s'",
        escaped[1], escaped[2], escaped[3], escaped[4], escaped[5],
        escaped[6], escaped[7], escaped[0]);
    if (mysql_query(conn, query)) goto update_failed;

    if (details->kind == PRODUCT_KIND_MEDICINE) {
        snprintf(query, sizeof(query),
            "UPDATE medicamento m JOIN produto_medicamento pm "
            "ON pm.id_medicamento=m.id_medicamento "
            "SET m.principio_ativo='%s',m.forma_farmaceutica='%s',"
            "m.concentracao='%s',m.tarja='%s',m.receita=%d,m.tipagem='%s' "
            "WHERE pm.id_produto='%s'",
            escaped[8], escaped[9], escaped[10], escaped[11],
            (strcmp(details->receita, "sim") == 0 ||
             strcmp(details->receita, "0") == 0) ? 0 : 1,
            escaped[13], escaped[0]);
    } else if (details->kind == PRODUCT_KIND_DERMOCOSMETIC) {
        snprintf(query, sizeof(query),
            "UPDATE dermocosmetico d JOIN produto_dermo pd "
            "ON pd.id_dermo=d.id_dermo SET d.tipo_pele='%s',d.volume='%s',"
            "d.fator_protecao='%s',d.indicacao='%s' WHERE pd.id_produto='%s'",
            escaped[14], escaped[15], escaped[16], escaped[17], escaped[0]);
    } else if (details->kind == PRODUCT_KIND_EQUIPMENT) {
        snprintf(query, sizeof(query),
            "UPDATE equipamentos e JOIN produto_equipamento pe "
            "ON pe.id_equipamentos=e.id_equipamentos SET e.modelo='%s',"
            "e.fabricante='%s',e.especificacoes='%s',e.garantia='%s' "
            "WHERE pe.id_produto='%s'",
            escaped[18], escaped[19], escaped[20], escaped[21], escaped[0]);
    }
    if (details->kind != PRODUCT_KIND_GENERAL && mysql_query(conn, query))
        goto update_failed;
    if (mysql_query(conn, "COMMIT")) goto update_failed;
    mysql_close(conn);
    return 0;

update_failed:
    fprintf(stderr, "Erro ao atualizar produto: %s\n", mysql_error(conn));
    mysql_query(conn, "ROLLBACK");
    mysql_close(conn);
    return -1;
}

int remove_product(const char *id_produto) {
    MYSQL *conn;
    MYSQL_RES *result;
    MYSQL_ROW row;
    char escaped_id[13];
    char medicine_id[7] = "";
    char equipment_id[7] = "";
    char dermo_id[7] = "";
    char query[512];

    if (id_produto == NULL || strlen(id_produto) != 6) return -1;
    conn = connect_to_database();
    if (conn == NULL) return -1;
    mysql_real_escape_string(conn, escaped_id, id_produto,
                             (unsigned long)strlen(id_produto));
    if (mysql_query(conn, "START TRANSACTION")) {
        mysql_close(conn);
        return -1;
    }
    snprintf(query, sizeof(query),
        "SELECT pm.id_medicamento,pe.id_equipamentos,pd.id_dermo "
        "FROM produto p LEFT JOIN produto_medicamento pm "
        "ON pm.id_produto=p.id_produto LEFT JOIN produto_equipamento pe "
        "ON pe.id_produto=p.id_produto LEFT JOIN produto_dermo pd "
        "ON pd.id_produto=p.id_produto WHERE p.id_produto='%s' LIMIT 1",
        escaped_id);
    if (mysql_query(conn, query)) goto delete_failed;
    result = mysql_store_result(conn);
    row = result != NULL ? mysql_fetch_row(result) : NULL;
    if (row == NULL) {
        if (result != NULL) mysql_free_result(result);
        mysql_query(conn, "ROLLBACK");
        mysql_close(conn);
        return 0;
    }
    copy_product_value(medicine_id, sizeof(medicine_id), row[0]);
    copy_product_value(equipment_id, sizeof(equipment_id), row[1]);
    copy_product_value(dermo_id, sizeof(dermo_id), row[2]);
    mysql_free_result(result);

    snprintf(query, sizeof(query),
             "DELETE FROM log_produto WHERE id_produto='%s'", escaped_id);
    if (mysql_query(conn, query)) goto delete_failed;
    snprintf(query, sizeof(query),
             "DELETE FROM produto_medicamento WHERE id_produto='%s'",
             escaped_id);
    if (mysql_query(conn, query)) goto delete_failed;
    snprintf(query, sizeof(query),
             "DELETE FROM produto_equipamento WHERE id_produto='%s'",
             escaped_id);
    if (mysql_query(conn, query)) goto delete_failed;
    snprintf(query, sizeof(query),
             "DELETE FROM produto_dermo WHERE id_produto='%s'", escaped_id);
    if (mysql_query(conn, query)) goto delete_failed;
    if (medicine_id[0] != '\0') {
        snprintf(query, sizeof(query),
                 "DELETE FROM medicamento m WHERE m.id_medicamento='%s' "
                 "AND NOT EXISTS (SELECT 1 FROM produto_medicamento pm "
                 "WHERE pm.id_medicamento=m.id_medicamento)",
                 medicine_id);
        if (mysql_query(conn, query)) goto delete_failed;
    }
    if (equipment_id[0] != '\0') {
        snprintf(query, sizeof(query),
                 "DELETE FROM equipamentos e WHERE e.id_equipamentos='%s' "
                 "AND NOT EXISTS (SELECT 1 FROM produto_equipamento pe "
                 "WHERE pe.id_equipamentos=e.id_equipamentos)",
                 equipment_id);
        if (mysql_query(conn, query)) goto delete_failed;
    }
    if (dermo_id[0] != '\0') {
        snprintf(query, sizeof(query),
                 "DELETE FROM dermocosmetico d WHERE d.id_dermo='%s' "
                 "AND NOT EXISTS (SELECT 1 FROM produto_dermo pd "
                 "WHERE pd.id_dermo=d.id_dermo)", dermo_id);
        if (mysql_query(conn, query)) goto delete_failed;
    }
    snprintf(query, sizeof(query), "DELETE FROM produto WHERE id_produto='%s'",
             escaped_id);
    if (mysql_query(conn, query) || mysql_affected_rows(conn) != 1)
        goto delete_failed;
    if (mysql_query(conn, "COMMIT")) goto delete_failed;
    mysql_close(conn);
    return 1;

delete_failed:
    fprintf(stderr, "Erro ao excluir produto: %s\n", mysql_error(conn));
    mysql_query(conn, "ROLLBACK");
    mysql_close(conn);
    return -1;
}
