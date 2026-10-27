#include "backend/src/infrastructure/repository/login_repository.h"
#include "backend/config/database_connector.h"
#include <stdio.h>
#include <stdlib.h> // Necessário para a função atoi()
#include <string.h>

int get_user_by_email(const char *email, char *hash_armazenado, char *perfil_usuario, int *id_usuario) {
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
        // A ordem agora é: row[0] = id_usuario, row[1] = senha, row[2] = perfil
        if (row[0] != NULL) {
            *id_usuario = atoi(row[0]); // Converte o ID que vem como texto para inteiro
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