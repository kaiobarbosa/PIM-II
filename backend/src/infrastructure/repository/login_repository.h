#ifndef LOGIN_REPOSITORY_H
#define LOGIN_REPOSITORY_H

int get_user_by_email(const char *email, char *hash_armazenado, char *perfil_usuario, int *id_usuario);

#endif
