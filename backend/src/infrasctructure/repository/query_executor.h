#ifndef QUERY_EXECUTOR_H
#define QUERY_EXECUTOR_H

#include <mysql/mysql.h>

void execute_query(MYSQL *conn, const char *query);

#endif
