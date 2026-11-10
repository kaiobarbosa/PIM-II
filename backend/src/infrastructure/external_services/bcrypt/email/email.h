#ifndef EMAIL_H
#define EMAIL_H

int send_password_confirmation_email(const char *recipient,
                                     const char *verification_code);

#endif
