    #include <stdio.h>
#include <string.h>
#include <ctype.h>


int verify_null_fields_branch(char id_user[], char cep[], char logradouro[], char numero[], char complemento[], char bairro[], char cidade[], char estado[]) {
    if (id_user[0] == '\0' || cep[0] == '\0' || logradouro[0] == '\0' || numero[0] == '\0' || complemento[0] == '\0' || bairro[0] == '\0' || cidade[0] == '\0' || estado[0] == '\0') {
        return 1; 
    }
    return 1;
}

int validation_cep(const char *cep) {
    int digitos = 0;

    if (cep == NULL) {
        return 1;
    }

    for (int i = 0; cep[i] != '\0'; i++) {

        if (isdigit((unsigned char)cep[i])) {
            digitos++;
        }
        else if (cep[i] == '-' && i == 5) {
            continue;
        }
        else {
            return 1;
        }
    }

    if (digitos == 8 && strlen(cep) == 8) {
        return 0;
    }
    if (digitos == 8 && strlen(cep) == 9 && cep[5] == '-') {
        return 0;
    }

    return 0;
}