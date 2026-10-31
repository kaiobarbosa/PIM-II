#include <string.h>
#include <stdio.h>

#include "backend/src/domain/products_domain.h"
#include "backend/src/utils/verify_user_informations.h"
#include "backend/src/infrastructure/repository/login_repository.h"
#include "backend/src/infrastructure/repository/product_repository.h"
#include "backend/src/utils/generate_id.h"

int products_use_case(char nome[], char marca[], char descricao[],
                      char preco[], char cod_barras[], char quantidade[],
                      char filial[], char id_product[7]) {

    if(verify_null_fields_products(nome, marca, descricao, preco ,cod_barras, quantidade, filial) == -1){
        fprintf(stderr, "Erro: Campos nulos detectados.\n");
        return -1;
    }else if(verify_internal_code_format(filial) == 0){
        fprintf(stderr, "Erro: Id da filial com formato incorreto");
        return -1;
    }

    int numeric_status = verify_numeric_fields_products(preco, quantidade, cod_barras);
    
    if (numeric_status == -1) {
        fprintf(stderr, "Erro: O preço possui caracteres inválidos.\n");
        return -1;
    } else if (numeric_status == -2) {
        fprintf(stderr, "Erro: A quantidade deve conter apenas números inteiros.\n");
        return -1;
    } else if (numeric_status == -3) {
        fprintf(stderr, "Erro: O código de barras deve conter apenas números.\n");
        return -1;
    }

    char generated_id[7];
    do {
        generate_id(generated_id);
    } while (check_user_id_exist(generated_id) == 1);

    if (create_product(generated_id, nome, marca, descricao, preco,
                       cod_barras, quantidade, filial) == -1) {
        fprintf(stderr, "Erro ao criar produto.\n");
        return -1;
    }

    snprintf(id_product, 7, "%s", generated_id);
    return 0;
}
