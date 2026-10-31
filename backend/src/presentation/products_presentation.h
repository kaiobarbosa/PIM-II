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

#endif
