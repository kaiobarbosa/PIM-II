#include <stdio.h>
#include "backend/src/use_case/login_use_case.h"

int login_presentation(char *email, char *password, char logged_id[7],
                       char *logged_profile) {

    int result = login_use_case(email, password, logged_id, logged_profile);

    if (result != 1) {
        fprintf(stderr, "Falha no login. Verifique suas credenciais.\n");
        return 0; 
    }

    return 1;
}
