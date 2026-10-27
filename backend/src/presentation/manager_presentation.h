#ifndef MANAGER_PRESENTATION_H
#define MANAGER_PRESENTATION_H

int register_new_manager(char name_manager[], char last_name_manager[], char internal_code_manager[], char branch_manager[], char email_manager[], char password_manager[], char id_manager[]);

int check_branch_exists(const char *branch_code);

#endif
