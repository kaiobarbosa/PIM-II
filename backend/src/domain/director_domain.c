#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int verify_null_fields(char name[], char last_name[], char internal_code[], char email[], char password[]) {
    if (name[0] == '\0' || last_name[0] == '\0' || internal_code[0] == '\0' || email[0] == '\0' || password[0] == '\0') {
        return 0; 
    }
    return 1;
}