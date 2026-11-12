#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "backend/src/domain/products_domain.h"
#include "backend/src/utils/verify_user_informations.h"
#include "backend/src/infrastructure/repository/login_repository.h"
#include "backend/src/infrastructure/repository/product_repository.h"
#include "backend/src/infrastructure/repository/access_control_repository.h"
#include "backend/src/utils/generate_id.h"

#include "backend/src/use_case/products_use_case.h"

int select_all_products_use_case(ProdutoItem *produtos, int max_produtos) {
    int count = select_all_products(produtos, max_produtos);

    if (count == -1) {
        fprintf(stderr, "Erro de comunicação com o banco ao buscar os produtos.\\n");
    }

    return count;
}

int select_products_by_name_use_case(ProdutoItem *produtos, int max_produtos, const char *nome) {
    int count = select_products_by_name(produtos, max_produtos, nome);

    if (count == -1) {
        fprintf(stderr, "Erro de comunicação com o banco ao buscar os produtos por nome.\\n");
    }

    return count;
}

int select_products_by_filial_use_case(ProdutoItem *produtos, int max_produtos, const char *filial) {
    int count = select_products_by_filial(produtos, max_produtos, filial);

    if (count == -1) {
        fprintf(stderr, "Erro de comunicação com o banco ao buscar os produtos por filial.\\n");
    }

    return count;
}

int get_quantidade_produto_by_id_use_case(const char *id_produto, const char *filial) {
    int quantidade = get_quantidade_produto_by_id(id_produto, filial);

    if (quantidade == -1) {
        fprintf(stderr, "Erro de comunicação com o banco ao buscar a quantidade do produto.\\n");
    }

    return quantidade;
}

int product_requires_prescription_use_case(const char *id_produto) {
    int result = product_requires_prescription(id_produto);
    if (result < 0)
        fprintf(stderr, "Erro ao consultar se o medicamento exige receita.\n");
    return result;
}

int get_product_details_use_case(const char *id_produto,
                                 ProductDetails *details) {
    return get_product_details(id_produto, details);
}

int update_product_use_case(const ProductDetails *details,
                            const char *id_funcionario) {
    int numeric_status;
    if (details == NULL || id_funcionario == NULL ||
        id_funcionario[0] == '\0' ||
        verify_null_fields_products(details->nome, details->marca,
                                    details->descricao, details->preco,
                                    details->codigo_de_barras,
                                    details->quantidade,
                                    details->id_filial) == -1 ||
        verify_internal_code_format(details->id_filial) == 0) {
        fprintf(stderr, "Erro: dados do produto incompletos ou inválidos.\n");
        return -1;
    }
    numeric_status = verify_numeric_fields_products(
        details->preco, details->quantidade, details->codigo_de_barras);
    if (numeric_status != 1 || details->kind < PRODUCT_KIND_GENERAL ||
        details->kind > PRODUCT_KIND_EQUIPMENT) {
        fprintf(stderr, "Erro: campos numéricos ou tipo do produto inválidos.\n");
        return -1;
    }
    if (details->kind == PRODUCT_KIND_MEDICINE &&
        (details->principio_ativo[0] == '\0' ||
         details->forma_farmaceutica[0] == '\0' ||
         details->concentracao[0] == '\0' || details->tarja[0] == '\0' ||
         details->tipagem[0] == '\0' ||
         (strcmp(details->receita, "sim") != 0 &&
          strcmp(details->receita, "nao") != 0 &&
          strcmp(details->receita, "não") != 0 &&
          strcmp(details->receita, "0") != 0 &&
          strcmp(details->receita, "1") != 0))) {
        fprintf(stderr, "Erro: dados do medicamento inválidos.\n");
        return -1;
    }
    if (details->kind == PRODUCT_KIND_DERMOCOSMETIC &&
        (details->tipo_pele[0] == '\0' || details->volume[0] == '\0' ||
         details->fator_protecao[0] == '\0' || details->indicacao[0] == '\0')) {
        fprintf(stderr, "Erro: dados do dermocosmético inválidos.\n");
        return -1;
    }
    if (details->kind == PRODUCT_KIND_EQUIPMENT &&
        (details->modelo[0] == '\0' || details->fabricante[0] == '\0' ||
         details->especificacoes[0] == '\0' || details->garantia[0] == '\0')) {
        fprintf(stderr, "Erro: dados do equipamento inválidos.\n");
        return -1;
    }
    if (!user_can_manage_product(id_funcionario, details->id_produto,
                                 details->id_filial)) {
        fprintf(stderr, "Erro: o usuário não pode editar este produto.\n");
        return -1;
    }
    return update_product_details(details);
}

