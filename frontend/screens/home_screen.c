#include <ncurses.h>
#include <string.h>

#include "frontend/components/sidebar.h"
#include "frontend/screens/employees_screen.h"
#include "frontend/screens/home_screen.h"
#include "frontend/screens/branch_screen.h"

#define MIN_SCREEN_HEIGHT 24
#define MIN_SCREEN_WIDTH 70
#define SEARCH_BUFFER_SIZE 256

void draw_resize_message(int screen_height, int screen_width) {
    const char *message = "Aumente o terminal para continuar";
    const char *required_size = "Tamanho minimo: 70 colunas x 24 linhas";

    erase();
    box(stdscr, 0, 0);
    if (screen_width > (int) strlen(message) + 2 && screen_height > 2) {
        mvprintw(screen_height / 2 - 1,
                 (screen_width - (int) strlen(message)) / 2, "%s", message);
    }
    if (screen_width > (int) strlen(required_size) + 2 && screen_height > 3) {
        mvprintw(screen_height / 2 + 1,
                 (screen_width - (int) strlen(required_size)) / 2,
                 "%s", required_size);
    }
    refresh();
    getch();
}

static void draw_search_section(int hero_left, int hero_width, int divider_y,
                                int focus, int editing, int confirmed,
                                const char *query) {
    const char *heading = "Busca rapida de produtos";
    int input_width = hero_width - 10;
    int input_left = hero_left + 4;

    attron(COLOR_PAIR(1) | A_BOLD);
    mvaddnstr(2, hero_left + 3, heading, hero_width - 6);
    attroff(COLOR_PAIR(1) | A_BOLD);

    attron(A_DIM);
    mvaddnstr(4, hero_left + 3, confirmed ? "Busca confirmada" :
              "Consulte um produto pelo nome", hero_width - 6);
    attroff(A_DIM);

    if (input_width > 4) {
        attron(COLOR_PAIR(focus == 5 ? 2 : 1) |
               (focus == 5 ? A_BOLD | A_REVERSE : 0));
        mvaddch(6, input_left, '[');
        mvhline(6, input_left + 1, ACS_HLINE, input_width - 2);
        mvaddch(6, input_left + input_width - 1, ']');
        if (query[0] != '\0') {
            mvaddnstr(6, input_left + 2, query, input_width - 4);
        } else if (!editing) {
            mvaddnstr(6, input_left + 2, "Digite o nome do produto...",
                      input_width - 4);
        }
        attroff(COLOR_PAIR(focus == 5 ? 2 : 1) |
                (focus == 5 ? A_BOLD | A_REVERSE : 0));
        if (editing) {
            int cursor_x = input_left + 2 + (int) strlen(query);
            if (cursor_x > input_left + input_width - 2) {
                cursor_x = input_left + input_width - 2;
            }
            move(6, cursor_x);
        }
    }
    mvhline(divider_y, hero_left + 1, ACS_HLINE, hero_width - 2);
}

static void draw_recent_outputs(int screen_height, int hero_left,
                                int hero_width) {
    int row;
    int content_width = hero_width - 6;

    attron(COLOR_PAIR(1) | A_BOLD);
    mvaddnstr(10, hero_left + 3, "Ultimas saidas do estoque", content_width);
    attroff(COLOR_PAIR(1) | A_BOLD);
    attron(A_DIM);
    mvaddnstr(11, hero_left + 3,
              "Exibicao ilustrativa ate a integracao com o estoque", content_width);
    attroff(A_DIM);

    attron(COLOR_PAIR(2) | A_BOLD);
    mvaddnstr(13, hero_left + 3, "PRODUTO", content_width / 2);
    mvaddnstr(13, hero_left + hero_width - 18, "SAIDA", 12);
    attroff(COLOR_PAIR(2) | A_BOLD);
    mvhline(14, hero_left + 3, ACS_HLINE, content_width);

    for (row = 0; row < 3 && 16 + row * 2 < screen_height - 4; row++) {
        int product_y = 16 + row * 2;
        if (row == 0) {
            mvaddnstr(product_y, hero_left + 3, "Exemplo: Dipirona 500 mg", content_width / 2);
            mvaddnstr(product_y, hero_left + hero_width - 18, "--/--", 12);
        } else if (row == 1) {
            mvaddnstr(product_y, hero_left + 3, "Exemplo: Paracetamol 750 mg", content_width / 2);
            mvaddnstr(product_y, hero_left + hero_width - 18, "--/--", 12);
        } else if (row == 2) {
            mvaddnstr(product_y, hero_left + 3, "Exemplo: Amoxicilina 500 mg", content_width / 2);
            mvaddnstr(product_y, hero_left + hero_width - 18, "--/--", 12);
        }
    }
}

