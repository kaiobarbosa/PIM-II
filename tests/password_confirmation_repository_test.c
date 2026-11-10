#include <stdio.h>

#include "backend/config/database_connector.h"
#include "backend/src/infrastructure/repository/password_confirmation_repository.h"
#include "backend/src/utils/generate_id.h"

int main(void) {
    const char test_email[] = "codex-password-change-test@invalid.example";
    const char test_code[] = "AB12CD";
    char recovery_id[7];
    int failed = 0;

    generate_id(recovery_id);
    if (save_password_confirmation(test_email, recovery_id, test_code) != 0) {
        fprintf(stderr, "FAIL: could not insert a confirmation row.\n");
        return 1;
    }
    if (verify_password_confirmation(test_email, test_code) != 1) {
        fprintf(stderr, "FAIL: inserted confirmation code did not validate.\n");
        failed = 1;
    }
    if (verify_password_confirmation(test_email, "ZZ99ZZ") != 0) {
        fprintf(stderr, "FAIL: an incorrect confirmation code validated.\n");
        failed = 1;
    }
    {
        MYSQL *conn = connect_to_database();
        char query[256];
        if (conn == NULL) {
            fprintf(stderr, "FAIL: could not age the test confirmation row.\n");
            delete_password_confirmation(test_email, test_code);
            return 1;
        }
        snprintf(query, sizeof(query),
                 "UPDATE recuperacao_senha SET criado_em="
                 "NOW() - INTERVAL 11 MINUTE WHERE email_usuario='%s' "
                 "AND codigo_verificacao='%s'",
                 test_email, test_code);
        if (mysql_query(conn, query)) {
            fprintf(stderr, "FAIL: could not mark test code as expired.\n");
            mysql_close(conn);
            delete_password_confirmation(test_email, test_code);
            return 1;
        }
        mysql_close(conn);
    }
    if (verify_password_confirmation(test_email, test_code) != 0) {
        fprintf(stderr, "FAIL: expired confirmation code validated.\n");
        failed = 1;
    }
    if (delete_password_confirmation(test_email, test_code) != 1) {
        fprintf(stderr, "FAIL: confirmation code was not consumed.\n");
        failed = 1;
    }
    if (verify_password_confirmation(test_email, test_code) != 0) {
        fprintf(stderr, "FAIL: consumed confirmation code still validated.\n");
        failed = 1;
    }

    if (failed) return 1;
    puts("PASS: MySQL insert, code lookup, mismatch, expiry, and consumption.");
    return 0;
}
