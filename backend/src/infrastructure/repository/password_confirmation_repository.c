#include "backend/src/infrastructure/repository/password_confirmation_repository.h"

#include <stdio.h>
#include <string.h>

#include "backend/config/database_connector.h"

static int escape_value(MYSQL *conn, char *destination, size_t capacity,
                        const char *value) {
    unsigned long length;

    if (value == NULL) return 0;
    length = (unsigned long)strlen(value);
    if (capacity < (size_t)(length * 2 + 1)) return 0;
    mysql_real_escape_string(conn, destination, value, length);
    return 1;
}

int profile_email_matches_user(const char *user_id, const char *profile,
                               const char *email) {
    MYSQL *conn = connect_to_database();
    char escaped_user_id[13];
    char escaped_profile[41];
    char escaped_email[511];
    char query[768];
    MYSQL_RES *result;
    int matches = 0;

    if (conn == NULL) return -1;
    if (!escape_value(conn, escaped_user_id, sizeof(escaped_user_id), user_id) ||
        !escape_value(conn, escaped_profile, sizeof(escaped_profile), profile) ||
        !escape_value(conn, escaped_email, sizeof(escaped_email), email)) {
        mysql_close(conn);
        return -1;
    }

    snprintf(query, sizeof(query),
             "SELECT 1 FROM vw_login_usuarios WHERE id_usuario='%s' "
             "AND perfil='%s' AND email='%s' LIMIT 1",
             escaped_user_id, escaped_profile, escaped_email);
    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao validar o e-mail do perfil: %s\n",
                mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    result = mysql_store_result(conn);
    if (result == NULL) {
        fprintf(stderr, "Erro ao ler o e-mail do perfil: %s\n",
                mysql_error(conn));
        mysql_close(conn);
        return -1;
    }
    matches = mysql_fetch_row(result) != NULL;
    mysql_free_result(result);
    mysql_close(conn);
    return matches;
}

int save_password_confirmation(const char *email, const char *recovery_id,
                               const char *verification_code) {
    MYSQL *conn = connect_to_database();
    char escaped_email[511];
    char escaped_id[13];
    char escaped_code[13];
    char query[768];

    if (conn == NULL) return -1;
    if (!escape_value(conn, escaped_email, sizeof(escaped_email), email) ||
        !escape_value(conn, escaped_id, sizeof(escaped_id), recovery_id) ||
        !escape_value(conn, escaped_code, sizeof(escaped_code),
                      verification_code)) {
        mysql_close(conn);
        return -1;
    }

    if (mysql_query(conn, "START TRANSACTION")) {
        mysql_close(conn);
        return -1;
    }
    snprintf(query, sizeof(query),
             "DELETE FROM recuperacao_senha WHERE email_usuario='%s'",
             escaped_email);
    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao remover codigos anteriores: %s\n",
                mysql_error(conn));
        mysql_query(conn, "ROLLBACK");
        mysql_close(conn);
        return -1;
    }

    snprintf(query, sizeof(query),
             "INSERT INTO recuperacao_senha "
             "(id_recuperacao, email_usuario, codigo_verificacao) "
             "VALUES ('%s', '%s', '%s')",
             escaped_id, escaped_email, escaped_code);
    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao salvar o codigo de confirmacao: %s\n",
                mysql_error(conn));
        mysql_query(conn, "ROLLBACK");
        mysql_close(conn);
        return -1;
    }

    if (mysql_query(conn, "COMMIT")) {
        fprintf(stderr, "Erro ao confirmar o codigo no banco: %s\n",
                mysql_error(conn));
        mysql_query(conn, "ROLLBACK");
        mysql_close(conn);
        return -1;
    }
    mysql_close(conn);
    return 0;
}

int verify_password_confirmation(const char *email,
                                 const char *verification_code) {
    MYSQL *conn = connect_to_database();
    char escaped_email[511];
    char escaped_code[13];
    char query[768];
    MYSQL_RES *result;
    int matches = 0;

    if (conn == NULL) return -1;
    if (!escape_value(conn, escaped_email, sizeof(escaped_email), email) ||
        !escape_value(conn, escaped_code, sizeof(escaped_code),
                      verification_code)) {
        mysql_close(conn);
        return -1;
    }

    snprintf(query, sizeof(query),
             "SELECT id_recuperacao FROM recuperacao_senha "
             "WHERE email_usuario='%s' AND codigo_verificacao='%s' "
             "AND criado_em >= NOW() - INTERVAL 10 MINUTE LIMIT 1",
             escaped_email, escaped_code);
    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao validar o codigo de confirmacao: %s\n",
                mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    result = mysql_store_result(conn);
    if (result == NULL) {
        fprintf(stderr, "Erro ao ler o codigo de confirmacao: %s\n",
                mysql_error(conn));
        mysql_close(conn);
        return -1;
    }
    matches = mysql_fetch_row(result) != NULL;
    mysql_free_result(result);
    mysql_close(conn);
    return matches;
}

int delete_password_confirmation(const char *email,
                                 const char *verification_code) {
    MYSQL *conn = connect_to_database();
    char escaped_email[511];
    char escaped_code[13];
    char query[768];
    int deleted;

    if (conn == NULL) return -1;
    if (!escape_value(conn, escaped_email, sizeof(escaped_email), email) ||
        !escape_value(conn, escaped_code, sizeof(escaped_code),
                      verification_code)) {
        mysql_close(conn);
        return -1;
    }

    snprintf(query, sizeof(query),
             "DELETE FROM recuperacao_senha WHERE email_usuario='%s' "
             "AND codigo_verificacao='%s'",
             escaped_email, escaped_code);
    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao remover o codigo de confirmacao: %s\n",
                mysql_error(conn));
        mysql_close(conn);
        return -1;
    }
    deleted = mysql_affected_rows(conn) > 0;
    mysql_close(conn);
    return deleted;
}
