#include <stdio.h>
#include <ncurses.h>
#include <string.h>

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
    if (screen_width > (int) strlen(message) + 2) {
        mvprintw(screen_height / 2 - 1,
                 (screen_width - (int) strlen(message)) / 2, "%s", message);
    }
    attroff(COLOR_PAIR(1) | A_BOLD);

    if (screen_width > (int) strlen(required_size) + 2 && screen_height > 1) {
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

static int read_action(int screen_height, int screen_width) {
    const char *register_button = "[ CADASTRAR ]";
    const char *cancel_button = "[ CANCELAR ]";
    int register_x = (screen_width - 31) / 2;
    int cancel_x = register_x + 20;
    int selected = 0;
    int character;

    while (1) {
        move(screen_height - 2, 0);
        clrtoeol();

        if (selected == 0) {
            attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
        } else {
            attron(COLOR_PAIR(2) | A_BOLD);
        }
        mvprintw(screen_height - 2, register_x, "%s", register_button);
        attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);

        if (selected == 1) {
            attron(COLOR_PAIR(1) | A_BOLD | A_REVERSE);
        } else {
            attron(COLOR_PAIR(1) | A_BOLD);
        }
        mvprintw(screen_height - 2, cancel_x, "%s", cancel_button);
        attroff(COLOR_PAIR(1) | A_BOLD | A_REVERSE);

        attron(A_DIM);
        mvprintw(screen_height - 1, (screen_width - 31) / 2,
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

void create_account_screen() {
    char director_name[51] = "";
    char director_last_name[51] = "";
    char director_internal_code[21] = "";
    char director_email[101] = "";
    char director_password[51] = "";
    int screen_height;
    int screen_width;
    int form_width;
    int form_left;
    int first_field_y;
    const char *title = "CADASTRO DE DIRETOR";

    initscr();

    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);

    start_color();
    use_default_colors();
    init_pair(1, COLOR_CYAN, -1);
    init_pair(2, COLOR_GREEN, -1);

    while (!terminal_is_large_enough()) {
        show_resize_message();
        getch();
    }

    getmaxyx(stdscr, screen_height, screen_width);
    form_width = 66;
    if (screen_width < form_width + 2) {
        form_width = screen_width - 4;
    }
    form_left = (screen_width - form_width) / 2;
    first_field_y = 7;

    box(stdscr, 0, 0);

    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(2, (screen_width - (int) strlen(title)) / 2, "%s", title);
    attroff(COLOR_PAIR(1) | A_BOLD);

    attron(COLOR_PAIR(2));
    mvhline(4, 2, ACS_HLINE, screen_width - 4);
    mvprintw(5, form_left, "INFORMACOES DE ACESSO");
    attroff(COLOR_PAIR(2));

    attron(A_BOLD);
    mvprintw(first_field_y, form_left, "Nome");
    mvprintw(first_field_y + 3, form_left, "Sobrenome");
    mvprintw(first_field_y + 6, form_left, "Codigo interno");
    mvprintw(first_field_y + 9, form_left, "Email");
    mvprintw(first_field_y + 12, form_left, "Senha");
    attroff(A_BOLD);

    attron(COLOR_PAIR(1));
    for (int field = 0; field < 5; field++) {
        int field_y = first_field_y + 1 + (field * 3);

        mvaddch(field_y, form_left, '[');
        mvhline(field_y, form_left + 1, ACS_HLINE, form_width - 3);
        mvaddch(field_y, form_left + form_width - 2, ']');
    }
    attroff(COLOR_PAIR(1));

    refresh();

    read_field(first_field_y + 1, form_left + 2, form_width - 4,
               director_name, sizeof(director_name), 0);
    read_field(first_field_y + 4, form_left + 2, form_width - 4,
               director_last_name, sizeof(director_last_name), 0);
    read_field(first_field_y + 7, form_left + 2, form_width - 4,
               director_internal_code, sizeof(director_internal_code), 0);
    read_field(first_field_y + 10, form_left + 2, form_width - 4,
               director_email, sizeof(director_email), 0);
    read_field(first_field_y + 13, form_left + 2, form_width - 4,
               director_password, sizeof(director_password), 1);

    if (read_action(screen_height, screen_width) == 1) {
        endwin();
        return;
    }

    endwin();

}
