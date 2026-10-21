#ifndef QUERY_EXECUTOR_H
#define QUERY_EXECUTOR_H

#include <mysql/mysql.h>

// Apenas a assinatura da função
void execute_query(MYSQL *conn, const char *query);

#endif
