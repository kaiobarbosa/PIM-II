#ifndef HOME_SCREEN_H
#define HOME_SCREEN_H

typedef enum {
    APP_SCREEN_EXIT,
    APP_SCREEN_HOME,
    APP_SCREEN_EMPLOYEES,
    APP_SCREEN_BRANCHES,
    APP_SCREEN_PRODUCTS
} AppScreen;

void home_screen(char user_id[7], char *user_profile);

void draw_resize_message(int screen_height, int screen_width);

#endif
