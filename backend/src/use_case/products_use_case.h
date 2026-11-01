#ifndef PRODUCTS_USE_CASE_H
#define PRODUCTS_USE_CASE_H

#include "backend/src/infrastructure/repository/product_repository.h"

int baixa_produto_estoque_use_case(char id_produto[], char quantidade[],
                                   char filial[], char id_funcionario[]);
int get_quantidade_produto_by_id_use_case(const char *id_produto, const char *filial);
int select_all_products_use_case(ProdutoItem *produtos, int max_produtos);
int select_products_by_name_use_case(ProdutoItem *produtos, int max_produtos, const char *nome);
int select_products_by_filial_use_case(ProdutoItem *produtos, int max_produtos, const char *filial);
int select_products_by_filial_and_name_use_case(ProdutoItem *produtos, int max_produtos, const char *filial, const char *nome);

int products_use_case(char nome[], char marca[], char descricao[],
                      char preco[], char cod_barras[], char quantidade[],
                      char filial[], char id_product[7], char id_funcionario[]);

int medicamento_use_case(char id_produto[], char principio_ativo[], char forma_farmaceutica[], char concentracao[], char tarja[], char receita[], char tipagem[]);

int equipamento_use_case (char id_produto[], char modelo[], char fabricante[], char especificacoes[], char garantia[]);

int dermocosmetico_use_case(char id_produto[], char tipo_pele[], char volume[], char fator_protecao[], char indicacao[]);

int select_produtos_vendidos_by_user_id_use_case(const char *id_user, ProdutoItem *produtos, int max_produtos);

#endif
