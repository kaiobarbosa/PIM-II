#ifndef PROFILE_USE_CASE_H
#define PROFILE_USE_CASE_H

#include "backend/src/infrastructure/repository/profile_repository.h"

int get_profile_use_case(const char *user_id, const char *profile,
                         ProfileInfo *info);
int update_profile_use_case(const ProfileInfo *info,
                            const char *new_password);
int request_profile_password_change_code_use_case(
    const ProfileInfo *info, const char *current_email);
int confirm_profile_password_change_use_case(
    const ProfileInfo *info, const char *current_email,
    const char *verification_code, const char *new_password);

#endif
