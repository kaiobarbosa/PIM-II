#include "backend/src/presentation/profile_presentation.h"

#include <stdio.h>

#include "backend/src/use_case/profile_use_case.h"

int get_profile_presentation(const char *user_id, const char *profile,
                             ProfileInfo *info) {
    int result = get_profile_use_case(user_id, profile, info);
    if (result < 0)
        fprintf(stderr, "Erro ao carregar informações do perfil.\n");
    return result;
}

int update_profile_presentation(const ProfileInfo *info,
                                const char *new_password) {
    int result = update_profile_use_case(info, new_password);
    if (result < 0)
        fprintf(stderr, "Erro ao salvar informações do perfil.\n");
    return result;
}
