#include "backend/src/infrastructure/repository/login_repository.h"
#include "backend/config/database_connector.h"
#include <stdio.h>
#include <string.h>

int get_user_by_email(const char *email, char *hash_armazenado, char *perfil_usuario) {
    MYSQL *conn = connect_to_database();
    if (conn == NULL) {
        fprintf(stderr, "Erro ao conectar ao banco de dados no login.\n");
        return 0; 
    }

    char query[512];
    snprintf(query, sizeof(query), "SELECT senha, perfil FROM vw_login_usuarios WHERE email = '%s'", email);

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
        // row[0] é a senha, row[1] é o perfil
        if (row[0] != NULL) {
            strcpy(hash_armazenado, row[0]);
        }
        
        if (row[1] != NULL) {
            strcpy(perfil_usuario, row[1]);
        }
        
        encontrou = 1; 
    }

    mysql_free_result(result);
    mysql_close(conn);

    return encontrou;
}
