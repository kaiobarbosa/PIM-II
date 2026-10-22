#include "backend/config/database_connector.h"
#include "backend/src/infrasctructure/repository/query_executor.h"

int check_first_use() {
    
    MYSQL *db_conn = connect_to_database();
    
    if (db_conn != NULL) {
        execute_query(db_conn, "SELECT * FROM Diretor");
        mysql_close(db_conn);
    }

    return 0;
}