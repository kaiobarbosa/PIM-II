#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int verify_null_fields(char name[], char last_name[], char internal_code[], char email[], char password[]) {
    if (name[0] == '\0' || last_name[0] == '\0' || internal_code[0] == '\0' || email[0] == '\0' || password[0] == '\0') {
        return 0; 
    }
    return 1;
}

int verify_internal_code_format(const char *internal_code) {
     if (strlen(internal_code) != 6) {
        return 0;
     }
    return 1; 
}

int verify_email_format(const char *email) {
    int at_count = 0;
    const char *at_position = NULL;

    for (const char *ptr = email; *ptr != '\0'; ++ptr) {
        if (*ptr == '@') {
            at_count++;
            at_position = ptr;
        }
    }

    if (at_count != 1 || at_position == email || *(at_position + 1) == '\0') {
        return 0; 
    }

    return 1; 
}