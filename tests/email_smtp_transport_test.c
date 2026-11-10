#define _DARWIN_C_SOURCE
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "backend/src/infrastructure/external_services/bcrypt/email/email.h"

int main(void) {
    char test_directory[] = "/tmp/pim-smtp-local-test-XXXXXX";
    const char *test_flag = getenv("PIM_SMTP_LOCAL_TEST");
    const char *smtp_url = getenv("SMTP_URL");

    if (test_flag == NULL || strcmp(test_flag, "1") != 0 ||
        smtp_url == NULL ||
        (strncmp(smtp_url, "smtp://localhost:", 17) != 0 &&
         strncmp(smtp_url, "smtp://127.0.0.1:", 17) != 0)) {
        fprintf(stderr,
                "Set PIM_SMTP_LOCAL_TEST=1 and a localhost SMTP_URL.\n");
        return 2;
    }
    if (mkdtemp(test_directory) == NULL || chdir(test_directory) != 0) {
        fprintf(stderr, "FAIL: could not isolate the local SMTP test.\n");
        return 2;
    }

    if (!send_password_confirmation_email("profile-test@example.invalid",
                                          "AB12CD")) {
        fprintf(stderr, "FAIL: SMTP transport returned an error.\n");
        return 1;
    }
    puts("PASS: SMTP transport accepted the confirmation message.");
    return 0;
}
