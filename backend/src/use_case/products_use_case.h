#ifndef PRODUCTS_USE_CASE_H
#define PRODUCTS_USE_CASE_H

int products_use_case(char nome[], char marca[], char descricao[],
                      char preco[], char cod_barras[], char quantidade[],
                      char filial[], char id_product[7]);

int medicamento_use_case(char id_produto[], char principio_ativo[], char forma_farmaceutica[], char concentracao[], char tarja[], char receita[], char tipagem[]);

#endif
