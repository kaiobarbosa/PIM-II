#ifndef PRODUCT_REPOSITORY_H
#define PRODUCT_REPOSITORY_H

#include <mysql/mysql.h>
#include <stdbool.h>

typedef struct {
    char id_produto[7];
    char nome[101];      
    char preco[21];      
    char quantidade[11]; 
    char filial[101];
    char data_venda[11];
} ProdutoItem;

typedef enum {
    PRODUCT_KIND_GENERAL,
    PRODUCT_KIND_DERMOCOSMETIC,
    PRODUCT_KIND_MEDICINE,
    PRODUCT_KIND_EQUIPMENT
} ProductKind;

typedef struct {
    char id_produto[7];
    ProductKind kind;
    char nome[101];
    char marca[51];
    char descricao[256];
    char preco[21];
    char codigo_de_barras[31];
    char quantidade[11];
    char id_filial[7];
    char principio_ativo[51];
    char forma_farmaceutica[51];
    char concentracao[51];
    char tarja[51];
    char receita[8];
    char tipagem[51];
    char tipo_pele[51];
    char volume[51];
    char fator_protecao[51];
    char indicacao[51];
    char modelo[51];
    char fabricante[51];
    char especificacoes[51];
    char garantia[51];
} ProductDetails;

int select_all_products(ProdutoItem *produtos, int max_produtos);
int get_quantidade_produto_by_id(const char *id_produto, const char *filial);
int product_requires_prescription(const char *id_produto);
int baixa_produto_estoque(const char *id_produto, const char *quantidade,
                          const char *filial);
int select_products_by_name(ProdutoItem *produtos, int max_produtos, const char *nome);
int select_products_by_filial(ProdutoItem *produtos, int max_produtos, const char *filial);
int select_products_by_filial_and_name(ProdutoItem *produtos, int max_produtos, const char *filial, const char *nome);

int create_product(const char *id, const char *nome, const char *marca, const char *descricao, const char *preco, const char *cod_barras, const char *quantidade, const char *filial);

int create_med(char id_med[], char principio_ativo[], char forma_farmaceutica[], char concentracao[], char tarja[], bool requires_prescription, char tipagem[]);

int create_prod_med (char id_prod[], char id_med[]);

int create_equip(char id_equipamento[], char modelo[], char fabricante[], char especificacoes[], char garantia[]);

int create_prod_equip(char id_prod[], char id_equip[]);

int create_dermo(char id_dermo[], char tipo_pele[], char volume[], char fator_protecao[], char indicacao[]);

int create_prod_dermo(char id_prod[], char id_dermo[]);

int create_log_product(char id_log[], char acao[], char id_produto[],
                       char id_funcionario[], char id_filial[]);

int get_produtos_vendidos_by_user_id(const char *id_user, ProdutoItem *produtos, int max_produtos);
int get_product_details(const char *id_produto, ProductDetails *details);
int update_product_details(const ProductDetails *details);
int remove_product(const char *id_produto);

#endif
