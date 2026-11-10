#include "backend/src/use_case/profile_use_case.h"

#include <stdio.h>
#include <string.h>

#include "backend/src/domain/validation/password_crypt.h"
#include "backend/src/infrastructure/external_services/bcrypt/email/email.h"
#include "backend/src/infrastructure/repository/password_confirmation_repository.h"
#include "backend/src/infrastructure/repository/profile_repository.h"
#include "backend/src/utils/generate_id.h"
#include "backend/src/utils/verify_user_informations.h"

static int validate_profile_info(const ProfileInfo *info) {
    if (info == NULL || info->user_id[0] == '\0' ||
        info->first_name[0] == '\0' || info->last_name[0] == '\0' ||
        info->email[0] == '\0' || verify_email_format(info->email) == 0) {
        fprintf(stderr, "Erro: Verifique os campos e o formato do e-mail.\n");
        return 0;
    }

    if ((strcmp(info->profile, "diretor") == 0 ||
         strcmp(info->profile, "gerente") == 0) &&
        strlen(info->internal_code) != 6) {
        fprintf(stderr, "Erro: O codigo interno deve conter 6 caracteres.\n");
        return 0;
    }
    if (strcmp(info->profile, "farmaceutico") == 0 &&
        (strlen(info->internal_code) != 6 || info->crf[0] == '\0')) {
        fprintf(stderr, "Erro: Informe codigo interno e CRF validos.\n");
        return 0;
    }
    if (strcmp(info->profile, "atendente") != 0 &&
        strcmp(info->profile, "diretor") != 0 &&
        strcmp(info->profile, "gerente") != 0 &&
        strcmp(info->profile, "farmaceutico") != 0) {
        return 0;
    }
    return 1;
}

static int valid_confirmation_code(const char *code) {
    size_t index;
    if (code == NULL || strlen(code) != 6) return 0;
    for (index = 0; index < 6; index++) {
        if (!((code[index] >= 'A' && code[index] <= 'Z') ||
              (code[index] >= '0' && code[index] <= '9'))) {
            return 0;
        }
    }
    return 1;
}

int get_profile_use_case(const char *user_id, const char *profile,
                         ProfileInfo *info) {
    return get_profile_info(user_id, profile, info);
}

int update_profile_use_case(const ProfileInfo *info,
                            const char *new_password) {
    if (!validate_profile_info(info)) {
        return -1;
    }

    if (new_password != NULL && new_password[0] != '\0') {
        fprintf(stderr, "A alteracao de senha exige confirmacao por e-mail.\n");
        return -1;
    }

    return update_profile_info(info, "");
}

int request_profile_password_change_code_use_case(
    const ProfileInfo *info, const char *current_email) {
    char recovery_id[7];
    char verification_code[7];
    int result;

    if (!validate_profile_info(info) || current_email == NULL ||
        verify_email_format(current_email) == 0) {
        return -1;
    }

    result = profile_email_matches_user(info->user_id, info->profile,
                                        current_email);
    if (result != 1) {
        fprintf(stderr, "O e-mail atual nao corresponde ao perfil.\n");
        return -1;
    }

    generate_id(recovery_id);
    generate_id(verification_code);
    if (save_password_confirmation(current_email, recovery_id,
                                   verification_code) != 0) {
        return -1;
    }

    if (!send_password_confirmation_email(current_email, verification_code)) {
        delete_password_confirmation(current_email, verification_code);
        return -1;
    }
    return 0;
}

int confirm_profile_password_change_use_case(
    const ProfileInfo *info, const char *current_email,
    const char *verification_code, const char *new_password) {
    char hashed_password[64] = "";
    int result;

    if (!validate_profile_info(info) || current_email == NULL ||
        verify_email_format(current_email) == 0 ||
        !valid_confirmation_code(verification_code) ||
        new_password == NULL || new_password[0] == '\0') {
        fprintf(stderr, "Dados invalidos para confirmar a alteracao de senha.\n");
        return -1;
    }

    result = profile_email_matches_user(info->user_id, info->profile,
                                        current_email);
    if (result != 1) {
        fprintf(stderr, "O e-mail atual nao corresponde ao perfil.\n");
        return -1;
    }
    result = verify_password_confirmation(current_email, verification_code);
    if (result != 1) {
        fprintf(stderr, "Codigo de confirmacao invalido ou expirado.\n");
        return -1;
    }
    if (!password_hash(new_password, hashed_password,
                       (int)sizeof(hashed_password))) {
        fprintf(stderr, "Erro ao proteger a nova senha.\n");
        return -1;
    }

    result = delete_password_confirmation(current_email, verification_code);
    if (result != 1) {
        fprintf(stderr, "Nao foi possivel consumir o codigo de confirmacao.\n");
        return -1;
    }
    if (update_profile_info(info, hashed_password) != 0) {
        fprintf(stderr, "Erro ao salvar o perfil apos confirmar a senha.\n");
        return -2;
    }
    return 0;
}
