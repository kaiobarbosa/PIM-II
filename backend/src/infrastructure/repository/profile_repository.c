#include "backend/config/database_connector.h"
#include "backend/src/infrastructure/repository/profile_repository.h"

#include <stdio.h>
#include <string.h>

static void copy_column(char *destination, size_t capacity, MYSQL_ROW row,
                        unsigned int column) {
    snprintf(destination, capacity, "%s",
             row[column] != NULL ? row[column] : "");
    size_t length = strlen(destination);
    while (length > 0 && destination[length - 1] == ' ')
        destination[--length] = '\0';
}

int get_profile_info(const char *user_id, const char *profile,
                     ProfileInfo *info) {
    MYSQL *conn = connect_to_database();
    if (conn == NULL) return -1;

    char query[1024];
    if (strcmp(profile, "diretor") == 0) {
        snprintf(query, sizeof(query),
                 "SELECT nome_diretor, sobrenome_diretor, codigo_interno, "
                 "'', 'Sede Principal', email_diretor "
                 "FROM diretor WHERE id_diretor = '%s'", user_id);
    } else if (strcmp(profile, "gerente") == 0) {
        snprintf(query, sizeof(query),
                 "SELECT g.nome_gerente, g.sobrenome_gerente, "
                 "g.codigo_interno, '', e.cep, g.email_gerente "
                 "FROM gerente g LEFT JOIN filial f ON f.id_filial = g.id_filial "
                 "LEFT JOIN endereco e ON e.id_endereco = f.id_endereco "
                 "WHERE g.id_gerente = '%s'", user_id);
    } else if (strcmp(profile, "farmaceutico") == 0) {
        snprintf(query, sizeof(query),
                 "SELECT p.nome_farmaceutico, p.sobrenome_farmaceutico, "
                 "p.codigo_interno, p.crf_farmaceutico, e.cep, "
                 "p.email_farmaceutico "
                 "FROM farmaceutico p "
                 "LEFT JOIN filial f ON f.id_filial = p.id_filial "
                 "LEFT JOIN endereco e ON e.id_endereco = f.id_endereco "
                 "WHERE p.id_farmaceutico = '%s'", user_id);
    } else if (strcmp(profile, "atendente") == 0) {
        snprintf(query, sizeof(query),
                 "SELECT a.nome_atendente, a.sobrenome_atendente, '', '', "
                 "e.cep, a.email_atendente "
                 "FROM atendente a "
                 "LEFT JOIN filial f ON f.id_filial = a.id_filial "
                 "LEFT JOIN endereco e ON e.id_endereco = f.id_endereco "
                 "WHERE a.id_atendente = '%s'", user_id);
    } else {
        mysql_close(conn);
        return -1;
    }

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao buscar perfil do usuário: %s\n",
                mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (result == NULL) {
        mysql_close(conn);
        return -1;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    if (row == NULL) {
        mysql_free_result(result);
        mysql_close(conn);
        return 0;
    }

    memset(info, 0, sizeof(*info));
    snprintf(info->user_id, sizeof(info->user_id), "%s", user_id);
    snprintf(info->profile, sizeof(info->profile), "%s", profile);
    copy_column(info->first_name, sizeof(info->first_name), row, 0);
    copy_column(info->last_name, sizeof(info->last_name), row, 1);
    copy_column(info->internal_code, sizeof(info->internal_code), row, 2);
    copy_column(info->crf, sizeof(info->crf), row, 3);
    copy_column(info->branch, sizeof(info->branch), row, 4);
    copy_column(info->email, sizeof(info->email), row, 5);

    mysql_free_result(result);
    mysql_close(conn);
    return 1;
}

int update_profile_info(const ProfileInfo *info, const char *password_hash) {
    MYSQL *conn = connect_to_database();
    if (conn == NULL) return -1;

    char user_id[15];
    char first_name[101];
    char last_name[101];
    char internal_code[17];
    char crf[31];
    char email[511];
    char escaped_password[129];
    char query[2048];
#define ESCAPE_FIELD(output, input) \
    mysql_real_escape_string(conn, output, input, (unsigned long)strlen(input))

    ESCAPE_FIELD(user_id, info->user_id);
    ESCAPE_FIELD(first_name, info->first_name);
    ESCAPE_FIELD(last_name, info->last_name);
    ESCAPE_FIELD(internal_code, info->internal_code);
    ESCAPE_FIELD(crf, info->crf);
    ESCAPE_FIELD(email, info->email);
    if (password_hash != NULL && password_hash[0] != '\0') {
        ESCAPE_FIELD(escaped_password, password_hash);
    } else {
        escaped_password[0] = '\0';
    }

    if (strcmp(info->profile, "diretor") == 0) {
        if (escaped_password[0] != '\0') {
            snprintf(query, sizeof(query),
                     "UPDATE diretor SET nome_diretor='%s', "
                     "sobrenome_diretor='%s', codigo_interno='%s', "
                     "email_diretor='%s', senha_diretor='%s' "
                     "WHERE id_diretor='%s'",
                     first_name, last_name, internal_code, email,
                     escaped_password, user_id);
        } else {
            snprintf(query, sizeof(query),
                     "UPDATE diretor SET nome_diretor='%s', "
                     "sobrenome_diretor='%s', codigo_interno='%s', "
                     "email_diretor='%s' WHERE id_diretor='%s'",
                     first_name, last_name, internal_code, email, user_id);
        }
    } else if (strcmp(info->profile, "gerente") == 0) {
        if (escaped_password[0] != '\0') {
            snprintf(query, sizeof(query),
                     "UPDATE gerente SET nome_gerente='%s', "
                     "sobrenome_gerente='%s', codigo_interno='%s', "
                     "email_gerente='%s', senha_gerente='%s' "
                     "WHERE id_gerente='%s'",
                     first_name, last_name, internal_code, email,
                     escaped_password, user_id);
        } else {
            snprintf(query, sizeof(query),
                     "UPDATE gerente SET nome_gerente='%s', "
                     "sobrenome_gerente='%s', codigo_interno='%s', "
                     "email_gerente='%s' WHERE id_gerente='%s'",
                     first_name, last_name, internal_code, email, user_id);
        }
    } else if (strcmp(info->profile, "farmaceutico") == 0) {
        if (escaped_password[0] != '\0') {
            snprintf(query, sizeof(query),
                     "UPDATE farmaceutico SET nome_farmaceutico='%s', "
                     "sobrenome_farmaceutico='%s', codigo_interno='%s', "
                     "crf_farmaceutico='%s', email_farmaceutico='%s', "
                     "senha_farmaceutico='%s' WHERE id_farmaceutico='%s'",
                     first_name, last_name, internal_code, crf, email,
                     escaped_password, user_id);
        } else {
            snprintf(query, sizeof(query),
                     "UPDATE farmaceutico SET nome_farmaceutico='%s', "
                     "sobrenome_farmaceutico='%s', codigo_interno='%s', "
                     "crf_farmaceutico='%s', email_farmaceutico='%s' "
                     "WHERE id_farmaceutico='%s'",
                     first_name, last_name, internal_code, crf, email,
                     user_id);
        }
    } else if (strcmp(info->profile, "atendente") == 0) {
        if (escaped_password[0] != '\0') {
            snprintf(query, sizeof(query),
                     "UPDATE atendente SET nome_atendente='%s', "
                     "sobrenome_atendente='%s', email_atendente='%s', "
                     "senha_atendente='%s' WHERE id_atendente='%s'",
                     first_name, last_name, email, escaped_password, user_id);
        } else {
            snprintf(query, sizeof(query),
                     "UPDATE atendente SET nome_atendente='%s', "
                     "sobrenome_atendente='%s', email_atendente='%s' "
                     "WHERE id_atendente='%s'",
                     first_name, last_name, email, user_id);
        }
    } else {
        mysql_close(conn);
        return -1;
    }

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao atualizar perfil do usuário: %s\n",
                mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    mysql_close(conn);
    return 0;
#undef ESCAPE_FIELD
}
