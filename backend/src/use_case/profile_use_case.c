#include "backend/src/use_case/profile_use_case.h"

#include <stdio.h>
#include <string.h>

#include "backend/src/domain/validation/password_crypt.h"
#include "backend/src/infrastructure/repository/profile_repository.h"
#include "backend/src/utils/verify_user_informations.h"

int get_profile_use_case(const char *user_id, const char *profile,
                         ProfileInfo *info) {
    return get_profile_info(user_id, profile, info);
}

int update_profile_use_case(const ProfileInfo *info,
                            const char *new_password) {
    if (info == NULL || info->user_id[0] == '\0' ||
        info->first_name[0] == '\0' || info->last_name[0] == '\0' ||
        info->email[0] == '\0' || verify_email_format(info->email) == 0) {
        fprintf(stderr, "Erro: Verifique os campos e o formato do e-mail.\n");
        return -1;
    }

    if ((strcmp(info->profile, "diretor") == 0 ||
         strcmp(info->profile, "gerente") == 0) &&
        strlen(info->internal_code) != 6) {
        fprintf(stderr, "Erro: O código interno deve conter 6 caracteres.\n");
        return -1;
    }
    if (strcmp(info->profile, "farmaceutico") == 0 &&
        (strlen(info->internal_code) != 6 || info->crf[0] == '\0')) {
        fprintf(stderr, "Erro: Informe código interno e CRF válidos.\n");
        return -1;
    }
    if (strcmp(info->profile, "atendente") != 0 &&
        strcmp(info->profile, "diretor") != 0 &&
        strcmp(info->profile, "gerente") != 0 &&
        strcmp(info->profile, "farmaceutico") != 0) {
        return -1;
    }

    char hashed_password[64] = "";
    if (new_password != NULL && new_password[0] != '\0' &&
        !password_hash(new_password, hashed_password,
                       (int)sizeof(hashed_password))) {
        fprintf(stderr, "Erro ao proteger a nova senha.\n");
        return -1;
    }

    return update_profile_info(info, hashed_password);
}
