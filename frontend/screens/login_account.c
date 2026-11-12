#include <ncurses.h>
#include "frontend/components/unicode_input.h"
#include <string.h>

#include "backend/src/presentation/login_presentation.h"
#include "frontend/screens/home_screen.h"

static int read_field(int y, int x, int width, char *value, int value_size,
                      int masked) {
    size_t position = strlen(value);
    int display_position = masked ? input_string_character_count(value) :
                                    input_string_display_width(value);
    int character;

    curs_set(1);
    move(y, x);
    if (masked) {
        int index;
        for (index = 0; index < display_position; index++) addch('*');
    } else {
        mvadd_utf8_clipped(y, x, value, width);
    }
    move(y, x + display_position);

    while (1) {
        character = read_input_character();
        if (character == KEY_RESIZE) {
            curs_set(0);
            return 1;
        }
        if (character == '\n' || character == KEY_ENTER || character == 27)
            break;
        if ((character == KEY_BACKSPACE || character == 127 || character == 8) &&
            position > 0) {
            input_backspace(value);
            position = strlen(value);
            display_position = masked ?
                               input_string_character_count(value) :
                               input_string_display_width(value);
            mvhline(y, x, ' ', width);
            if (masked) {
                int index;
                for (index = 0; index < display_position; index++)
                    mvaddch(y, x + index, '*');
            } else {
                mvadd_utf8_clipped(y, x, value, width);
            }
            move(y, x + display_position);
        } else if (input_character_is_printable(character) &&
                   position < (size_t)value_size - 1) {
            size_t previous_position = position;
            int character_width = input_character_display_width(character);
            if (display_position + (masked ? 1 : character_width) > width)
                continue;
            if (input_character_append(value, (size_t)value_size, &position,
                                       character)) {
                if (masked) {
                    addch('*');
                    display_position++;
                } else {
                    addnstr(value + previous_position,
                            (int)(position - previous_position));
                    display_position += character_width;
                }
            }
        }
    }

    value[position] = '\0';
    curs_set(0);
    return 0;
}

static void draw_login_form(int screen_width, int form_width, int form_left,
                            const char *title) {
    const int first_field_y = 8;

    clear();
    box(stdscr, 0, 0);

    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(2, (screen_width - (int) strlen(title)) / 2, "%s", title);
    attroff(COLOR_PAIR(1) | A_BOLD);

    attron(COLOR_PAIR(2));
    mvhline(4, 2, ACS_HLINE, screen_width - 4);
    mvprintw(5, form_left, "ENTRE COM SUA CONTA");
    attroff(COLOR_PAIR(2));

    attron(A_BOLD);
    mvprintw(first_field_y, form_left, "Email");
    mvprintw(first_field_y + 4, form_left, "Senha");
    attroff(A_BOLD);

    attron(COLOR_PAIR(1));
    for (int field = 0; field < 2; field++) {
        int field_y = first_field_y + 1 + (field * 4);

        mvaddch(field_y, form_left, '[');
        mvhline(field_y, form_left + 1, ACS_HLINE, form_width - 3);
        mvaddch(field_y, form_left + form_width - 2, ']');
    }
    attroff(COLOR_PAIR(1));

    refresh();
}

static int read_login_action(int screen_height, int screen_width) {
    const char *login_button = "[ ENTRAR ]";
    const char *cancel_button = "[ CANCELAR ]";
    int buttons_width = 27;
    int login_x = (screen_width - buttons_width) / 2;
    int cancel_x = login_x + 16;
    int selected = 0;
    int character;

    while (1) {
        mvhline(screen_height - 3, 1, ' ', screen_width - 2);

        if (selected == 0) {
            attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
        } else {
            attron(COLOR_PAIR(2) | A_BOLD);
        }
        mvprintw(screen_height - 3, login_x, "%s", login_button);
        attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);

        if (selected == 1) {
            attron(COLOR_PAIR(1) | A_BOLD | A_REVERSE);
        } else {
            attron(COLOR_PAIR(1) | A_BOLD);
        }
        mvprintw(screen_height - 3, cancel_x, "%s", cancel_button);
        attroff(COLOR_PAIR(1) | A_BOLD | A_REVERSE);

        attron(A_DIM);
        mvprintw(screen_height - 2, (screen_width - 35) / 2,
                 "Use as setas ou TAB para selecionar");
        attroff(A_DIM);
        refresh();

        character = read_input_character();
        if (character == KEY_RESIZE) return -1;
        if (character == KEY_LEFT || character == KEY_RIGHT ||
            character == '\t') {
            selected = !selected;
        } else if (character == '\n' || character == KEY_ENTER) {
            return selected;
        } else if (character == 27) {
            return 1;
        }
    }
}

void login_account_screen() {
    char user_email[101] = "";       
    char user_password[51] = "";
    char session_user_id[7] = "";
    char session_user_profile[20] = "";
    int screen_height;
    int screen_width;
    int form_width;
    int form_left;
    const char *title = "FARMASTOCK | LOGIN";
    initscr();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);

    start_color();
    use_default_colors();
    init_pair(1, COLOR_CYAN, -1);
    init_pair(2, COLOR_GREEN, -1);
    init_pair(3, COLOR_RED, -1); 

    wait_for_minimum_terminal_size();

    while (1) {
        wait_for_minimum_terminal_size();
        getmaxyx(stdscr, screen_height, screen_width);
        form_width = 66;
        form_left = (screen_width - form_width) / 2;
        draw_login_form(screen_width, form_width, form_left, title);

        if (read_field(9, form_left + 2, form_width - 4,
                       user_email, sizeof(user_email), 0) ||
            read_field(13, form_left + 2, form_width - 4,
                       user_password, sizeof(user_password), 1))
            continue;

        int action = read_login_action(screen_height, screen_width);
        if (action < 0) continue;

        if (action == 1) { 
            break;
        }

        int login_result = login_presentation(user_email, user_password,
                                             session_user_id,
                                             session_user_profile);

        if (login_result == 1) {
            clear();
            attron(COLOR_PAIR(2) | A_BOLD);
            mvprintw(screen_height / 2, (screen_width - 16) / 2, "Acesso Liberado!");
            attroff(COLOR_PAIR(2) | A_BOLD);
            refresh();
            read_input_character();
            
            home_screen(session_user_id, session_user_profile); 
            
            break; 
            
        } else {
            
            attron(COLOR_PAIR(3) | A_BOLD);
            mvprintw(screen_height - 5, (screen_width - 28) / 2, "E-mail ou senha incorretos.");
            attroff(COLOR_PAIR(3) | A_BOLD);
            refresh();
            read_input_character();

            user_email[0] = '\0';
            user_password[0] = '\0';
        }
    }

    endwin();
}
