#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int verify_null_fields_pharmacist(char name[], char last_name[], char internal_code[], char crf[], char email[], char password[], char id_branch[], char id_supervisor[]) {
    if (name[0] == '\0' || last_name[0] == '\0' || internal_code[0] == '\0' || crf[0] == '\0' || email[0] == '\0' || password[0] == '\0' || id_branch[0] == '\0' || id_supervisor[0] == '\0') {
        return 0; 
    }
    return 1;
}