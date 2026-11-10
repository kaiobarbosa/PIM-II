#include <stdio.h>
#include <string.h>

#include "backend/src/domain/validation/password_crypt.h"
#include "backend/src/infrastructure/external_services/bcrypt/email/email.h"
#include "backend/src/infrastructure/repository/password_confirmation_repository.h"
#include "backend/src/infrastructure/repository/profile_repository.h"
#include "backend/src/use_case/profile_use_case.h"

static int update_calls;
static int verify_code_result;
static int delete_code_calls;
static int mail_result = 1;
static char last_email[256];
static char last_code[7];
static char last_password_hash[64];

int get_profile_info(const char *user_id, const char *profile,
                     ProfileInfo *info) {
    (void)user_id;
    (void)profile;
    (void)info;
    return 1;
}

int update_profile_info(const ProfileInfo *info, const char *password_hash) {
    (void)info;
    update_calls++;
    snprintf(last_password_hash, sizeof(last_password_hash), "%s",
             password_hash != NULL ? password_hash : "");
    return 0;
}

int profile_email_matches_user(const char *user_id, const char *profile,
                               const char *email) {
    (void)user_id;
    (void)profile;
    return strcmp(email, "current@example.com") == 0;
}

int save_password_confirmation(const char *email, const char *recovery_id,
                               const char *verification_code) {
    (void)recovery_id;
    snprintf(last_email, sizeof(last_email), "%s", email);
    snprintf(last_code, sizeof(last_code), "%s", verification_code);
    return 0;
}

int verify_password_confirmation(const char *email,
                                 const char *verification_code) {
    if (strcmp(email, "current@example.com") != 0 ||
        strcmp(verification_code, "AB12CD") != 0) {
        return 0;
    }
    return verify_code_result;
}

int delete_password_confirmation(const char *email,
                                 const char *verification_code) {
    (void)email;
    (void)verification_code;
    delete_code_calls++;
    return 1;
}

int send_password_confirmation_email(const char *recipient,
                                     const char *verification_code) {
    snprintf(last_email, sizeof(last_email), "%s", recipient);
    snprintf(last_code, sizeof(last_code), "%s", verification_code);
    return mail_result;
}

int password_hash(const char *password, char *hash, int hash_size) {
    if (password == NULL || hash_size < 10) return 0;
    snprintf(hash, (size_t)hash_size, "test-hash");
    return 1;
}

static int check(int condition, const char *description) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", description);
        return 0;
    }
    return 1;
}

int main(void) {
    ProfileInfo info = {0};

    snprintf(info.user_id, sizeof(info.user_id), "ABC123");
    snprintf(info.profile, sizeof(info.profile), "atendente");
    snprintf(info.first_name, sizeof(info.first_name), "Ada");
    snprintf(info.last_name, sizeof(info.last_name), "Lovelace");
    snprintf(info.email, sizeof(info.email), "new@example.com");

    if (!check(update_profile_use_case(&info, "nova-senha") == -1 &&
                   update_calls == 0,
               "a atualizacao direta nao pode salvar senha sem codigo")) {
        return 1;
    }
    if (!check(update_profile_use_case(&info, "") == 0 &&
                   update_calls == 1 && last_password_hash[0] == '\0',
               "a edicao de perfil sem troca de senha continua funcionando")) {
        return 1;
    }

    update_calls = 0;
    verify_code_result = 0;
    if (!check(confirm_profile_password_change_use_case(
                   &info, "current@example.com", "AB12CD", "nova-senha") ==
                       -1 &&
                   update_calls == 0 && delete_code_calls == 0,
               "codigo invalido ou expirado bloqueia a alteracao")) {
        return 1;
    }

    verify_code_result = 1;
    if (!check(confirm_profile_password_change_use_case(
                   &info, "current@example.com", "AB12CD", "nova-senha") ==
                       0 &&
                   update_calls == 1 && delete_code_calls == 1 &&
                   strcmp(last_password_hash, "test-hash") == 0,
               "codigo valido consome o codigo e salva o hash")) {
        return 1;
    }

    if (!check(request_profile_password_change_code_use_case(
                   &info, "current@example.com") == 0 &&
                   strcmp(last_email, "current@example.com") == 0 &&
                   strlen(last_code) == 6,
               "o codigo e enviado ao email atual do perfil")) {
        return 1;
    }

    mail_result = 0;
    delete_code_calls = 0;
    if (!check(request_profile_password_change_code_use_case(
                   &info, "current@example.com") == -1 &&
                   delete_code_calls == 1,
               "falha de envio remove o codigo salvo")) {
        return 1;
    }

    puts("PASS: 6 verificacoes do caso de uso de alteracao de senha.");
    return 0;
}
