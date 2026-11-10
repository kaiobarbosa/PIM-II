#ifndef PRODUCTS_DOMAIN_H
#define PRODUCTS_DOMAIN_H

int verify_null_fields_products(const char *nome, const char *marca,
                                const char *descricao, const char *preco,
                                const char *cod_barras, const char *quantidade,
                                const char *filial);

int verify_null_fields_med(char principio_ativo[], char forma_farmaceutica[], char concentracao[], char tarja[], char receita[], char tipagem[]);

int verify_null_fields_equip(char modelo[], char fabricante[], char especificacoes[], char garantia[]);

int verify_null_fields_dermo(char tipo_pele[], char volume[], char fator_protecao[], char indicacao[]);

int verify_numeric_fields_products(const char *preco, const char *quantidade,
                                   const char *cod_barras);
int verify_null_fields_baixa(char id_produto[], char quantidade[], char filial[]);
int verify_numeric_fields_baixa(char quantidade[]);

int is_only_digits(const char *str);

#endif
