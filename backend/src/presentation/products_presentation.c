#include <stdio.h>
#include <stdlib.h>

#include "backend/src/use_case/products_use_case.h"

int products_presentation(char nome[], char marca[], char descricao[],
                          char preco[], char cod_barras[], char quantidade[],
                          char filial[], char id_product[7]) {

    if (products_use_case(nome, marca, descricao, preco, cod_barras,
                          quantidade, filial, id_product) == -1) {
        fprintf(stderr, "Erro ao cadastrar produto.\n");
        return -1;
    }

    return 0;
}

int product_med(char nome[], char marca[], char descricao[], char preco[],
                char cod_barras[], char quantidade[], char filial[],
                char id_product[7], char principio_ativo[],
                char forma_farmaceutica[], char concentracao[], char tarja[],
                char receita[], char tipagem[]) {
    if (products_use_case(nome, marca, descricao, preco, cod_barras,
                          quantidade, filial, id_product) == -1) {
        fprintf(stderr, "Erro ao cadastrar produto.\n");
        return -1;
    }
    if (medicamento_use_case(id_product, principio_ativo,
                             forma_farmaceutica, concentracao, tarja, receita,
                             tipagem) == -1) {
        fprintf(stderr, "Erro ao cadastrar produto.\n");
        return -1;
    }
    return 0;
}
