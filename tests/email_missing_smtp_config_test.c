#define _DARWIN_C_SOURCE
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "backend/src/infrastructure/external_services/bcrypt/email/email.h"

int main(void) {
    char test_directory[] = "/tmp/pim-smtp-unconfigured-test-XXXXXX";

    unsetenv("SMTP_URL");
    unsetenv("SMTP_USERNAME");
    unsetenv("SMTP_PASSWORD");
    unsetenv("SMTP_FROM");
    unsetenv("SMTP_CA_BUNDLE");
    if (mkdtemp(test_directory) == NULL || chdir(test_directory) != 0) {
        fprintf(stderr, "FAIL: could not isolate the SMTP configuration test.\n");
        return 2;
    }

    if (send_password_confirmation_email("profile-test@example.invalid",
                                         "AB12CD")) {
        fprintf(stderr, "FAIL: email send unexpectedly succeeded.\n");
        return 1;
    }
    puts("PASS: email send fails closed when SMTP is not configured.");
    return 0;
}
