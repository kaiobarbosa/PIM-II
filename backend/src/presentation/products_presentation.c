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
