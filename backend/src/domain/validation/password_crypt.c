#include <stdio.h>
#include <string.h>

#include "backend/src/infrastructure/external_services/bcrypt/bcrypt.h"
#include "backend/src/domain/validation/password_crypt.h"

#define CUSTO_BCRYPT 12


int password_hash(const char *senha, char *hash, int tamanho_hash) {
    if (senha == NULL || hash == NULL || tamanho_hash < BCRYPT_HASHSIZE) {
        return 0;
    }

    char salt[BCRYPT_HASHSIZE];

    if (bcrypt_gensalt(CUSTO_BCRYPT, salt) != 0) {
        return 0; 
    }

    if (bcrypt_hashpw(senha, salt, hash) != 0) {
        return 0; 
    }

    return 1; 
}

int password_verify(const char *senha_digitada, const char *hash_armazenado) {
    if (senha_digitada == NULL || hash_armazenado == NULL) {
        return 0;
    }

    if (bcrypt_checkpw(senha_digitada, hash_armazenado) == 0) {
        return 1; 
    }

    return 0; 
}
