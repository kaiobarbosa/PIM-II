#include <ncurses.h>
#include <stdio.h>
#include <string.h>

#include "backend/src/presentation/branch_presentation.h"
#include "backend/src/presentation/products_presentation.h"
#include "frontend/components/sidebar.h"
#include "frontend/screens/employees_screen.h"
#include "frontend/screens/home_screen.h"
#include "frontend/screens/branch_screen.h"
#include "frontend/screens/product_screen.h"

#define MIN_SCREEN_HEIGHT 24
#define MIN_SCREEN_WIDTH 70
#define SEARCH_BUFFER_SIZE 256
#define MAX_HOME_PRODUCTS 128
#define MAX_HOME_BRANCHES 128
#define HOME_PRODUCTS_PER_PAGE 6

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

static void draw_search_results(int screen_height, int hero_left,
                                int hero_width, const ProdutoItem products[],
                                int product_count,
                                const BranchItem branches[], int branch_count,
                                int scroll, int search_error) {
    int table_x = hero_left + 3;
    int table_width = hero_width - 6;
    int name_width = table_width * 42 / 100;
    int quantity_width = 5;
    int price_width = 9;
    int branch_width = table_width - name_width - quantity_width - price_width - 3;
    int quantity_x = table_x + name_width + 1;
    int price_x = quantity_x + quantity_width + 1;
    int branch_x = price_x + price_width + 1;
    int row;

    attron(COLOR_PAIR(1) | A_BOLD);
    mvaddnstr(10, table_x, "RESULTADOS DA PESQUISA", table_width);
    attroff(COLOR_PAIR(1) | A_BOLD);

    if (search_error) {
        mvaddnstr(12, table_x, "Falha ao pesquisar produtos.", table_width);
        return;
    }
    if (product_count == 0) {
        mvaddnstr(12, table_x, "Nenhum produto encontrado.", table_width);
        return;
    }

    attron(COLOR_PAIR(1) | A_BOLD);
    mvaddnstr(11, table_x, "NOME", name_width);
    mvaddnstr(11, quantity_x, "QTD", quantity_width);
    mvaddnstr(11, price_x, "PRECO", price_width);
    if (branch_width > 0) mvaddnstr(11, branch_x, "FILIAL", branch_width);
    attroff(COLOR_PAIR(1) | A_BOLD);
    mvhline(12, table_x, ACS_HLINE, table_width);

    for (row = 0; row < HOME_PRODUCTS_PER_PAGE && row + scroll < product_count;
         row++) {
        const ProdutoItem *product = &products[row + scroll];
        char branch_label[101];
        int branch_index;
        snprintf(branch_label, sizeof(branch_label), "%s", product->filial);
        for (branch_index = 0; branch_index < branch_count; branch_index++) {
            if (strcmp(product->filial, branches[branch_index].id_filial) == 0) {
                snprintf(branch_label, sizeof(branch_label), "%s",
                         branches[branch_index].cep);
                break;
            }
        }
        mvaddnstr(13 + row, table_x, product->nome, name_width);
        mvaddnstr(13 + row, quantity_x, product->quantidade, quantity_width);
        mvaddnstr(13 + row, price_x, product->preco, price_width);
        if (branch_width > 0)
            mvaddnstr(13 + row, branch_x, branch_label, branch_width);
    }

    if (product_count > HOME_PRODUCTS_PER_PAGE) {
        char page_status[48];
        int last = scroll + HOME_PRODUCTS_PER_PAGE < product_count ?
                   scroll + HOME_PRODUCTS_PER_PAGE : product_count;
        snprintf(page_status, sizeof(page_status), "%d-%d de %d",
                 scroll + 1, last, product_count);
        mvaddnstr(screen_height - 3, table_x, page_status, table_width);
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
    int search_error = 0;
    int product_count = 0;
    int branch_count;
    int result_scroll = 0;
    ProdutoItem products[MAX_HOME_PRODUCTS] = {{0}};
    BranchItem branches[MAX_HOME_BRANCHES] = {{0}};
    char search_query[SEARCH_BUFFER_SIZE] = "";

    branch_count = select_all_branches_presentation(branches,
                                                     MAX_HOME_BRANCHES);
    if (branch_count < 0) branch_count = 0;

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
        if (search_confirmed) {
            draw_search_results(screen_height, hero_left, hero_width, products,
                                product_count, branches, branch_count,
                                result_scroll, search_error);
        } else {
            draw_recent_outputs(screen_height, hero_left, hero_width);
        }

        attron(A_DIM);
        if (editing_search) {
            mvaddnstr(screen_height - 2, 2,
                      "Digite o produto  |  Enter: confirmar  |  Esc: cancelar",
                      screen_width - 4);
        } else {
            mvaddnstr(screen_height - 2, 2,
                      "Setas: mover | PgUp/PgDn: resultados | Enter: selecionar | Esc: sair",
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
                result_scroll = 0;
                search_error = 0;
                product_count = search_query[0] == '\0' ?
                    select_all_products_presentation(products,
                                                     MAX_HOME_PRODUCTS) :
                    select_products_by_name_presentation(
                        products, MAX_HOME_PRODUCTS, search_query);
                if (product_count < 0) {
                    search_error = 1;
                    product_count = 0;
                }
            } else if (character == 27) {
                editing_search = 0;
                search_confirmed = 0;
                search_query[0] = '\0';
                search_length = 0;
                product_count = 0;
                result_scroll = 0;
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
        } else if (character == KEY_NPAGE && search_confirmed &&
                   result_scroll + HOME_PRODUCTS_PER_PAGE < product_count) {
            result_scroll += HOME_PRODUCTS_PER_PAGE;
        } else if (character == KEY_PPAGE && search_confirmed) {
            result_scroll = result_scroll >= HOME_PRODUCTS_PER_PAGE ?
                            result_scroll - HOME_PRODUCTS_PER_PAGE : 0;
        } else if (character == '\n' || character == KEY_ENTER) {
            if (focus == 5) {
                editing_search = 1;
                search_confirmed = 0;
                search_length = (int) strlen(search_query);
            } else if (focus == 3) {
                return APP_SCREEN_BRANCHES;
            } else if (focus == 2) {    
                return APP_SCREEN_EMPLOYEES;
            } else if (focus == 1) {
                return APP_SCREEN_PRODUCTS;
            } else if (focus == 0) {
                return APP_SCREEN_HOME;
            }
        } else if (character == 27) {
            curs_set(0);
            return APP_SCREEN_EXIT;
        }
    }
}

void home_screen(char user_id[7], char *user_profile) {
    AppScreen current_screen = APP_SCREEN_HOME;

    while (current_screen != APP_SCREEN_EXIT) {
        if (current_screen == APP_SCREEN_HOME) {
            current_screen = home_screen_view();
        } else if (current_screen == APP_SCREEN_EMPLOYEES) {
            current_screen = (AppScreen) employee_screen(user_id, user_profile);
        } else if (current_screen == APP_SCREEN_BRANCHES) {
            current_screen = (AppScreen) branch_screen(user_id, user_profile);
        } else if (current_screen == APP_SCREEN_PRODUCTS) {
            current_screen = (AppScreen) product_screen(user_id, user_profile);
        } else {
            current_screen = APP_SCREEN_HOME;
        }
    }
    curs_set(0);
}
