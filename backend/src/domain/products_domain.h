#ifndef PRODUCTS_DOMAIN_H
#define PRODUCTS_DOMAIN_H

int verify_null_fields_products(char nome[], char marca[], char descricao[], char preco[], char cod_barras[], char quantidade[], char filial[]);

int verify_null_fields_med(char principio_ativo[], char forma_farmaceutica[], char concentracao[], char tarja[], char receita[], char tipagem[]);

int verify_numeric_fields_products(char preco[], char quantidade[], char cod_barras[]);

#endif
