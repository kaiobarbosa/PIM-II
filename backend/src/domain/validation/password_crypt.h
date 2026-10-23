#ifndef PASSWORD_CRYPT_H
#define PASSWORD_CRYPT_H

int password_hash(const char *senha, char *hash, int tamanho_hash);
int password_verify(const char *senha_digitada, const char *hash_armazenado);

#endif
