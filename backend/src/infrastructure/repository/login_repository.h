#ifndef LOGIN_REPOSITORY_H
#define LOGIN_REPOSITORY_H

// Busca o usuário pelo e-mail na View. 
// Retorna 1 se encontrar (preenchendo o hash e o perfil) ou 0 se não encontrar.
int get_user_by_email(const char *email, char *hash_armazenado, char *perfil_usuario);

#endif