static AppScreen home_screen_view(void) {
    int screen_height;
    int screen_width;
    int sidebar_width;
    int hero_left;
    int hero_width;
    int divider_y;
    int focus = 0;
    int character;
    int editing_search = 0;
    int search_confirmed = 0;
    int search_length = 0;
    char search_query[SEARCH_BUFFER_SIZE] = "";

    getmaxyx(stdscr, screen_height, screen_width);
    if (screen_height < MIN_SCREEN_HEIGHT || screen_width < MIN_SCREEN_WIDTH) {
        draw_resize_message(screen_height, screen_width);
        return APP_SCREEN_EXIT;
    }

    sidebar_width = screen_width / 3;
    hero_left = sidebar_width + 1;
    hero_width = screen_width - hero_left - 1;
    divider_y = 1 + (screen_height - 2) / 3;

    keypad(stdscr, TRUE);
    while (1) {
        erase();
        box(stdscr, 0, 0);
        mvvline(1, sidebar_width, ACS_VLINE, screen_height - 2);
        mvhline(divider_y, hero_left, ACS_HLINE, screen_width - hero_left - 1);

        draw_sidebar(screen_height, sidebar_width, focus == 5 ? -1 : focus);
        draw_search_section(hero_left, hero_width, divider_y, focus,
                            editing_search, search_confirmed, search_query);
        draw_recent_outputs(screen_height, hero_left, hero_width);

        attron(A_DIM);
        if (editing_search) {
            mvaddnstr(screen_height - 2, 2,
                      "Digite o produto  |  Enter: confirmar  |  Esc: cancelar",
                      screen_width - 4);
        } else {
            mvaddnstr(screen_height - 2, 2,
                      "Setas: mover  |  Enter: selecionar  |  Esc: sair",
                      screen_width - 4);
        }
        attroff(A_DIM);
        curs_set(editing_search ? 1 : 0);
        refresh();

        character = getch();
        if (editing_search) {
            if (character == '\n' || character == KEY_ENTER) {
                editing_search = 0;
                search_confirmed = 1;
            } else if (character == 27) {
                editing_search = 0;
                search_confirmed = 0;
                search_query[0] = '\0';
                search_length = 0;
            } else if (character == KEY_BACKSPACE || character == 127 ||
                       character == 8) {
                if (search_length > 0) {
                    search_query[--search_length] = '\0';
                }
            } else if (character >= 32 && character <= 255 &&
                       search_length < SEARCH_BUFFER_SIZE - 1) {
                search_query[search_length++] = (char) character;
                search_query[search_length] = '\0';
            }
        } else if (character == KEY_UP) {
            focus = focus == 5 ? 4 : focus == 0 ? 4 : focus - 1;
        } else if (character == KEY_DOWN) {
            focus = focus == 5 ? 0 : (focus + 1) % 5;
        } else if (character == KEY_LEFT || character == KEY_RIGHT) {
            focus = focus == 5 ? 0 : 5;
            
        } else if (character == '\n' || character == KEY_ENTER) {
            if (focus == 5) {
                editing_search = 1;
                search_confirmed = 0;
                search_length = (int) strlen(search_query);
            } else if (focus == 3) {
                return APP_SCREEN_BRANCHES;
            } else if (focus == 2) {    
                return APP_SCREEN_EMPLOYEES;
            } else if (focus == 0) {
                return APP_SCREEN_HOME;
            }
        } else if (character == 27) {
            curs_set(0);
            return APP_SCREEN_EXIT;
        }
    }
}

void home_screen(void) {
    AppScreen current_screen = APP_SCREEN_HOME;

    while (current_screen != APP_SCREEN_EXIT) {
        if (current_screen == APP_SCREEN_HOME) {
            current_screen = home_screen_view();
        } else if (current_screen == APP_SCREEN_EMPLOYEES) {
            current_screen = (AppScreen) employee_screen();
        } else if (current_screen == APP_SCREEN_BRANCHES) {
            current_screen = (AppScreen) branch_screen();
        } else {
            current_screen = APP_SCREEN_HOME;
        }
    }
    curs_set(0);
}
