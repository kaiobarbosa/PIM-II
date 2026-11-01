#ifndef PRODUCT_REPOSITORY_H
#define PRODUCT_REPOSITORY_H

#include <mysql/mysql.h>
#include <stdbool.h>

int create_product(const char *id, const char *nome, const char *marca, const char *descricao, const char *preco, const char *cod_barras, const char *quantidade, const char *filial);

int create_med(char id_med[], char principio_ativo[], char forma_farmaceutica[], char concentracao[], char tarja[], bool receita, char tipagem[]);

int create_prod_med (char id_prod[], char id_med[]);

int create_equip(char id_equipamento[], char modelo[], char fabricante[], char especificacoes[], char garantia[]);

int create_prod_equip(char id_prod[], char id_equip[]);

int create_dermo(char id_dermo[], char tipo_pele[], char volume[], char fator_protecao[], char indicacao[]);

int create_prod_dermo(char id_prod[], char id_dermo[]);

#endif

