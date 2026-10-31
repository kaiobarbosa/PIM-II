#include <stdio.h>
#include <string.h>
#include <ctype.h>

int verify_null_fields_products(char nome[], char marca[], char descricao[], char preco[], char cod_barras[], char quantidade[], char filial[]) {
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

static int is_only_digits(const char *str) {
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

int verify_numeric_fields_products(char preco[], char quantidade[], char cod_barras[]) {
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
