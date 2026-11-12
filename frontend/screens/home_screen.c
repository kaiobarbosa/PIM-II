#include <ncurses.h>
#include "frontend/components/unicode_input.h"
#include <stdio.h>
#include <string.h>

#include "backend/src/presentation/branch_presentation.h"
#include "backend/src/presentation/products_presentation.h"
#include "backend/src/presentation/profile_presentation.h"
#include "frontend/components/sidebar.h"
#include "frontend/screens/employees_screen.h"
#include "frontend/screens/home_screen.h"
#include "frontend/screens/branch_screen.h"
#include "frontend/screens/product_screen.h"
#include "frontend/screens/profile_screen.h"

#define SEARCH_BUFFER_SIZE 256
#define MAX_HOME_PRODUCTS 128
#define MAX_HOME_BRANCHES 128
#define HOME_PRODUCTS_PER_PAGE 6

static void draw_search_section(int hero_left, int hero_width, int divider_y,
                                int focus, int editing, int confirmed,
                                const char *query) {
    const char *heading = "Busca rapida de produtos";
    int input_width = hero_width - 10;
    int input_left = hero_left + 4;

    attron(COLOR_PAIR(1) | A_BOLD);
    mvadd_utf8_clipped(2, hero_left + 3, heading, hero_width - 6);
    attroff(COLOR_PAIR(1) | A_BOLD);

    attron(A_DIM);
    mvadd_utf8_clipped(4, hero_left + 3, confirmed ? "Busca confirmada" :
              "Consulte um produto pelo nome", hero_width - 6);
    attroff(A_DIM);

    if (input_width > 4) {
        attron(COLOR_PAIR(focus == 5 ? 2 : 1) |
               (focus == 5 ? A_BOLD | A_REVERSE : 0));
        mvaddch(6, input_left, '[');
        mvhline(6, input_left + 1, ACS_HLINE, input_width - 2);
        mvaddch(6, input_left + input_width - 1, ']');
        if (query[0] != '\0') {
            mvadd_utf8_clipped(6, input_left + 2, query, input_width - 4);
        } else if (!editing) {
            mvadd_utf8_clipped(6, input_left + 2, "Digite o nome do produto...",
                      input_width - 4);
        }
        attroff(COLOR_PAIR(focus == 5 ? 2 : 1) |
                (focus == 5 ? A_BOLD | A_REVERSE : 0));
        if (editing) {
            int cursor_x = input_left + 2 + input_string_display_width(query);
            if (cursor_x > input_left + input_width - 2) {
                cursor_x = input_left + input_width - 2;
            }
            move(6, cursor_x);
        }
    }
    mvhline(divider_y, hero_left + 1, ACS_HLINE, hero_width - 2);
}

static void draw_search_results(int screen_height, int hero_left,
                                int hero_width, const ProdutoItem products[],
                                int product_count,
                                const BranchItem branches[], int branch_count,
                                int scroll, int search_error, int focus,
                                int selected_result, const char *heading,
                                const char *empty_message) {
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
    mvadd_utf8_clipped(10, table_x, heading, table_width);
    attroff(COLOR_PAIR(1) | A_BOLD);

    if (search_error) {
        mvadd_utf8_clipped(12, table_x, "Falha ao pesquisar produtos.", table_width);
        return;
    }
    if (product_count == 0) {
        mvadd_utf8_clipped(12, table_x, empty_message, table_width);
        return;
    }

    attron(COLOR_PAIR(1) | A_BOLD);
    mvadd_utf8_clipped(11, table_x, "NOME", name_width);
    mvadd_utf8_clipped(11, quantity_x, "QTD", quantity_width);
    mvadd_utf8_clipped(11, price_x, "PRECO", price_width);
    if (branch_width > 0) mvadd_utf8_clipped(11, branch_x, "FILIAL", branch_width);
    attroff(COLOR_PAIR(1) | A_BOLD);
    mvhline(12, table_x, ACS_HLINE, table_width);

    for (row = 0; row < HOME_PRODUCTS_PER_PAGE && row + scroll < product_count;
         row++) {
        const ProdutoItem *product = &products[row + scroll];
        int selected = focus == 6 && row + scroll == selected_result;
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
        if (selected) {
            attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
            mvhline(13 + row, table_x - 1, ' ', table_width + 2);
        }
        mvadd_utf8_clipped(13 + row, table_x, product->nome, name_width);
        mvadd_utf8_clipped(13 + row, quantity_x, product->quantidade, quantity_width);
        mvadd_utf8_clipped(13 + row, price_x, product->preco, price_width);
        if (branch_width > 0)
            mvadd_utf8_clipped(13 + row, branch_x, branch_label, branch_width);
        if (selected) attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    }

    if (product_count > HOME_PRODUCTS_PER_PAGE) {
        char page_status[48];
        int last = scroll + HOME_PRODUCTS_PER_PAGE < product_count ?
                   scroll + HOME_PRODUCTS_PER_PAGE : product_count;
        snprintf(page_status, sizeof(page_status), "%d-%d de %d",
                 scroll + 1, last, product_count);
        mvadd_utf8_clipped(screen_height - 3, table_x, page_status, table_width);
    }
}

