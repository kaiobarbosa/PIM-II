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
