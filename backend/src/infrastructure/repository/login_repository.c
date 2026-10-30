#include "backend/src/infrastructure/repository/login_repository.h"
#include "backend/config/database_connector.h"
#include <stdio.h>
#include <string.h>

int get_user_by_email(const char *email, char *hash_armazenado,
                      char *perfil_usuario, char id_usuario[7]) {
    MYSQL *conn = connect_to_database();
    if (conn == NULL) {
        fprintf(stderr, "Erro ao conectar ao banco de dados no login.\n");
        return 0; 
    }

    char query[512];
    // Adicionamos o id_usuario no início do SELECT
    snprintf(query, sizeof(query), "SELECT id_usuario, senha, perfil FROM vw_login_usuarios WHERE email = '%s'", email);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao buscar usuário: %s\n", mysql_error(conn));
        mysql_close(conn);
        return 0;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (result == NULL) {
        mysql_close(conn);
        return 0;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    int encontrou = 0;

    if (row) {
        if (row[0] != NULL) {
            snprintf(id_usuario, 7, "%s", row[0]);
        }
        if (row[1] != NULL) {
            strcpy(hash_armazenado, row[1]);
        }
        if (row[2] != NULL) {
            strcpy(perfil_usuario, row[2]);
        }
        
        encontrou = 1; 
    }

    mysql_free_result(result);
    mysql_close(conn);

    return encontrou;
}

int check_user_id_exist(const char *id_user) {
    MYSQL *conn = connect_to_database();
    if (conn == NULL) {
        return -1; 
    }

    char query[256];
    // A mágica acontece aqui: consultamos a View que unifica as 4 tabelas de usuários!
    snprintf(query, sizeof(query), "SELECT COUNT(*) FROM vw_login_usuarios WHERE id_usuario = '%s'", id_user);
    
    int count = 0;
    
    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao buscar ID do usuario: %s\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (result != NULL) {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row && row[0]) {
            count = atoi(row[0]); 
        }
        mysql_free_result(result);
    }

    mysql_close(conn);
    
    // Retorna 1 se o ID já estiver sendo usado por ALGUÉM, ou 0 se estiver livre
    return (count > 0) ? 1 : 0; 
}

int get_all_users_info(UserInfoItem *users_list, int max_users) {
    MYSQL *conn = connect_to_database();
    if (conn == NULL) {
        return -1;
    }

    // Consulta a nova view
    const char *query = "SELECT id_usuario, nome, filial, cargo FROM vw_informacoes_usuarios";
    
    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao buscar a lista de usuarios: %s\n", mysql_error(conn));
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

    // Preenchemos o array recebido com os dados do banco até atingir o limite
    while ((row = mysql_fetch_row(result)) && count < max_users) {
        if (row[0] != NULL) strcpy(users_list[count].id_usuario, row[0]);
        if (row[1] != NULL) strcpy(users_list[count].nome, row[1]);
        if (row[2] != NULL) strcpy(users_list[count].filial, row[2]);
        if (row[3] != NULL) strcpy(users_list[count].cargo, row[3]);
        count++;
    }

    mysql_free_result(result);
    mysql_close(conn);

    return count;
}