static void draw_product_actions(int hero_left, int hero_width,
                                 const ProdutoItem *product, int focus) {
    static const char *actions[] = {"Venda", "Ver endereco da filial"};
    int index;
    attron(COLOR_PAIR(1) | A_BOLD);
    mvadd_utf8_clipped(10, hero_left + 3, product->nome, hero_width - 6);
    mvadd_utf8_clipped(12, hero_left + 3, "O que deseja fazer?", hero_width - 6);
    attroff(COLOR_PAIR(1) | A_BOLD);
    for (index = 0; index < 2; index++) {
        if (focus == index) attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
        mvadd_utf8_clipped(14 + index * 2, hero_left + 5, actions[index],
                  hero_width - 10);
        if (focus == index) attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    }
}

static void draw_branch_address(int hero_left, int hero_width,
                                const ProdutoItem *product,
                                const BranchItem *branch) {
    char line[512];
    attron(COLOR_PAIR(1) | A_BOLD);
    mvadd_utf8_clipped(10, hero_left + 3, "ENDERECO DA FILIAL", hero_width - 6);
    attroff(COLOR_PAIR(1) | A_BOLD);
    snprintf(line, sizeof(line), "Produto: %s", product->nome);
    mvadd_utf8_clipped(12, hero_left + 3, line, hero_width - 6);
    snprintf(line, sizeof(line), "CEP: %s", branch->cep);
    mvadd_utf8_clipped(14, hero_left + 3, line, hero_width - 6);
    snprintf(line, sizeof(line), "Logradouro: %s", branch->logradouro);
    mvadd_utf8_clipped(15, hero_left + 3, line, hero_width - 6);
    snprintf(line, sizeof(line), "Numero: %s", branch->numero);
    mvadd_utf8_clipped(16, hero_left + 3, line, hero_width - 6);
    snprintf(line, sizeof(line), "Complemento: %s",
             branch->complemento[0] != '\0' ? branch->complemento : "-");
    mvadd_utf8_clipped(17, hero_left + 3, line, hero_width - 6);
    snprintf(line, sizeof(line), "Bairro: %s", branch->bairro);
    mvadd_utf8_clipped(18, hero_left + 3, line, hero_width - 6);
    snprintf(line, sizeof(line), "Cidade/Estado: %s/%s",
             branch->cidade, branch->estado);
    mvadd_utf8_clipped(19, hero_left + 3, line, hero_width - 6);
}

static const BranchItem *find_product_branch(const ProdutoItem *product,
                                             const BranchItem branches[],
                                             int branch_count) {
    int index;
    for (index = 0; index < branch_count; index++) {
        if (strcmp(product->filial, branches[index].id_filial) == 0)
            return &branches[index];
    }
    return NULL;
}

