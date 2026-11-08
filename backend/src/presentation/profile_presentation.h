#ifndef PROFILE_PRESENTATION_H
#define PROFILE_PRESENTATION_H

#include "backend/src/infrastructure/repository/profile_repository.h"

int get_profile_presentation(const char *user_id, const char *profile,
                             ProfileInfo *info);
int update_profile_presentation(const ProfileInfo *info,
                                const char *new_password);

#endif
