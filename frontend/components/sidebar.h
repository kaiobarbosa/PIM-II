#ifndef SIDEBAR_H
#define SIDEBAR_H

void draw_sidebar(int screen_height, int sidebar_width, int focus);
void draw_profile_sidebar(int screen_height, int sidebar_width,
                          const char *first_name, const char *last_name,
                          const char *profile,
                          const char *internal_code, const char *crf,
                          const char *branch, const char *email,
                          int password_info_visible, int focus, int editing);

#endif