static void draw_sale(int screen_height, int hero_left, int hero_width,
                      const ProdutoItem *product, const char *quantity,
                      const char *crf, int crf_required, int input_focus,
                      const char *status_message, int status_is_error) {
    attron(COLOR_PAIR(1) | A_BOLD);
    mvadd_utf8_clipped(10, hero_left + 3, "REGISTRAR VENDA", hero_width - 6);
    attroff(COLOR_PAIR(1) | A_BOLD);
    mvadd_utf8_clipped(12, hero_left + 3, product->nome, hero_width - 6);
    mvadd_utf8_clipped(14, hero_left + 3, "Quantidade a vender:", hero_width - 6);
    if (input_focus == 0) attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    mvaddch(16, hero_left + 3, '[');
    mvhline(16, hero_left + 4, ' ', 14);
    mvaddch(16, hero_left + 18, ']');
    mvadd_utf8_clipped(16, hero_left + 5, quantity, 12);
    if (input_focus == 0) {
        move(16, hero_left + 5 + (int)strlen(quantity));
        attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    }
    if (crf_required) {
        mvadd_utf8_clipped(18, hero_left + 3, "CRF do farmaceutico da filial:",
                  hero_width - 6);
        if (input_focus == 1) attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
        mvaddch(20, hero_left + 3, '[');
        mvhline(20, hero_left + 4, ' ', 18);
        mvaddch(20, hero_left + 22, ']');
        mvadd_utf8_clipped(20, hero_left + 5, crf, 16);
        if (input_focus == 1) {
            move(20, hero_left + 5 + (int)strlen(crf));
            attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
        }
    }
    if (status_message[0] != '\0') {
        attron(COLOR_PAIR(status_is_error ? 3 : 2) | A_BOLD);
        mvadd_utf8_clipped(screen_height - 3, hero_left + 3, status_message,
                  hero_width - 6);
        attroff(COLOR_PAIR(status_is_error ? 3 : 2) | A_BOLD);
    }
}

static int product_is_in_user_branch(const ProdutoItem *product,
                                     const BranchItem branches[],
                                     int branch_count, const char *user_branch) {
    int index;
    for (index = 0; index < branch_count; index++) {
        if (strcmp(branches[index].id_filial, product->filial) == 0)
            return strcmp(branches[index].cep, user_branch) == 0;
    }
    return 0;
}

