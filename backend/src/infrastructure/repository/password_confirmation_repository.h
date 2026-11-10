#ifndef PASSWORD_CONFIRMATION_REPOSITORY_H
#define PASSWORD_CONFIRMATION_REPOSITORY_H

int profile_email_matches_user(const char *user_id, const char *profile,
                               const char *email);
int save_password_confirmation(const char *email, const char *recovery_id,
                               const char *verification_code);
int verify_password_confirmation(const char *email,
                                 const char *verification_code);
int delete_password_confirmation(const char *email,
                                 const char *verification_code);

#endif
