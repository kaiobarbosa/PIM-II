#include "backend/config/database_connector.h"
#include "backend/src/infrastructure/repository/access_control_repository.h"

#include <stdio.h>
#include <string.h>

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

int user_can_manage_product(const char *user_id, const char *product_id,
                            const char *target_branch_id) {
    MYSQL *conn;
    MYSQL_RES *result;
    MYSQL_ROW row;
    char escaped_user[13];
    char escaped_product[13];
    char query[1200];
    char profile[20] = "";
    char own_branch[7] = "";
    char product_branch[7] = "";
    int allowed = 0;

    if (user_id == NULL || product_id == NULL || strlen(user_id) != 6 ||
        strlen(product_id) != 6 ||
        (target_branch_id != NULL && strlen(target_branch_id) != 6)) {
        return 0;
    }

    conn = connect_to_database();
    if (conn == NULL) return 0;
    mysql_real_escape_string(conn, escaped_user, user_id,
                             (unsigned long)strlen(user_id));
    mysql_real_escape_string(conn, escaped_product, product_id,
                             (unsigned long)strlen(product_id));
    snprintf(query, sizeof(query),
        "SELECT u.perfil, COALESCE(u.id_filial,''), p.id_filial FROM ("
        "SELECT id_diretor AS id_usuario,'diretor' AS perfil,NULL AS id_filial "
        "FROM diretor UNION ALL "
        "SELECT id_gerente,'gerente',id_filial FROM gerente UNION ALL "
        "SELECT id_farmaceutico,'farmaceutico',id_filial FROM farmaceutico UNION ALL "
        "SELECT id_atendente,'atendente',id_filial FROM atendente"
        ") u JOIN produto p ON p.id_produto='%s' "
        "WHERE u.id_usuario='%s' LIMIT 1", escaped_product, escaped_user);
    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao validar permissões de produto: %s\n",
                mysql_error(conn));
        mysql_close(conn);
        return 0;
    }
    result = mysql_store_result(conn);
    row = result != NULL ? mysql_fetch_row(result) : NULL;
    if (row != NULL) {
        snprintf(profile, sizeof(profile), "%s",
                 row[0] != NULL ? row[0] : "");
        snprintf(own_branch, sizeof(own_branch), "%s",
                 row[1] != NULL ? row[1] : "");
        snprintf(product_branch, sizeof(product_branch), "%s",
                 row[2] != NULL ? row[2] : "");
        if (strcmp(profile, "diretor") == 0) {
            allowed = 1;
        } else if (strcmp(profile, "gerente") == 0 &&
                   strcmp(own_branch, product_branch) == 0) {
            allowed = target_branch_id == NULL ||
                      strcmp(own_branch, target_branch_id) == 0;
        }
    }
    if (result != NULL) mysql_free_result(result);
    mysql_close(conn);
    return allowed;
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

int user_can_sell_product(const char *user_id, const char *product_id,
                          const char *branch_id, const char *crf) {
    MYSQL *conn;
    char escaped_user[13];
    char escaped_product[13];
    char escaped_crf[31];
    char escaped_branch[13];
    char query[1400];
    MYSQL_RES *result;
    MYSQL_ROW row;
    char profile[20];
    char own_branch[7];
    char own_crf[16];
    int requires_prescription;
    int allowed = 0;

    if (user_id == NULL || product_id == NULL || branch_id == NULL ||
        crf == NULL) {
        return 0;
    }
    if (strlen(user_id) != 6 || strlen(product_id) != 6 ||
        strlen(branch_id) != 6 || strlen(crf) > 15) {
        return 0;
    }

    conn = connect_to_database();
    if (conn == NULL) return 0;
    mysql_real_escape_string(conn, escaped_user, user_id,
                            (unsigned long)strlen(user_id));
    mysql_real_escape_string(conn, escaped_product, product_id,
                            (unsigned long)strlen(product_id));
    mysql_real_escape_string(conn, escaped_crf, crf,
                            (unsigned long)strlen(crf));
    mysql_real_escape_string(conn, escaped_branch, branch_id,
                            (unsigned long)strlen(branch_id));
    snprintf(query, sizeof(query),
        "SELECT u.perfil, COALESCE(u.id_filial,''), COALESCE(u.crf,''), "
        "EXISTS (SELECT 1 FROM produto_medicamento pm "
        "JOIN medicamento m ON m.id_medicamento=pm.id_medicamento "
        "WHERE pm.id_produto='%s' AND m.receita=0) FROM ("
        "SELECT id_diretor AS id_usuario,'diretor' AS perfil,NULL AS id_filial," 
        "NULL AS crf FROM diretor UNION ALL "
        "SELECT id_gerente,'gerente',id_filial,NULL FROM gerente UNION ALL "
        "SELECT id_farmaceutico,'farmaceutico',id_filial,crf_farmaceutico "
        "FROM farmaceutico UNION ALL "
        "SELECT id_atendente,'atendente',id_filial,NULL FROM atendente"
        ") u WHERE u.id_usuario='%s' LIMIT 1",
        escaped_product, escaped_user);
    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erro ao validar permissão de venda: %s\n",
                mysql_error(conn));
        mysql_close(conn);
        return 0;
    }
    result = mysql_store_result(conn);
    row = result != NULL ? mysql_fetch_row(result) : NULL;
    if (row != NULL) {
        snprintf(profile, sizeof(profile), "%s",
                 row[0] != NULL ? row[0] : "");
        snprintf(own_branch, sizeof(own_branch), "%s",
                 row[1] != NULL ? row[1] : "");
        snprintf(own_crf, sizeof(own_crf), "%s",
                 row[2] != NULL ? row[2] : "");
        requires_prescription = row[3] != NULL && strcmp(row[3], "1") == 0;
    } else {
        profile[0] = '\0';
        own_branch[0] = '\0';
        own_crf[0] = '\0';
        requires_prescription = 0;
    }
    if (result != NULL) mysql_free_result(result);

    if (profile[0] != '\0') {
        if (strcmp(profile, "diretor") == 0) {
            allowed = 1;
        } else if (strcmp(own_branch, branch_id) == 0) {
            if (strcmp(profile, "gerente") == 0) {
                allowed = 1;
            } else if (strcmp(profile, "farmaceutico") == 0) {
                allowed = !requires_prescription || own_crf[0] != '\0';
            } else if (strcmp(profile, "atendente") == 0) {
                if (!requires_prescription) {
                    allowed = 1;
                } else if (crf[0] != '\0') {
                    MYSQL_RES *crf_result;
                    MYSQL_ROW crf_row;
                    snprintf(query, sizeof(query),
                             "SELECT 1 FROM farmaceutico "
                             "WHERE crf_farmaceutico='%s' AND id_filial='%s' "
                             "LIMIT 1",
                             escaped_crf, escaped_branch);
                    if (mysql_query(conn, query)) {
                        fprintf(stderr,
                                "Erro ao validar o CRF do farmacêutico: %s\n",
                                mysql_error(conn));
                    } else {
                        crf_result = mysql_store_result(conn);
                        crf_row = crf_result != NULL ?
                                  mysql_fetch_row(crf_result) : NULL;
                        allowed = crf_row != NULL;
                        if (crf_result != NULL) mysql_free_result(crf_result);
                    }
                }
            }
        }
    }
    mysql_close(conn);
    return allowed;
}
