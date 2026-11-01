#ifndef PRODUCTS_PRESENTATION_H
#define PRODUCTS_PRESENTATION_H

int products_presentation(char nome[], char marca[], char descricao[],
                          char preco[], char cod_barras[], char quantidade[],
                          char filial[], char id_product[7]);

int product_med(char nome[], char marca[], char descricao[], char preco[],
                char cod_barras[], char quantidade[], char filial[],
                char id_product[7], char principio_ativo[],
                char forma_farmaceutica[], char concentracao[], char tarja[],
                char receita[], char tipagem[]);

int product_equip(char nome[], char marca[], char descricao[], char preco[],
                  char cod_barras[], char quantidade[], char filial[],
                  char id_product[7], char modelo[], char fabricante[],
                  char especificacoes[], char garantia[]);

int product_dermo(char nome[], char marca[], char descricao[], char preco[],
                  char cod_barras[], char quantidade[], char filial[],
                  char id_product[7], char tipo_pele[], char volume[],
                  char fator_protecao[], char indicacao[]);

#endif
