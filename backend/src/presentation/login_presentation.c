#include <stdio.h>
#include "backend/src/use_case/login_use_case.h"

int login_presentation(char *email, char *password) {

    int result = login_use_case(email, password);

    if (result != 1) {
        fprintf(stderr, "Falha no login. Verifique suas credenciais.\n");
        return 0; 
    }

    return 1;
}
