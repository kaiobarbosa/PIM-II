#include <ncurses.h>
#include <string.h>

#include "backend/src/presentation/login_presentation.h"
#include "frontend/screens/home_screen.h"

#define MIN_SCREEN_HEIGHT 24
#define MIN_SCREEN_WIDTH 70

static int terminal_is_large_enough(void) {
    int screen_height;
    int screen_width;

    getmaxyx(stdscr, screen_height, screen_width);
    return screen_height >= MIN_SCREEN_HEIGHT &&
           screen_width >= MIN_SCREEN_WIDTH;
}

static void show_resize_message(void) {
    int screen_height;
    int screen_width;
    const char *message = "Aumente o terminal para continuar";
    const char *required_size = "Tamanho minimo: 70 colunas x 24 linhas";

    clear();
    getmaxyx(stdscr, screen_height, screen_width);
    box(stdscr, 0, 0);

    attron(COLOR_PAIR(1) | A_BOLD);
    if (screen_width > (int) strlen(message) + 2 && screen_height > 2) {
        mvprintw(screen_height / 2 - 1,
                 (screen_width - (int) strlen(message)) / 2,
                 "%s", message);
    }
    attroff(COLOR_PAIR(1) | A_BOLD);

    if (screen_width > (int) strlen(required_size) + 2 && screen_height > 3) {
        mvprintw(screen_height / 2 + 1,
                 (screen_width - (int) strlen(required_size)) / 2,
                 "%s", required_size);
    }

    refresh();
}

static void read_field(int y, int x, int width, char *value, int value_size,
                       int masked) {
    int position = 0;
    int character;

    curs_set(1);
    move(y, x);

    while ((character = getch()) != '\n' && character != KEY_ENTER &&
           character != 27) {
        if ((character == KEY_BACKSPACE || character == 127 || character == 8) &&
            position > 0) {
            position--;
            mvaddch(y, x + position, ' ');
            move(y, x + position);
        } else if (character >= 32 && character <= 126 &&
                   position < value_size - 1 && position < width) {
            value[position++] = (char) character;
            addch(masked ? '*' : character);
        }
    }

    value[position] = '\0';
    curs_set(0);
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

        character = getch();
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

    while (!terminal_is_large_enough()) {
        show_resize_message();
        getch();
    }

    getmaxyx(stdscr, screen_height, screen_width);
    form_width = 66;
    form_left = (screen_width - form_width) / 2;

    while (1) {
        draw_login_form(screen_width, form_width, form_left, title);

        read_field(9, form_left + 2, form_width - 4,
                   user_email, sizeof(user_email), 0);
        read_field(13, form_left + 2, form_width - 4,
                   user_password, sizeof(user_password), 1);

        int action = read_login_action(screen_height, screen_width);

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
            getch();
            
            home_screen(session_user_id, session_user_profile); 
            
            break; 
            
        } else {
            
            attron(COLOR_PAIR(3) | A_BOLD);
            mvprintw(screen_height - 5, (screen_width - 28) / 2, "E-mail ou senha incorretos.");
            attroff(COLOR_PAIR(3) | A_BOLD);
            refresh();
            getch();

            user_email[0] = '\0';
            user_password[0] = '\0';
        }
    }

    endwin();
}
