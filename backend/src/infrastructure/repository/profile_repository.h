#ifndef PROFILE_REPOSITORY_H
#define PROFILE_REPOSITORY_H

typedef struct {
    char user_id[7];
    char profile[20];
    char first_name[51];
    char last_name[51];
    char internal_code[9];
    char crf[16];
    char branch[51];
    char email[256];
} ProfileInfo;

int get_profile_info(const char *user_id, const char *profile,
                     ProfileInfo *info);
int update_profile_info(const ProfileInfo *info, const char *password_hash);

#endif
