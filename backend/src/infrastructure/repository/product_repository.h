#ifndef PRODUCT_REPOSITORY_H
#define PRODUCT_REPOSITORY_H

#include <mysql/mysql.h>

int create_product(const char *id, const char *nome, const char *marca, const char *descricao, const char *preco, const char *cod_barras, const char *quantidade, const char *filial);

#endif

