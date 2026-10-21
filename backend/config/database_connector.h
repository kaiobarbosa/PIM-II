#ifndef DATABASE_CONNECTOR_H
#define DATABASE_CONNECTOR_H

#include <mysql/mysql.h>

// Apenas a assinatura da função (o contrato)
MYSQL* connect_to_database();

#endif