static AppScreen home_screen_view(char user_id[7], char *user_profile) {
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
    int selected_result = -1;
    int action_menu_open = 0;
    int branch_info_mode = 0;
    int action_focus = 0;
    int sale_mode = 0;
    int sale_quantity_length = 0;
    int sale_crf_length = 0;
    int sale_input_focus = 0;
    int sale_crf_required = 0;
    int status_is_error = 0;
    ProdutoItem products[MAX_HOME_PRODUCTS] = {{0}};
    ProdutoItem recent_sales[MAX_HOME_PRODUCTS] = {{0}};
    int recent_sales_count;
    BranchItem branches[MAX_HOME_BRANCHES] = {{0}};
    char search_query[SEARCH_BUFFER_SIZE] = "";
    char sale_quantity[11] = "";
    char sale_crf[16] = "";
    char status_message[96] = "";
    ProfileInfo current_user = {0};
    int local_stock_role = strcmp(user_profile, "gerente") == 0 ||
                           strcmp(user_profile, "farmaceutico") == 0 ||
                           strcmp(user_profile, "atendente") == 0;

    if (local_stock_role &&
        get_profile_presentation(user_id, user_profile, &current_user) != 1)
        return APP_SCREEN_HOME;

    branch_count = select_all_branches_presentation(branches,
                                                     MAX_HOME_BRANCHES);
    if (branch_count < 0) branch_count = 0;
    recent_sales_count = select_produtos_vendidos_by_user_id_presentation(
        user_id, recent_sales, MAX_HOME_PRODUCTS);
    if (recent_sales_count < 0) recent_sales_count = 0;

    keypad(stdscr, TRUE);
    while (1) {
        wait_for_minimum_terminal_size();
        getmaxyx(stdscr, screen_height, screen_width);
        sidebar_width = screen_width / 3;
        hero_left = sidebar_width + 1;
        hero_width = screen_width - hero_left - 1;
        divider_y = 1 + (screen_height - 2) / 3;

        ProdutoItem *visible_products = search_confirmed ? products :
                                        recent_sales;
        int visible_product_count = search_confirmed ? product_count :
                                    recent_sales_count;
        erase();
        box(stdscr, 0, 0);
        mvvline(1, sidebar_width, ACS_VLINE, screen_height - 2);
        mvhline(divider_y, hero_left, ACS_HLINE, screen_width - hero_left - 1);

        draw_sidebar(screen_height, sidebar_width,
                     focus == 5 ? -1 : focus, user_profile);
        draw_search_section(hero_left, hero_width, divider_y, focus,
                            editing_search, search_confirmed, search_query);
        if (branch_info_mode && selected_result >= 0 &&
            selected_result < visible_product_count) {
            const BranchItem *branch = find_product_branch(
                &visible_products[selected_result], branches, branch_count);
            if (branch != NULL) {
                draw_branch_address(hero_left, hero_width,
                                    &visible_products[selected_result], branch);
            } else {
                mvadd_utf8_clipped(12, hero_left + 3,
                          "Nao foi possivel localizar o endereco da filial.",
                          hero_width - 6);
            }
        } else if (sale_mode && selected_result >= 0 &&
            selected_result < visible_product_count) {
            draw_sale(screen_height, hero_left, hero_width,
                      &visible_products[selected_result], sale_quantity,
                      sale_crf, sale_crf_required,
                      sale_input_focus, status_message, status_is_error);
        } else if (action_menu_open && selected_result >= 0 &&
                   selected_result < visible_product_count) {
            draw_product_actions(hero_left, hero_width,
                                 &visible_products[selected_result], action_focus);
        } else {
            draw_search_results(
                screen_height, hero_left, hero_width, visible_products,
                visible_product_count, branches, branch_count, result_scroll,
                search_confirmed ? search_error : 0, focus, selected_result,
                search_confirmed ? "RESULTADOS DA PESQUISA" : "ULTIMAS VENDAS",
                search_confirmed ? "Nenhum produto encontrado." :
                                   "Nenhuma venda registrada.");
        }
        if (status_message[0] != '\0' && !sale_mode) {
            attron(COLOR_PAIR(status_is_error ? 3 : 2) | A_BOLD);
            mvadd_utf8_clipped(screen_height - 3, hero_left + 3, status_message,
                      hero_width - 6);
            attroff(COLOR_PAIR(status_is_error ? 3 : 2) | A_BOLD);
        }

        attron(A_DIM);
        if (sale_mode) {
            if (sale_crf_required)
                mvadd_utf8_clipped(screen_height - 2, 2,
                          "Tab/Setas: alternar quantidade e CRF | Enter: confirmar | Esc: voltar",
                          screen_width - 4);
            else
                mvadd_utf8_clipped(screen_height - 2, 2,
                          "Digite a quantidade | Enter: confirmar venda | Esc: voltar",
                          screen_width - 4);
        } else if (branch_info_mode) {
            mvadd_utf8_clipped(screen_height - 2, 2,
                      "Enter/Esc: voltar as opcoes do produto",
                      screen_width - 4);
        } else if (action_menu_open) {
            mvadd_utf8_clipped(screen_height - 2, 2,
                      "Setas: escolher acao | Enter: selecionar | Esc: voltar",
                      screen_width - 4);
        } else if (editing_search) {
            mvadd_utf8_clipped(screen_height - 2, 2,
                      "Digite o produto  |  Enter: confirmar  |  Esc: cancelar",
                      screen_width - 4);
        } else {
            mvadd_utf8_clipped(screen_height - 2, 2,
                      "Setas: mover | PgUp/PgDn: resultados | Enter: selecionar | Esc: sair",
                      screen_width - 4);
        }
        attroff(A_DIM);
        curs_set(editing_search || sale_mode ? 1 : 0);
        refresh();

        character = read_input_character();
        if (character == KEY_RESIZE) continue;
        if (branch_info_mode) {
            if (character == 27 || character == '\n' ||
                character == KEY_ENTER) {
                branch_info_mode = 0;
            }
        } else if (sale_mode) {
            if (character == 27) {
                sale_mode = 0;
                action_menu_open = 1;
                status_message[0] = '\0';
            } else if (sale_crf_required &&
                       (character == '\t' || character == KEY_UP ||
                        character == KEY_DOWN)) {
                sale_input_focus = !sale_input_focus;
            } else if (character == KEY_BACKSPACE || character == 127 ||
                       character == 8) {
                if (sale_input_focus) {
                    if (sale_crf_length > 0)
                        sale_crf[--sale_crf_length] = '\0';
                } else if (sale_quantity_length > 0) {
                    sale_quantity[--sale_quantity_length] = '\0';
                }
            } else if (sale_input_focus && sale_crf_required &&
                       character >= 32 && character <= 126 &&
                       sale_crf_length < (int)sizeof(sale_crf) - 1) {
                sale_crf[sale_crf_length++] = (char)character;
                sale_crf[sale_crf_length] = '\0';
            } else if (!sale_input_focus && character >= '0' &&
                       character <= '9' &&
                       sale_quantity_length <
                           (int)sizeof(sale_quantity) - 1) {
                sale_quantity[sale_quantity_length++] = (char)character;
                sale_quantity[sale_quantity_length] = '\0';
            } else if (character == '\n' || character == KEY_ENTER) {
                if (sale_quantity_length == 0 || atoi(sale_quantity) <= 0) {
                    snprintf(status_message, sizeof(status_message),
                             "Informe uma quantidade maior que zero.");
                    status_is_error = 1;
                } else if (sale_crf_required && sale_crf[0] == '\0') {
                    snprintf(status_message, sizeof(status_message),
                             "Informe o CRF de um farmaceutico da filial.");
                    status_is_error = 1;
                } else if (local_stock_role &&
                           !product_is_in_user_branch(
                               &visible_products[selected_result], branches,
                               branch_count, current_user.branch)) {
                    snprintf(status_message, sizeof(status_message),
                             "Baixa permitida somente na sua filial.");
                    status_is_error = 1;
                } else if (baixa_produto_estoque_presentation(
                               visible_products[selected_result].id_produto,
                               sale_quantity,
                               visible_products[selected_result].filial,
                               user_id, sale_crf) == 0) {
                    int remaining =
                        atoi(visible_products[selected_result].quantidade) -
                        atoi(sale_quantity);
                    snprintf(visible_products[selected_result].quantidade,
                             sizeof(visible_products[selected_result].quantidade),
                             "%d", remaining);
                    recent_sales_count =
                        select_produtos_vendidos_by_user_id_presentation(
                            user_id, recent_sales, MAX_HOME_PRODUCTS);
                    if (recent_sales_count < 0) recent_sales_count = 0;
                    sale_mode = 0;
                    sale_crf[0] = '\0';
                    sale_crf_length = 0;
                    sale_crf_required = 0;
                    sale_input_focus = 0;
                    action_menu_open = 0;
                    focus = 6;
                    if (!search_confirmed) {
                        selected_result = 0;
                        result_scroll = 0;
                    }
                    snprintf(status_message, sizeof(status_message),
                             "Venda registrada. Estoque atualizado.");
                    status_is_error = 0;
                } else {
                    snprintf(status_message, sizeof(status_message), "%s",
                             sale_crf_required ?
                             "CRF nao pertence a filial ou estoque insuficiente." :
                             "Falha na venda. Verifique o estoque disponivel.");
                    status_is_error = 1;
                }
            }
        } else if (action_menu_open) {
            if (character == 27) {
                action_menu_open = 0;
                focus = 6;
                status_message[0] = '\0';
            } else if (character == KEY_UP || character == KEY_DOWN) {
                action_focus = action_focus == 0 ? 1 : 0;
            } else if (character == '\n' || character == KEY_ENTER) {
                if (action_focus == 0) {
                    sale_quantity[0] = '\0';
                    sale_quantity_length = 0;
                    sale_crf[0] = '\0';
                    sale_crf_length = 0;
                    sale_input_focus = 0;
                    status_message[0] = '\0';
                    if (strcmp(user_profile, "atendente") == 0) {
                        int prescription_required =
                            product_requires_prescription_presentation(
                                visible_products[selected_result].id_produto);
                        if (prescription_required < 0) {
                            snprintf(status_message, sizeof(status_message),
                                     "Nao foi possivel consultar a exigencia de receita.");
                            status_is_error = 1;
                        } else {
                            sale_crf_required = prescription_required;
                            sale_mode = 1;
                        }
                    } else {
                        sale_crf_required = 0;
                        sale_mode = 1;
                    }
                } else {
                    branch_info_mode = 1;
                }
            }
        } else if (editing_search) {
            if (character == '\n' || character == KEY_ENTER) {
                editing_search = 0;
                search_confirmed = 1;
                result_scroll = 0;
                search_error = 0;
                status_message[0] = '\0';
                status_is_error = 0;
                product_count = search_query[0] == '\0' ?
                    select_all_products_presentation(products,
                                                     MAX_HOME_PRODUCTS) :
                    select_products_by_name_presentation(
                        products, MAX_HOME_PRODUCTS, search_query);
                if (product_count < 0) {
                    search_error = 1;
                    product_count = 0;
                }
                selected_result = product_count > 0 ? 0 : -1;
            } else if (character == 27) {
                editing_search = 0;
                search_confirmed = 0;
                search_query[0] = '\0';
                search_length = 0;
                product_count = 0;
                result_scroll = 0;
                selected_result = -1;
            } else if (character == KEY_BACKSPACE || character == 127 ||
                       character == 8) {
                input_backspace(search_query);
                search_length = (int)strlen(search_query);
            } else if (input_character_is_printable(character)) {
                size_t length = (size_t)search_length;
                input_character_append(search_query, sizeof(search_query),
                                       &length, character);
                search_length = (int)length;
            }
        } else if (focus == 6 && visible_product_count > 0 &&
                   (character == KEY_UP || character == KEY_DOWN)) {
            if (character == KEY_UP) {
                if (selected_result > 0) {
                    selected_result--;
                    if (selected_result < result_scroll) result_scroll--;
                } else {
                    focus = 5;
                }
            } else if (selected_result + 1 < visible_product_count) {
                selected_result++;
                if (selected_result >= result_scroll + HOME_PRODUCTS_PER_PAGE)
                    result_scroll++;
            }
        } else if (character == KEY_UP) {
                focus = focus == 5 ? 4 : sidebar_move_focus(
                    user_profile, focus, -1);
        } else if (character == KEY_DOWN) {
            if (focus == 5 && visible_product_count > 0) {
                focus = 6;
                if (selected_result < 0) selected_result = result_scroll;
            } else {
                focus = sidebar_move_focus(user_profile, focus, 1);
            }
        } else if (character == KEY_LEFT || character == KEY_RIGHT) {
            focus = focus == 5 || focus == 6 ? 0 : 5;
        } else if (character == KEY_NPAGE &&
                   result_scroll + HOME_PRODUCTS_PER_PAGE <
                       visible_product_count) {
            result_scroll += HOME_PRODUCTS_PER_PAGE;
            if (selected_result < result_scroll ||
                selected_result >= result_scroll + HOME_PRODUCTS_PER_PAGE)
                selected_result = result_scroll;
        } else if (character == KEY_PPAGE && visible_product_count > 0) {
            result_scroll = result_scroll >= HOME_PRODUCTS_PER_PAGE ?
                            result_scroll - HOME_PRODUCTS_PER_PAGE : 0;
            if (selected_result < result_scroll ||
                selected_result >= result_scroll + HOME_PRODUCTS_PER_PAGE)
                selected_result = result_scroll;
        } else if (character == '\n' || character == KEY_ENTER) {
            if (focus == 6 && selected_result >= 0 &&
                selected_result < visible_product_count) {
                action_focus = 0;
                action_menu_open = 1;
                status_message[0] = '\0';
            } else if (focus == 5) {
                editing_search = 1;
                search_confirmed = 0;
                search_length = (int) strlen(search_query);
                result_scroll = 0;
                status_message[0] = '\0';
            } else if (focus == 3) {
                if (sidebar_item_allowed(user_profile, 3))
                    return APP_SCREEN_BRANCHES;
            } else if (focus == 2) {
                if (sidebar_item_allowed(user_profile, 2))
                    return APP_SCREEN_EMPLOYEES;
            } else if (focus == 1) {
                return APP_SCREEN_PRODUCTS;
            } else if (focus == 0) {
                return APP_SCREEN_HOME;
            } else if (focus == 4) {
                return APP_SCREEN_PROFILE;
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
            current_screen = home_screen_view(user_id, user_profile);
        } else if (current_screen == APP_SCREEN_EMPLOYEES) {
            current_screen = sidebar_item_allowed(user_profile, 2) ?
                (AppScreen) employee_screen(user_id, user_profile) : APP_SCREEN_HOME;
        } else if (current_screen == APP_SCREEN_BRANCHES) {
            current_screen = sidebar_item_allowed(user_profile, 3) ?
                (AppScreen) branch_screen(user_id, user_profile) : APP_SCREEN_HOME;
        } else if (current_screen == APP_SCREEN_PRODUCTS) {
            current_screen = (AppScreen) product_screen(user_id, user_profile);
        } else if (current_screen == APP_SCREEN_PROFILE) {
            current_screen = (AppScreen) profile_screen(user_id, user_profile);
        } else {
            current_screen = APP_SCREEN_HOME;
        }
    }
    curs_set(0);
}
