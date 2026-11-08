#include "backend/config/database_connector.h"
#include "backend/src/infrastructure/repository/access_control_repository.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

int get_user_access_context(const char *user_id, char *profile,
                            unsigned long profile_capacity, char branch_id[7]) {
    MYSQL *conn = connect_to_database();
    char query[512];
    char escaped_id[15];
    MYSQL_RES *result;
    MYSQL_ROW row;
    if (conn == NULL || user_id == NULL || profile == NULL ||
        branch_id == NULL || profile_capacity == 0) {
        if (conn != NULL) mysql_close(conn);
        return -1;
    }
    mysql_real_escape_string(conn, escaped_id, user_id,
                            (unsigned long)strlen(user_id));
    snprintf(query, sizeof(query),
        "SELECT perfil, COALESCE(id_filial, '') FROM ("
        "SELECT id_diretor AS id_usuario, 'diretor' AS perfil, "
        "NULL AS id_filial FROM diretor UNION ALL "
        "SELECT id_gerente, 'gerente', id_filial FROM gerente UNION ALL "
        "SELECT id_farmaceutico, 'farmaceutico', id_filial FROM farmaceutico UNION ALL "
        "SELECT id_atendente, 'atendente', id_filial FROM atendente"
        ") usuarios WHERE id_usuario='%s' LIMIT 1", escaped_id);
    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao consultar permissões do usuário: %s\n",
                mysql_error(conn));
        mysql_close(conn);
        return -1;
    }
    result = mysql_store_result(conn);
    row = result != NULL ? mysql_fetch_row(result) : NULL;
    if (row == NULL) {
        if (result != NULL) mysql_free_result(result);
        mysql_close(conn);
        return 0;
    }
    snprintf(profile, profile_capacity, "%s", row[0] != NULL ? row[0] : "");
    snprintf(branch_id, 7, "%s", row[1] != NULL ? row[1] : "");
    mysql_free_result(result);
    mysql_close(conn);
    return 1;
}

int user_can_create_product_in_branch(const char *user_id,
                                      const char *branch_id) {
    char profile[20] = "";
    char own_branch[7] = "";
    int status = get_user_access_context(user_id, profile, sizeof(profile),
                                         own_branch);
    if (status != 1) return 0;
    if (strcmp(profile, "diretor") == 0) return 1;
    return strcmp(profile, "gerente") == 0 &&
           strcmp(own_branch, branch_id) == 0;
}

int user_can_withdraw_stock_from_branch(const char *user_id,
                                        const char *branch_id) {
    char profile[20] = "";
    char own_branch[7] = "";
    int status = get_user_access_context(user_id, profile, sizeof(profile),
                                         own_branch);
    if (status != 1) return 0;
    if (strcmp(profile, "diretor") == 0 || strcmp(profile, "gerente") == 0)
        return 1;
    return (strcmp(profile, "farmaceutico") == 0 ||
            strcmp(profile, "atendente") == 0) &&
           strcmp(own_branch, branch_id) == 0;
}

static int is_severe_tarja(const char *tarja) {
    char normalized[96];
    size_t index;
    snprintf(normalized, sizeof(normalized), "%s", tarja != NULL ? tarja : "");
    for (index = 0; normalized[index] != '\0'; index++)
        normalized[index] = (char)tolower((unsigned char)normalized[index]);
    if (strstr(normalized, "sem tarja") != NULL ||
        strstr(normalized, "nao") != NULL ||
        strstr(normalized, "não") != NULL ||
        strstr(normalized, "comum") != NULL ||
        strstr(normalized, "livre") != NULL) return 0;
    return normalized[0] != '\0';
}

int user_can_sell_product(const char *user_id, const char *product_id,
                          const char *branch_id, const char *crf) {
    MYSQL *conn = connect_to_database();
    char escaped_user[15], escaped_product[15];
    char query[1400];
    MYSQL_RES *result;
    MYSQL_ROW row;
    int allowed = 0;
    if (conn == NULL || user_id == NULL || product_id == NULL ||
        branch_id == NULL || crf == NULL) {
        if (conn != NULL) mysql_close(conn);
        return 0;
    }
    mysql_real_escape_string(conn, escaped_user, user_id,
                            (unsigned long)strlen(user_id));
    mysql_real_escape_string(conn, escaped_product, product_id,
                            (unsigned long)strlen(product_id));
    snprintf(query, sizeof(query),
        "SELECT u.perfil, COALESCE(u.id_filial,''), COALESCE(u.crf,''), "
        "COALESCE(m.tarja,'') FROM ("
        "SELECT id_diretor AS id_usuario,'diretor' AS perfil,NULL AS id_filial," 
        "NULL AS crf FROM diretor UNION ALL "
        "SELECT id_gerente,'gerente',id_filial,NULL FROM gerente UNION ALL "
        "SELECT id_farmaceutico,'farmaceutico',id_filial,crf_farmaceutico "
        "FROM farmaceutico UNION ALL "
        "SELECT id_atendente,'atendente',id_filial,NULL FROM atendente"
        ") u LEFT JOIN produto_medicamento pm ON pm.id_produto='%s' "
        "LEFT JOIN medicamento m ON m.id_medicamento=pm.id_medicamento "
        "WHERE u.id_usuario='%s' LIMIT 1", escaped_product, escaped_user);
    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao validar permissão de venda: %s\n",
                mysql_error(conn));
        mysql_close(conn);
        return 0;
    }
    result = mysql_store_result(conn);
    row = result != NULL ? mysql_fetch_row(result) : NULL;
    if (row != NULL) {
        const char *profile = row[0] != NULL ? row[0] : "";
        const char *own_branch = row[1] != NULL ? row[1] : "";
        const char *crf = row[2] != NULL ? row[2] : "";
        const char *tarja = row[3] != NULL ? row[3] : "";
        int severe = is_severe_tarja(tarja);
        if (strcmp(profile, "diretor") == 0 || strcmp(profile, "gerente") == 0)
            allowed = 1;
        else if ((strcmp(profile, "farmaceutico") == 0 ||
                  strcmp(profile, "atendente") == 0) &&
                 strcmp(own_branch, branch_id) == 0) {
            if (strcmp(profile, "farmaceutico") == 0)
                allowed = !severe ||
                    (crf[0] != '\0' && strcmp(crf, row[2] != NULL ? row[2] : "") == 0);
            else
                allowed = !severe;
        }
    }
    if (result != NULL) mysql_free_result(result);
    mysql_close(conn);
    return allowed;
}
