#include <stdio.h>
#include <stdlib.h>
#include <time.h> 

static int semente_iniciada = 0;

void generate_id(char id[]) {
    if (semente_iniciada == 0) {
        srand((unsigned int)time(NULL));
        semente_iniciada = 1;
    }

    const char caracteres[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    int tamanho_caracteres = sizeof(caracteres) - 1;

    for (int i = 0; i < 6; i++) {
        id[i] = caracteres[rand() % tamanho_caracteres];
    }

    id[6] = '\0';
}