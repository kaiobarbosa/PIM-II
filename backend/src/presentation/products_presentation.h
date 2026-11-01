#ifndef PRODUCTS_PRESENTATION_H
#define PRODUCTS_PRESENTATION_H

#include "backend/src/infrastructure/repository/product_repository.h"

int select_all_products_presentation(ProdutoItem *produtos, int max_produtos);
int select_products_by_name_presentation(ProdutoItem *produtos, int max_produtos, const char *nome);
int select_products_by_filial_presentation(ProdutoItem *produtos, int max_produtos, const char *filial);
int select_products_by_filial_and_name_presentation(ProdutoItem *produtos, int max_produtos, const char *filial, const char *nome);

int products_presentation(char nome[], char marca[], char descricao[],
                          char preco[], char cod_barras[], char quantidade[],
                          char filial[], char id_product[7], char id_funcionario[]);

int product_med(char nome[], char marca[], char descricao[], char preco[],
                char cod_barras[], char quantidade[], char filial[],
                char id_product[7], char principio_ativo[],
                char forma_farmaceutica[], char concentracao[], char tarja[],
                char receita[], char tipagem[], char id_funcionario[]);

int product_equip(char nome[], char marca[], char descricao[], char preco[],
                  char cod_barras[], char quantidade[], char filial[],
                  char id_product[7], char modelo[], char fabricante[],
                  char especificacoes[], char garantia[], char id_funcionario[]);

int product_dermo(char nome[], char marca[], char descricao[], char preco[],
                  char cod_barras[], char quantidade[], char filial[],
                  char id_product[7], char tipo_pele[], char volume[],
                  char fator_protecao[], char indicacao[], char id_funcionario[]);

int baixa_produto_estoque_presentation(char id_produto[], char quantidade[], char filial[]);

#endif