#ifndef DIRECTOR_DOMAIN_H
#define DIRECTOR_DOMAIN_H

int verify_null_fields(char name[], char last_name[], char internal_code[], char email[], char password[]);

int verify_internal_code_format(const char *internal_code);

int verify_email_format(const char *email);

#endif