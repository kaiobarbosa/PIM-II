#include <stdio.h>
#include <string.h>
#include <ctype.h>

int verify_null_fields_products(const char *nome, const char *marca,
                                const char *descricao, const char *preco,
                                const char *cod_barras, const char *quantidade,
                                const char *filial) {
    if (nome[0] == '\0' || marca[0] == '\0' || descricao[0] == '\0' || preco[0] == '\0' || cod_barras[0] == '\0' || quantidade[0] == '\0' || filial[0] == '\0') {
        return -1;
    }
    return 1;
}

int verify_null_fields_med(char principio_ativo[], char forma_farmaceutica[], char concentracao[], char tarja[], char receita[], char tipagem[]) {
    if (principio_ativo[0] == '\0' || forma_farmaceutica[0] == '\0' || concentracao[0] == '\0' || tarja[0] == '\0' || receita[0] == '\0' || tipagem[0] == '\0') {
        return -1; 
    }
    return 1;
}

int verify_null_fields_equip(char modelo[], char fabricante[], char especificacoes[], char garantia[]){
    if (modelo[0] == '\0' || fabricante[0] == '\0' || especificacoes[0] == '\0' || garantia[0] == '\0') {
        return -1;
    }
    return 1;
}

int verify_null_fields_dermo(char tipo_pele[], char volume[], char fator_protecao[], char indicacao[]){
    if (tipo_pele[0] == '\0' || volume[0] == '\0' || fator_protecao[0] == '\0' || indicacao[0] == '\0') {
        return -1;
    }
    return 1;
}

int is_only_digits(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isdigit((unsigned char)str[i])) {
            return -1; 
        }
    }
    return 1; 
}

static int is_valid_price(const char *str) {
    int count_pontuacao = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '.' || str[i] == ',') {
            count_pontuacao++;
            if (count_pontuacao > 1) return 0; 
        } else if (!isdigit((unsigned char)str[i])) {
            return -1; 
        }
    }
    return 1;
}

int verify_numeric_fields_products(const char *preco, const char *quantidade,
                                   const char *cod_barras) {
    if (is_valid_price(preco) == -1) {
        return -1; 
    }
    if (is_only_digits(quantidade) == -1) {
        return -2; 
    }
    if (is_only_digits(cod_barras) == -1) {
        return -3;
    }
    
    return 1; 
}

int verify_null_fields_baixa(char id_produto[], char quantidade[], char filial[]) {
    if (id_produto[0] == '\0' || quantidade[0] == '\0' || filial[0] == '\0') {
        return -1;
    }
    return 1;
}

int verify_numeric_fields_baixa(char quantidade[]) {
    return is_only_digits(quantidade) == 1 ? 1 : -1;
}
