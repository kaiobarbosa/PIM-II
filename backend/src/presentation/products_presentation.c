#include <stdio.h>
#include <stdlib.h>

#include "backend/src/use_case/products_use_case.h"
#include "backend/src/presentation/products_presentation.h"

int select_all_products_presentation(ProdutoItem *produtos, int max_produtos) {

    int result = select_all_products_use_case(produtos, max_produtos);

    if (result == -1) {
        fprintf(stderr, "Falha na camada de apresentacao ao resgatar produtos.\\n");
        return -1;
    }

    return result;
}

int select_products_by_name_presentation(ProdutoItem *produtos, int max_produtos, const char *nome) {

    int result = select_products_by_name_use_case(produtos, max_produtos, nome);

    if (result == -1) {
        fprintf(stderr, "Falha na camada de apresentacao ao resgatar produtos por nome.\\n");
        return -1;
    }

    return result;
}

int select_products_by_filial_presentation(ProdutoItem *produtos, int max_produtos, const char *filial) {

    int result = select_products_by_filial_use_case(produtos, max_produtos, filial);

    if (result == -1) {
        fprintf(stderr, "Falha na camada de apresentacao ao resgatar produtos por filial.\\n");
        return -1;
    }

    return result;
}

int select_products_by_filial_and_name_presentation(ProdutoItem *produtos, int max_produtos, const char *filial, const char *nome) {

    int result = select_products_by_filial_and_name_use_case(produtos, max_produtos, filial, nome);

    if (result == -1) {
        fprintf(stderr, "Falha na camada de apresentacao ao resgatar produtos por filial e nome.\\n");
        return -1;
    }

    return result;
}

int products_presentation(char nome[], char marca[], char descricao[],
                          char preco[], char cod_barras[], char quantidade[],
                          char filial[], char id_product[7], char id_funcionario[]) {

    if (products_use_case(nome, marca, descricao, preco, cod_barras,
                          quantidade, filial, id_product, id_funcionario) == -1) {
        fprintf(stderr, "Erro ao cadastrar produto.\n");
        return -1;
    }

    return 0;
}

int product_med(char nome[], char marca[], char descricao[], char preco[],
                char cod_barras[], char quantidade[], char filial[],
                char id_product[7], char principio_ativo[],
                char forma_farmaceutica[], char concentracao[], char tarja[],
                char receita[], char tipagem[], char id_funcionario[]) {
    if (products_use_case(nome, marca, descricao, preco, cod_barras,
                          quantidade, filial, id_product, id_funcionario) == -1) {
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

int product_equip(char nome[], char marca[], char descricao[], char preco[],
                  char cod_barras[], char quantidade[], char filial[],
                  char id_product[7], char modelo[], char fabricante[],
                  char especificacoes[], char garantia[], char id_funcionario[]) {
    if (products_use_case(nome, marca, descricao, preco, cod_barras,
                          quantidade, filial, id_product, id_funcionario) == -1) {
        fprintf(stderr, "Erro ao cadastrar produto.\n");
        return -1;
    }
    if (equipamento_use_case(id_product, modelo,
                             fabricante, especificacoes, garantia) == -1) {
        fprintf(stderr, "Erro ao cadastrar equipamento.\n");
        return -1;
    }
    return 0;
}

int product_dermo(char nome[], char marca[], char descricao[], char preco[],
                  char cod_barras[], char quantidade[], char filial[],
                  char id_product[7], char tipo_pele[], char volume[],
                  char fator_protecao[], char indicacao[], char id_funcionario[]) {
    if (products_use_case(nome, marca, descricao, preco, cod_barras,
                          quantidade, filial, id_product, id_funcionario) == -1) {
        fprintf(stderr, "Erro ao cadastrar produto.\n");
        return -1;
    }
    if (dermocosmetico_use_case(id_product, tipo_pele,
                             volume, fator_protecao, indicacao) == -1) {
        fprintf(stderr, "Erro ao cadastrar dermocosmetico.\n");
        return -1;
    }
    return 0;
}

int baixa_produto_estoque_presentation(char id_produto[], char quantidade[],
                                       char filial[], char id_funcionario[],
                                       char crf[]) {
    if (baixa_produto_estoque_use_case(id_produto, quantidade, filial,
                                       id_funcionario, crf) == -1) {
        fprintf(stderr, "Erro ao realizar a baixa do produto.\n");
        return -1;
    }
    return 0;
}

int product_requires_prescription_presentation(const char *id_produto) {
    return product_requires_prescription_use_case(id_produto);
}

int select_produtos_vendidos_by_user_id_presentation(const char *id_user, ProdutoItem *produtos, int max_produtos) {
    int result = select_produtos_vendidos_by_user_id_use_case(id_user, produtos, max_produtos);

    if (result == -1) {
        fprintf(stderr, "Falha na camada de apresentacao ao resgatar produtos vendidos por usuario.\\n");
        return -1;
    }

    return result;
}