int delete_product_use_case(const char *id_produto,
                            const char *id_funcionario) {
    if (id_produto == NULL || id_funcionario == NULL ||
        !user_can_manage_product(id_funcionario, id_produto, NULL)) {
        fprintf(stderr, "Erro: o usuário não pode excluir este produto.\n");
        return -1;
    }
    return remove_product(id_produto);
}

int select_products_by_filial_and_name_use_case(ProdutoItem *produtos, int max_produtos, const char *filial, const char *nome) {
    int count = select_products_by_filial_and_name(produtos, max_produtos, filial, nome);

    if (count == -1) {
        fprintf(stderr, "Erro de comunicação com o banco ao buscar os produtos por filial e nome.\\n");
    }

    return count;
}

int products_use_case(char nome[], char marca[], char descricao[],
                      char preco[], char cod_barras[], char quantidade[],
                      char filial[], char id_product[7], char id_funcionario[]) {

    if (!user_can_create_product_in_branch(id_funcionario, filial)) {
        fprintf(stderr, "Erro: O usuário não pode cadastrar produtos nesta filial.\n");
        return -1;
    }

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
    }else{
        char id_log[7];
        generate_id(id_log);
        if (create_log_product(id_log, "Criação", generated_id,
                               id_funcionario, filial, NULL) == -1) {
            fprintf(stderr, "Erro ao criar log de produto.\n");
            return -1;
        }
    }

    snprintf(id_product, 7, "%s", generated_id);
    return 0;
}



int medicamento_use_case(char id_produto[], char principio_ativo[], char forma_farmaceutica[], char concentracao[], char tarja[], char receita[], char tipagem[]){

    if(verify_null_fields_med(principio_ativo, forma_farmaceutica, concentracao, tarja, receita, tipagem) == -1){
        fprintf(stderr, "Erro: Campos nulos detectados.\n");
        return -1;
    }

    bool requires_prescription;

    if (strcmp(receita, "sim") == 0 || strcmp(receita, "0") == 0) {
        requires_prescription = true;
    } else if (strcmp(receita, "nao") == 0 ||
               strcmp(receita, "não") == 0 || strcmp(receita, "1") == 0) {
        requires_prescription = false;
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

    if(create_med(generated_id, principio_ativo, forma_farmaceutica,
                  concentracao, tarja, requires_prescription, tipagem) == -1){
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

int baixa_produto_estoque_use_case(char id_produto[], char quantidade[],
                                   char filial[], char id_funcionario[],
                                   char crf[]) {
    if (verify_null_fields_baixa(id_produto, quantidade, filial) == -1 ||
        id_funcionario[0] == '\0') {
        fprintf(stderr, "Erro: Campos nulos detectados.\n");
        return -1;
    } else if (verify_internal_code_format(filial) == 0) {
        fprintf(stderr, "Erro: Id da filial com formato incorreto");
        return -1;
    }

    if (!user_can_sell_product(id_funcionario, id_produto, filial, crf)) {
        fprintf(stderr, "Erro: O usuário não pode vender este produto nesta filial ou o CRF informado não autoriza a venda.\n");
        return -1;
    }

    int numeric_status = verify_numeric_fields_baixa(quantidade);

    if (numeric_status == -1) {
        fprintf(stderr, "Erro: A quantidade deve conter apenas números inteiros.\n");
        return -1;
    }

    int quantidade_estoque =
        get_quantidade_produto_by_id_use_case(id_produto, filial);
    if (quantidade_estoque < 0) {
        fprintf(stderr, "Erro ao consultar a quantidade em estoque.\n");
        return -1;
    }

    int quantidade_baixa = atoi(quantidade);

    if (quantidade_baixa <= 0) {
        fprintf(stderr, "Erro: A quantidade deve ser maior que zero.\n");
        return -1;
    } else if (quantidade_baixa > quantidade_estoque) {
        fprintf(stderr, "Erro: A quantidade a ser baixada é maior do que a quantidade em estoque.\n");
        return -1;
    }

    char id_log[7];
    generate_id(id_log);
    if (register_product_sale(id_log, id_produto, id_funcionario, filial,
                              quantidade) == -1) {
        fprintf(stderr, "Erro ao realizar e registrar a venda do produto.\n");
        return -1;
    }

    return 0;
}

int select_produtos_vendidos_by_user_id_use_case(const char *id_user, ProdutoItem *produtos, int max_produtos) {
    int count = get_produtos_vendidos_by_user_id(id_user, produtos, max_produtos);

    if (count == -1) {
        fprintf(stderr, "Erro de comunicação com o banco ao buscar os produtos vendidos pelo usuário.\\n");
    }

    return count;
}
