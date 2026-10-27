#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int verify_null_fields_manager(char name[], char last_name[], char internal_code[], char branch[], char email[], char password[], char id_director[]) {
    if (name[0] == '\0' || last_name[0] == '\0' || internal_code[0] == '\0' || branch[0] == '\0' || email[0] == '\0' || password[0] == '\0' || id_director[0] == '\0') {
        return 0; 
    }
    return 1;
}

int verify_branch_code(const char *branch_code) {
    if (strlen(branch_code) != 6) {
        return 0; 
    }
    return 1; 
}