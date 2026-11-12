#include "backend/config/database_connector.h"
#include "backend/config/env_parser.h" 
#include <stdio.h>
#include <stdlib.h> 

MYSQL* connect_to_database() {
    load_env_file(".env");

    MYSQL *conn = mysql_init(NULL);

    if (conn == NULL) {
        fprintf(stderr, "Erro ao inicializar o MySQL\\n");
        return NULL;
    }

    const char *db_host = getenv("DB_HOST") ? getenv("DB_HOST") : "localhost";
    const char *db_user = getenv("DB_USER") ? getenv("DB_USER") : "root";
    const char *db_pass = getenv("DB_PASS") ? getenv("DB_PASS") : "";
    const char *db_name = getenv("DB_NAME") ? getenv("DB_NAME") : "pharma_database";
    
    const char *db_port_str = getenv("DB_PORT");
    int db_port = db_port_str ? atoi(db_port_str) : 3306;

 
    if (mysql_real_connect(conn, db_host, db_user, db_pass, db_name, db_port, NULL, 0) == NULL) {
        fprintf(stderr, "Erro na conexão: %s\\n", mysql_error(conn));
        mysql_close(conn);
        return NULL;
    }

    if (mysql_set_character_set(conn, "utf8mb4") != 0) {
        fprintf(stderr, "Erro ao configurar conexão UTF-8: %s\n",
                mysql_error(conn));
        mysql_close(conn);
        return NULL;
    }
    return conn;
}
