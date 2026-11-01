#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "backend/src/domain/products_domain.h"
#include "backend/src/utils/verify_user_informations.h"
#include "backend/src/infrastructure/repository/login_repository.h"
#include "backend/src/infrastructure/repository/product_repository.h"
#include "backend/src/utils/generate_id.h"

#include "backend/src/use_case/products_use_case.h"

int select_all_products_use_case(ProdutoItem *produtos, int max_produtos) {
    int count = select_all_products(produtos, max_produtos);
    
    if (count == -1) {
        fprintf(stderr, "Erro de comunicação com o banco ao buscar os produtos.\\n");
    }
    
    return count;
}

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
    int id_status;
    do {
        generate_id(generated_id);
        id_status = check_user_id_exist(generated_id);
        if (id_status < 0) {
            fprintf(stderr, "Erro ao verificar disponibilidade do ID do produto.\n");
            return -1;
        }
    } while (id_status == 1);

    if (create_product(generated_id, nome, marca, descricao, preco,
                       cod_barras, quantidade, filial) == -1) {
        fprintf(stderr, "Erro ao criar produto.\n");
        return -1;
    }

    snprintf(id_product, 7, "%s", generated_id);
    return 0;
}



int medicamento_use_case(char id_produto[], char principio_ativo[], char forma_farmaceutica[], char concentracao[], char tarja[], char receita[], char tipagem[]){

    if(verify_null_fields_med(principio_ativo, forma_farmaceutica, concentracao, tarja, receita, tipagem) == -1){
        fprintf(stderr, "Erro: Campos nulos detectados.\n");
        return -1;
    }
    
    bool receita_bool;

    if (strcmp(receita, "sim") == 0 || strcmp(receita, "1") == 0) {
        receita_bool = true;
    } else if (strcmp(receita, "nao") == 0 ||
               strcmp(receita, "não") == 0 || strcmp(receita, "0") == 0) {
        receita_bool = false;
    } else {
        fprintf(stderr, "Erro: Receita digitada, inválida.\n");
        return -1;
    }

    char generated_id[7];
    int id_status;
    do {
        generate_id(generated_id);
        id_status = check_user_id_exist(generated_id);
        if (id_status < 0) {
            fprintf(stderr, "Erro ao verificar disponibilidade do ID do medicamento.\n");
            return -1;
        }
    } while (id_status == 1);

    if(create_med(generated_id, principio_ativo, forma_farmaceutica, concentracao, tarja, receita_bool, tipagem) == -1){
        fprintf(stderr, "Erro: Nao foi possivel criar medicamento.\n");
        return -1;
    }else if(create_prod_med(id_produto, generated_id) == -1){
        fprintf(stderr, "Erro: Nao foi possivel criar produto_medicamento.\n");
        return -1;
    }

    return 0;
}

int equipamento_use_case (char id_produto[], char modelo[], char fabricante[], char especificacoes[], char garantia[]){

    if(verify_null_fields_equip(modelo, fabricante, especificacoes, garantia) == -1){
        fprintf(stderr, "Erro: Campos nulos detectados.\n");
        return -1;
    }

    char generated_id[7];
    int id_status;
    do {
        generate_id(generated_id);
        id_status = check_user_id_exist(generated_id);
        if (id_status < 0) {
            fprintf(stderr, "Erro ao verificar disponibilidade do ID do equipamento.\n");
            return -1;
        }
    } while (id_status == 1);

    if(create_equip(generated_id, modelo, fabricante, especificacoes, garantia) == -1){
        fprintf(stderr, "Erro: Nao foi possivel criar o equipamento.\n");
        return -1;
    }else if(create_prod_equip(id_produto, generated_id) == -1){
        fprintf(stderr, "Erro: Nao foi possivel criar o produto_equipamento.\n");
        return -1;
    }

    return 0;
}

int dermocosmetico_use_case(char id_produto[], char tipo_pele[], char volume[], char fator_protecao[], char indicacao[]){

    if(verify_null_fields_dermo(tipo_pele, volume, fator_protecao, indicacao) == -1){
        fprintf(stderr, "Erro: Campos nulos detectados.\n");
        return -1;
    }

    char generated_id[7];
    int id_status;
    do {
        generate_id(generated_id);
        id_status = check_user_id_exist(generated_id);
        if (id_status < 0) {
            fprintf(stderr, "Erro ao verificar disponibilidade do ID do dermocosmetico.\n");
            return -1;
        }
    } while (id_status == 1);

    if(create_dermo(generated_id, tipo_pele, volume, fator_protecao, indicacao) == -1){
        fprintf(stderr, "Erro: Nao foi possivel criar o dermocosmetico.\n");
        return -1;
    }else if(create_prod_dermo(id_produto, generated_id) == -1){
        fprintf(stderr, "Erro: Nao foi possivel criar o produto_dermo.\n");
        return -1;
    }

    return 0;
}
