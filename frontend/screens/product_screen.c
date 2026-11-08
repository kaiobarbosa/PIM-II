#include <ncurses.h>
#include <stdio.h>
#include <string.h>

#include "backend/src/presentation/branch_presentation.h"
#include "backend/src/presentation/products_presentation.h"
#include "frontend/components/sidebar.h"
#include "frontend/screens/home_screen.h"
#include "frontend/screens/product_screen.h"

#define MIN_SCREEN_HEIGHT 24
#define MIN_SCREEN_WIDTH 70
#define MAX_PRODUCTS 128
#define MAX_BRANCHES 128
#define MAX_PRODUCT_FIELDS 23
#define PRODUCTS_PER_PAGE 6

typedef enum {
    PRODUCT_GENERAL,
    PRODUCT_DERMOCOSMETIC,
    PRODUCT_MEDICINE,
    PRODUCT_EQUIPMENT,
    PRODUCT_TYPE_COUNT
} ProductType;

typedef enum {
    FIELD_TYPE,
    FIELD_NAME,
    FIELD_BRAND,
    FIELD_DESCRIPTION,
    FIELD_PRICE,
    FIELD_BARCODE,
    FIELD_QUANTITY,
    FIELD_BRANCH,
    FIELD_ACTIVE_INGREDIENT,
    FIELD_PHARMACEUTICAL_FORM,
    FIELD_CONCENTRATION,
    FIELD_STRIPE,
    FIELD_PRESCRIPTION,
    FIELD_CLASSIFICATION,
    FIELD_SKIN_TYPE,
    FIELD_VOLUME,
    FIELD_SUN_PROTECTION,
    FIELD_INDICATION,
    FIELD_DERMOCOSMETIC_ID,
    FIELD_MODEL,
    FIELD_MANUFACTURER,
    FIELD_SPECIFICATIONS,
    FIELD_WARRANTY
} ProductField;

typedef enum {
    SCREEN_MENU,
    SCREEN_REGISTER,
    SCREEN_STOCK,
    SCREEN_ACTIONS,
    SCREEN_SALE
} ProductScreenMode;

typedef struct {
    char id[7];
    char values[MAX_PRODUCT_FIELDS][128];
    ProductType type;
    char type_name[32];
    int branch_index;
} Product;

static const char *product_type_names[PRODUCT_TYPE_COUNT] = {
    "Geral", "Dermocosmeticos", "Medicamentos", "Equipamentos"
};

static const char *field_labels[MAX_PRODUCT_FIELDS] = {
    "Tipo", "Nome", "Marca", "Descricao", "Preco", "Codigo barras",
    "Quantidade", "Filial", "Principio ativo", "Forma farmaceutica",
    "Concentracao", "Tarja", "Receita", "Tipagem", "Tipo de pele",
    "Volume", "Fator protecao", "Indicacao", "ID produto", "Modelo",
    "Fabricante", "Especificacoes", "Garantia"
};

static int get_form_fields(ProductType type, ProductField fields[]) {
    static const ProductField common[] = {
        FIELD_TYPE, FIELD_NAME, FIELD_BRAND, FIELD_DESCRIPTION, FIELD_PRICE,
        FIELD_BARCODE, FIELD_QUANTITY, FIELD_BRANCH
    };
    static const ProductField medicine[] = {
        FIELD_ACTIVE_INGREDIENT, FIELD_PHARMACEUTICAL_FORM,
        FIELD_CONCENTRATION, FIELD_STRIPE, FIELD_PRESCRIPTION,
        FIELD_CLASSIFICATION
    };
    static const ProductField dermocosmetic[] = {
        FIELD_SKIN_TYPE, FIELD_VOLUME, FIELD_SUN_PROTECTION, FIELD_INDICATION
    };
    static const ProductField equipment[] = {
        FIELD_MODEL, FIELD_MANUFACTURER, FIELD_SPECIFICATIONS, FIELD_WARRANTY
    };
    int count = 0;
    int index;

    for (index = 0; index < (int)(sizeof(common) / sizeof(common[0])); index++) {
        fields[count++] = common[index];
    }
    if (type == PRODUCT_MEDICINE) {
        for (index = 0; index < (int)(sizeof(medicine) / sizeof(medicine[0])); index++) {
            fields[count++] = medicine[index];
        }
    } else if (type == PRODUCT_DERMOCOSMETIC) {
        for (index = 0; index < (int)(sizeof(dermocosmetic) /
                                      sizeof(dermocosmetic[0])); index++) {
            fields[count++] = dermocosmetic[index];
        }
    } else if (type == PRODUCT_EQUIPMENT) {
        for (index = 0; index < (int)(sizeof(equipment) / sizeof(equipment[0]));
             index++) {
            fields[count++] = equipment[index];
        }
    }
    return count;
}

static const char *get_product_field_value(const Product *product,
                                           ProductField field,
                                           const BranchItem branches[],
                                           int branch_count) {
    if (field == FIELD_TYPE) return product->type_name;
    if (field == FIELD_BRANCH) {
        if (product->branch_index >= 0 && product->branch_index < branch_count) {
            return branches[product->branch_index].cep;
        }
        return "Selecione uma filial";
    }
    return product->values[field];
}

static void draw_form_field(int y, int hero_left, int hero_width,
                            const Product *product, ProductField field,
                            int selected, int editing,
                            const BranchItem branches[], int branch_count) {
    int value_x = hero_left + 19;
    int value_width = hero_width - 23;
    const char *value = get_product_field_value(product, field, branches,
                                                branch_count);

    mvaddnstr(y, hero_left + 3, field_labels[field], 15);
    if (selected) attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    else attron(COLOR_PAIR(1));
    if (value_width > 2) {
        mvaddch(y, value_x, '[');
        mvhline(y, value_x + 1, ' ', value_width - 2);
        mvaddch(y, value_x + value_width - 1, ']');
        mvaddnstr(y, value_x + 2, value, value_width - 4);
        if (selected && editing) {
            int cursor_x = value_x + 2 + (int)strlen(value);
            if (cursor_x > value_x + value_width - 2) {
                cursor_x = value_x + value_width - 2;
            }
            move(y, cursor_x);
        }
    }
    if (selected) attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    else attroff(COLOR_PAIR(1));
}

static void draw_menu(int screen_height, int hero_left, int hero_width,
                      int divider_y, int focus,
                      const Product products[], int product_count,
                      int products_scroll, int selected_product) {
    int table_x = hero_left + 3;
    int table_width = hero_width - 6;
    int name_width = table_width * 42 / 100;
    int quantity_width = 5;
    int price_width = 9;
    int branch_width = table_width - name_width - quantity_width - price_width - 3;
    int name_x = table_x;
    int quantity_x = name_x + name_width + 1;
    int price_x = quantity_x + quantity_width + 1;
    int branch_x = price_x + price_width + 1;
    int visible_count = PRODUCTS_PER_PAGE;
    int row;

    mvhline(divider_y, hero_left, ACS_HLINE, hero_width);
    if (focus == 0) attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    mvaddnstr(6, hero_left + 4, "+ Cadastrar novo produto", hero_width - 8);
    if (focus == 0) attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    if (focus == 1) attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    mvaddnstr(divider_y - 3, hero_left + 4, "> Pesquisar",
              hero_width - 8);
    if (focus == 1) attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);

    attron(COLOR_PAIR(1) | A_BOLD);
    mvaddnstr(divider_y + 1, table_x, "PRODUTOS", table_width);
    mvaddnstr(divider_y + 2, name_x, "NOME", name_width);
    mvaddnstr(divider_y + 2, quantity_x, "QTD", quantity_width);
    mvaddnstr(divider_y + 2, price_x, "PRECO", price_width);
    if (branch_width > 0)
        mvaddnstr(divider_y + 2, branch_x, "FILIAL", branch_width);
    attroff(COLOR_PAIR(1) | A_BOLD);
    mvhline(divider_y + 3, table_x, ACS_HLINE, table_width);

    for (row = 0; row < visible_count && row + products_scroll < product_count;
         row++) {
        int y = divider_y + 4 + row;
        const Product *product = &products[row + products_scroll];
        int selected = focus == 2 && row + products_scroll == selected_product;
        if (selected) {
            attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
            mvhline(y, table_x - 1, ' ', table_width + 2);
        }
        mvaddnstr(y, name_x, product->values[FIELD_NAME], name_width);
        mvaddnstr(y, quantity_x, product->values[FIELD_QUANTITY],
                  quantity_width);
        mvaddnstr(y, price_x, product->values[FIELD_PRICE], price_width);
        if (branch_width > 0)
            mvaddnstr(y, branch_x, product->values[FIELD_BRANCH], branch_width);
        if (selected) attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    }
    if (product_count == 0)
        mvaddnstr(divider_y + 4, table_x, "Nenhum produto encontrado.",
                  table_width);
    else if (product_count > PRODUCTS_PER_PAGE) {
        char page_status[48];
        snprintf(page_status, sizeof(page_status), "%d-%d de %d",
                 products_scroll + 1,
                 products_scroll + PRODUCTS_PER_PAGE < product_count ?
                     products_scroll + PRODUCTS_PER_PAGE : product_count,
                 product_count);
        mvaddnstr(screen_height - 3, table_x, page_status, table_width);
    }
}

static void draw_register(int screen_height, int hero_left, int hero_width,
                          int divider_y, const Product *product,
                          const ProductField fields[], int field_count,
                          int focus, int editing,
                          const BranchItem branches[], int branch_count,
                          const char *status_message, int status_is_error) {
    int index;
    int lower_y = divider_y + 1;
    int button_y = screen_height - 4;

    attron(COLOR_PAIR(1) | A_BOLD);
    mvaddnstr(2, hero_left + 3, "CADASTRAR PRODUTO", hero_width - 6);
    mvaddnstr(divider_y + 1, hero_left + 3,
              product->type == PRODUCT_GENERAL ? "DADOS ADICIONAIS" :
              "DADOS ESPECIFICOS", hero_width - 6);
    attroff(COLOR_PAIR(1) | A_BOLD);
    for (index = 0; index < field_count; index++) {
        int y;
        if (index < 8) y = 3 + index;
        else y = lower_y + 1 + index - 8;
        if (y >= button_y - 1) continue;
        draw_form_field(y, hero_left, hero_width, product, fields[index],
                        focus == index, editing && focus == index,
                        branches, branch_count);
    }

    if (focus == field_count) attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    mvaddnstr(button_y, hero_left + 4, "[ CANCELAR ]", 14);
    if (focus == field_count) attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    if (focus == field_count + 1) attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    mvaddnstr(button_y, hero_left + 23, "[ CADASTRAR ]", 16);
    if (focus == field_count + 1) attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);

    if (status_message[0] != '\0') {
        attron(COLOR_PAIR(status_is_error ? 3 : 2) | A_BOLD);
        mvaddnstr(screen_height - 3, hero_left + 3, status_message,
                  hero_width - 6);
        attroff(COLOR_PAIR(status_is_error ? 3 : 2) | A_BOLD);
    }
}

static int build_stock_results(const Product products[], int product_count,
                               const char *search, int branch_filter,
                               int indexes[]) {
    int count = 0;
    int index;
    for (index = 0; index < product_count; index++) {
        int branch_matches = branch_filter < 0 ||
                             products[index].branch_index == branch_filter;
        int name_matches = search[0] == '\0' ||
                           strstr(products[index].values[FIELD_NAME], search) != NULL;
        if (branch_matches && name_matches) indexes[count++] = index;
    }
    return count;
}

static void draw_stock(int screen_height, int hero_left, int hero_width,
                       const Product products[], int indexes[], int result_count,
                       const char *search, int branch_filter,
                       const BranchItem branches[], int branch_count,
                       int focus, int scroll, int editing) {
    int row;
    int visible_count = PRODUCTS_PER_PAGE;
    int filter_x = hero_left + 16;
    int filter_width = hero_width - 21;
    int table_x = hero_left + 3;
    int table_width = hero_width - 6;
    int quantity_width = 5;
    int price_width = 10;
    int branch_width = 10;
    int name_width = table_width - quantity_width - price_width -
                     branch_width - 3;
    int name_x = table_x;
    int quantity_x = name_x + name_width + 1;
    int price_x = quantity_x + quantity_width + 1;
    int branch_x = price_x + price_width + 1;
    char branch_name[64];

    (void)screen_height;

    attron(COLOR_PAIR(1) | A_BOLD);
    mvaddnstr(2, hero_left + 3, "ESTOQUE DE PRODUTOS", hero_width - 6);
    attroff(COLOR_PAIR(1) | A_BOLD);
    mvaddnstr(4, hero_left + 3, "Produto", 12);
    if (focus == 0) attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    mvaddch(4, filter_x, '[');
    mvhline(4, filter_x + 1, ' ', filter_width - 2);
    mvaddch(4, filter_x + filter_width - 1, ']');
    mvaddnstr(4, filter_x + 2, search, filter_width - 4);
    if (editing) {
        int cursor_x = filter_x + 2 + (int)strlen(search);
        if (cursor_x > filter_x + filter_width - 2)
            cursor_x = filter_x + filter_width - 2;
        move(4, cursor_x);
    }
    if (focus == 0) attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);

    mvaddnstr(6, hero_left + 3, "Filial", 12);
    if (branch_filter < 0) snprintf(branch_name, sizeof(branch_name), "Geral");
    else if (branch_filter < branch_count)
        snprintf(branch_name, sizeof(branch_name), "%s",
                 branches[branch_filter].cep);
    else snprintf(branch_name, sizeof(branch_name), "Geral");
    if (focus == 1) attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    mvaddnstr(6, filter_x, "[", 1);
    mvhline(6, filter_x + 1, ' ', filter_width - 2);
    mvaddch(6, filter_x + filter_width - 1, ']');
    mvaddnstr(6, filter_x + 2, branch_name, filter_width - 4);
    if (focus == 1) attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);

    if (focus == 2) attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    mvaddnstr(8, hero_left + 3, "[ PESQUISAR ]", 16);
    if (focus == 2) attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);

    attron(COLOR_PAIR(1) | A_BOLD);
    mvaddnstr(11, name_x, "NOME", name_width);
    mvaddnstr(11, quantity_x, "QTD", quantity_width);
    mvaddnstr(11, price_x, "PRECO", price_width);
    mvaddnstr(11, branch_x, "FILIAL", branch_width);
    attroff(COLOR_PAIR(1) | A_BOLD);
    mvhline(12, hero_left + 3, ACS_HLINE, hero_width - 6);
    for (row = 0; row < visible_count && row + scroll < result_count; row++) {
        int product_index = indexes[row + scroll];
        int y = 13 + row;
        const Product *product = &products[product_index];
        if (focus == row + scroll + 3) {
            attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
            mvhline(y, hero_left + 2, ' ', hero_width - 4);
        }
        mvaddnstr(y, name_x, product->values[FIELD_NAME], name_width);
        mvaddnstr(y, quantity_x, product->values[FIELD_QUANTITY],
                  quantity_width);
        mvaddnstr(y, price_x, product->values[FIELD_PRICE], price_width);
        mvaddnstr(y, branch_x, product->values[FIELD_BRANCH], branch_width);
        if (focus == row + scroll + 3)
            attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    }
}

static void draw_product_actions(int hero_left, int hero_width,
                                 const Product *product, int focus) {
    static const char *actions[] = {"Venda", "Editar", "Excluir"};
    int index;

    attron(COLOR_PAIR(1) | A_BOLD);
    mvaddnstr(3, hero_left + 3, product->values[FIELD_NAME], hero_width - 6);
    mvaddnstr(5, hero_left + 3, "O que deseja fazer?", hero_width - 6);
    attroff(COLOR_PAIR(1) | A_BOLD);
    for (index = 0; index < 3; index++) {
        if (focus == index) attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
        mvaddnstr(7 + index * 2, hero_left + 5, actions[index],
                  hero_width - 10);
        if (focus == index) attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    }
}

static void draw_sale(int screen_height, int hero_left, int hero_width,
                      const Product *product, const char *quantity,
                      const char *status_message, int status_is_error) {
    attron(COLOR_PAIR(1) | A_BOLD);
    mvaddnstr(3, hero_left + 3, "REGISTRAR VENDA", hero_width - 6);
    attroff(COLOR_PAIR(1) | A_BOLD);
    mvaddnstr(5, hero_left + 3, product->values[FIELD_NAME], hero_width - 6);
    mvaddnstr(7, hero_left + 3, "Quantidade a vender:", hero_width - 6);
    attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    mvaddch(9, hero_left + 3, '[');
    mvhline(9, hero_left + 4, ' ', 14);
    mvaddch(9, hero_left + 18, ']');
    mvaddnstr(9, hero_left + 5, quantity, 12);
    move(9, hero_left + 5 + (int)strlen(quantity));
    attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    if (status_message[0] != '\0') {
        attron(COLOR_PAIR(status_is_error ? 3 : 2) | A_BOLD);
        mvaddnstr(screen_height - 3, hero_left + 3, status_message,
                  hero_width - 6);
        attroff(COLOR_PAIR(status_is_error ? 3 : 2) | A_BOLD);
    }
}

static void clear_product(Product *product) {
    memset(product, 0, sizeof(*product));
    product->branch_index = -1;
}

static void set_product_type(Product *product, ProductType type) {
    product->type = type;
    snprintf(product->type_name, sizeof(product->type_name), "%s",
             product_type_names[type]);
}

static int is_valid_prescription_value(const char *value) {
    return strcmp(value, "sim") == 0 || strcmp(value, "nao") == 0 ||
           strcmp(value, "não") == 0 || strcmp(value, "1") == 0 ||
           strcmp(value, "0") == 0;
}

static void map_product_items(Product products[], const ProdutoItem items[],
                              int product_count, const BranchItem branches[],
                              int branch_count) {
    int item_index;

    for (item_index = 0; item_index < product_count; item_index++) {
        int branch_index;
        snprintf(products[item_index].id, sizeof(products[item_index].id),
                 "%s", items[item_index].id_produto);
        snprintf(products[item_index].values[FIELD_NAME],
                 sizeof(products[item_index].values[FIELD_NAME]), "%s",
                 items[item_index].nome);
        snprintf(products[item_index].values[FIELD_QUANTITY],
                 sizeof(products[item_index].values[FIELD_QUANTITY]), "%s",
                 items[item_index].quantidade);
        snprintf(products[item_index].values[FIELD_PRICE],
                 sizeof(products[item_index].values[FIELD_PRICE]), "%s",
                 items[item_index].preco);
        products[item_index].branch_index = -1;
        for (branch_index = 0; branch_index < branch_count; branch_index++) {
            if (strcmp(items[item_index].filial,
                       branches[branch_index].id_filial) == 0) {
                products[item_index].branch_index = branch_index;
                break;
            }
        }
        snprintf(products[item_index].values[FIELD_BRANCH],
                 sizeof(products[item_index].values[FIELD_BRANCH]), "%s",
                 products[item_index].branch_index >= 0 ?
                     branches[products[item_index].branch_index].cep :
                     items[item_index].filial);
    }
}

static int refresh_product_list(Product products[], ProdutoItem product_items[],
                                const BranchItem branches[], int branch_count) {
    int product_count = select_all_products_presentation(product_items,
                                                          MAX_PRODUCTS);

    if (product_count < 0) return -1;

    map_product_items(products, product_items, product_count, branches,
                      branch_count);
    return product_count;
}

static int search_products(ProdutoItem products[], const char *name,
                           int branch_filter, const BranchItem branches[]) {
    if (name[0] != '\0' && branch_filter >= 0) {
        return select_products_by_filial_and_name_presentation(
            products, MAX_PRODUCTS, branches[branch_filter].id_filial, name);
    }
    if (name[0] != '\0') {
        return select_products_by_name_presentation(products, MAX_PRODUCTS,
                                                    name);
    }
    if (branch_filter >= 0) {
        return select_products_by_filial_presentation(
            products, MAX_PRODUCTS, branches[branch_filter].id_filial);
    }
    return select_all_products_presentation(products, MAX_PRODUCTS);
}

int product_screen(char user_id[7], char *user_profile) {
    Product products[MAX_PRODUCTS] = {{0}};
    ProdutoItem product_items[MAX_PRODUCTS] = {{0}};
    BranchItem branches[MAX_BRANCHES] = {{0}};
    int product_count = 0;
    int branch_count;
    int screen_height;
    int screen_width;
    int sidebar_width;
    int hero_left;
    int hero_width;
    int divider_y;
    int sidebar_focus = 1;
    int is_in_sidebar = 1;
    int character;
    int menu_focus = 0;
    ProductScreenMode mode = SCREEN_MENU;
    Product form_product;
    ProductField form_fields[MAX_PRODUCT_FIELDS];
    int form_field_count = 0;
    int form_focus = 0;
    int form_editing = 0;
    char status_message[96] = "";
    int status_is_error = 0;
    char stock_search[128] = "";
    char applied_search[128] = "";
    int stock_search_length = 0;
    int branch_filter = -1;
    int applied_branch_filter = -1;
    int stock_focus = 0;
    int stock_scroll = 0;
    int products_scroll = 0;
    int selected_product = -1;
    int action_focus = 0;
    char sale_quantity[11] = "";
    int sale_quantity_length = 0;

    (void)user_profile;
    clear_product(&form_product);
    branch_count = select_all_branches_presentation(branches, MAX_BRANCHES);
    if (branch_count < 0) branch_count = 0;
    product_count = refresh_product_list(products, product_items, branches,
                                         branch_count);
    if (product_count < 0) product_count = 0;
    getmaxyx(stdscr, screen_height, screen_width);
    if (screen_height < MIN_SCREEN_HEIGHT || screen_width < MIN_SCREEN_WIDTH) {
        draw_resize_message(screen_height, screen_width);
        return APP_SCREEN_HOME;
    }
    sidebar_width = screen_width / 3;
    hero_left = sidebar_width + 1;
    hero_width = screen_width - hero_left - 1;
    divider_y = screen_height / 2 - 1;
    keypad(stdscr, TRUE);

    while (1) {
        int result_indexes[MAX_PRODUCTS];
        int result_count = build_stock_results(products, product_count,
                                               applied_search,
                                               applied_branch_filter,
                                               result_indexes);
        erase();
        box(stdscr, 0, 0);
        mvvline(1, sidebar_width, ACS_VLINE, screen_height - 2);
        draw_sidebar(screen_height, sidebar_width,
                     is_in_sidebar ? sidebar_focus : -1);
        if (mode == SCREEN_MENU) {
            draw_menu(screen_height, hero_left, hero_width, divider_y,
                      menu_focus, products, product_count, products_scroll,
                      selected_product);
            if (status_message[0] != '\0') {
                attron(COLOR_PAIR(status_is_error ? 3 : 2) | A_BOLD);
                mvaddnstr(screen_height - 3, hero_left + 3, status_message,
                          hero_width - 6);
                attroff(COLOR_PAIR(status_is_error ? 3 : 2) | A_BOLD);
            }
        } else if (mode == SCREEN_REGISTER) {
            draw_register(screen_height, hero_left, hero_width, divider_y,
                          &form_product, form_fields, form_field_count,
                          form_focus, form_editing, branches, branch_count,
                          status_message, status_is_error);
            mvhline(divider_y, hero_left, ACS_HLINE, hero_width);
        } else if (mode == SCREEN_ACTIONS) {
            draw_product_actions(hero_left, hero_width,
                                 &products[selected_product], action_focus);
        } else if (mode == SCREEN_SALE) {
            draw_sale(screen_height, hero_left, hero_width,
                      &products[selected_product], sale_quantity,
                      status_message, status_is_error);
        } else {
            draw_stock(screen_height, hero_left, hero_width, products,
                       result_indexes, result_count, stock_search,
                       branch_filter, branches, branch_count, stock_focus,
                       stock_scroll, stock_focus == 0 && form_editing);
        }
        attron(A_DIM);
        mvaddnstr(screen_height - 2, 2,
                  "Setas: navegar | Enter: selecionar | PgUp/PgDn: produtos | Esc: voltar",
                  screen_width - 4);
        attroff(A_DIM);
        curs_set(form_editing || mode == SCREEN_SALE ? 1 : 0);
        refresh();

        character = getch();
        if (is_in_sidebar) {
            if (character == 27) return APP_SCREEN_HOME;
            if (character == KEY_UP) {
                sidebar_focus = sidebar_focus == 0 ? 4 : sidebar_focus - 1;
            } else if (character == KEY_DOWN) {
                sidebar_focus = (sidebar_focus + 1) % 5;
            } else if (character == KEY_RIGHT) {
                is_in_sidebar = 0;
            } else if (character == '\n' || character == KEY_ENTER) {
                if (sidebar_focus == 0) return APP_SCREEN_HOME;
                if (sidebar_focus == 1) is_in_sidebar = 0;
                if (sidebar_focus == 2) return APP_SCREEN_EMPLOYEES;
                if (sidebar_focus == 3) return APP_SCREEN_BRANCHES;
                if (sidebar_focus == 4) return APP_SCREEN_PROFILE;
            }
            continue;
        }

        if (mode == SCREEN_MENU) {
            if (character == 27) {
                is_in_sidebar = 1;
            } else if (character == KEY_LEFT) {
                if (menu_focus == 0) is_in_sidebar = 1;
                else menu_focus--;
            } else if (character == KEY_RIGHT) {
                if (menu_focus < 1) menu_focus++;
                else if (product_count > 0) {
                    menu_focus = 2;
                    if (selected_product < 0) selected_product = products_scroll;
                }
            } else if (character == KEY_UP) {
                if (menu_focus == 2 && selected_product > 0) {
                    selected_product--;
                    if (selected_product < products_scroll) products_scroll--;
                } else if (menu_focus == 2) {
                    menu_focus = 1;
                } else if (menu_focus == 1) {
                    menu_focus = 0;
                }
            } else if (character == KEY_DOWN) {
                if (menu_focus < 2) {
                    if (menu_focus == 1 && product_count > 0) {
                        menu_focus = 2;
                        if (selected_product < 0)
                            selected_product = products_scroll;
                    } else if (menu_focus == 0) {
                        menu_focus++;
                    }
                } else if (selected_product + 1 < product_count) {
                    selected_product++;
                    if (selected_product >= products_scroll + PRODUCTS_PER_PAGE)
                        products_scroll++;
                }
            } else if (character == KEY_NPAGE) {
                if (products_scroll + PRODUCTS_PER_PAGE < product_count) {
                    products_scroll += PRODUCTS_PER_PAGE;
                    if (selected_product < products_scroll ||
                        selected_product >= products_scroll + PRODUCTS_PER_PAGE)
                        selected_product = products_scroll;
                }
            } else if (character == KEY_PPAGE) {
                products_scroll = products_scroll >= PRODUCTS_PER_PAGE ?
                                  products_scroll - PRODUCTS_PER_PAGE : 0;
                if (selected_product < products_scroll ||
                    selected_product >= products_scroll + PRODUCTS_PER_PAGE)
                    selected_product = products_scroll;
            } else if (character == '\n' || character == KEY_ENTER) {
                if (menu_focus == 0) {
                    clear_product(&form_product);
                    set_product_type(&form_product, PRODUCT_GENERAL);
                    form_field_count = get_form_fields(form_product.type,
                                                       form_fields);
                    form_focus = 0;
                    form_editing = 0;
                    status_message[0] = '\0';
                    mode = SCREEN_REGISTER;
                } else if (menu_focus == 1) {
                    mode = SCREEN_STOCK;
                    stock_focus = 0;
                    stock_scroll = 0;
                } else if (menu_focus == 2 && selected_product >= 0 &&
                           selected_product < product_count) {
                    action_focus = 0;
                    status_message[0] = '\0';
                    mode = SCREEN_ACTIONS;
                }
            }
        } else if (mode == SCREEN_REGISTER) {
            if (form_editing) {
                char *value = form_product.values[form_fields[form_focus]];
                if (character == '\n' || character == KEY_ENTER ||
                    character == 27) {
                    form_editing = 0;
                } else if (character == KEY_BACKSPACE || character == 127 ||
                           character == 8) {
                    int length = (int)strlen(value);
                    if (length > 0) value[length - 1] = '\0';
                } else if (character >= 32 && character <= 255 &&
                           strlen(value) < sizeof(form_product.values[0]) - 1) {
                    int length = (int)strlen(value);
                    value[length] = (char)character;
                    value[length + 1] = '\0';
                }
            } else if (character == KEY_LEFT && form_focus == 0) {
                is_in_sidebar = 1;
            } else if (character == KEY_UP) {
                form_focus = form_focus == 0 ? form_field_count + 1 :
                             form_focus - 1;
            } else if (character == KEY_DOWN) {
                form_focus = (form_focus + 1) % (form_field_count + 2);
            } else if ((character == KEY_LEFT || character == KEY_RIGHT) &&
                       form_focus < form_field_count) {
                ProductField field = form_fields[form_focus];
                if (field == FIELD_TYPE) {
                    int direction = character == KEY_RIGHT ? 1 : -1;
                    set_product_type(&form_product, (ProductType)
                        ((form_product.type + direction + PRODUCT_TYPE_COUNT) %
                         PRODUCT_TYPE_COUNT));
                    form_field_count = get_form_fields(form_product.type,
                                                       form_fields);
                    if (form_focus >= form_field_count) form_focus = 0;
                } else if (field == FIELD_BRANCH && branch_count > 0) {
                    int direction = character == KEY_RIGHT ? 1 : -1;
                    if (form_product.branch_index < 0)
                        form_product.branch_index = direction > 0 ? 0 :
                                                    branch_count - 1;
                    else form_product.branch_index =
                        (form_product.branch_index + direction + branch_count) %
                        branch_count;
                }
            } else if (character == '\n' || character == KEY_ENTER) {
                if (form_focus < form_field_count) {
                    ProductField field = form_fields[form_focus];
                    if (field == FIELD_TYPE) {
                        set_product_type(&form_product, (ProductType)
                            ((form_product.type + 1) % PRODUCT_TYPE_COUNT));
                        form_field_count = get_form_fields(form_product.type,
                                                           form_fields);
                    } else if (field == FIELD_BRANCH) {
                        if (branch_count > 0) {
                            form_product.branch_index =
                                (form_product.branch_index + 1) % branch_count;
                        } else {
                            snprintf(status_message, sizeof(status_message),
                                     "Cadastre uma filial antes do produto.");
                            status_is_error = 1;
                        }
                    } else {
                        form_editing = 1;
                    }
                } else if (form_focus == form_field_count) {
                    mode = SCREEN_MENU;
                } else if (form_product.values[FIELD_NAME][0] == '\0' ||
                           form_product.values[FIELD_BRAND][0] == '\0' ||
                           form_product.values[FIELD_DESCRIPTION][0] == '\0' ||
                           form_product.values[FIELD_PRICE][0] == '\0' ||
                           form_product.values[FIELD_BARCODE][0] == '\0' ||
                           form_product.values[FIELD_QUANTITY][0] == '\0' ||
                           form_product.branch_index < 0 ||
                           form_product.branch_index >= branch_count) {
                    snprintf(status_message, sizeof(status_message),
                             "Preencha todos os campos gerais e selecione a filial.");
                    status_is_error = 1;
                } else if (form_product.type == PRODUCT_MEDICINE &&
                           (form_product.values[FIELD_ACTIVE_INGREDIENT][0] == '\0' ||
                            form_product.values[FIELD_PHARMACEUTICAL_FORM][0] == '\0' ||
                            form_product.values[FIELD_CONCENTRATION][0] == '\0' ||
                            form_product.values[FIELD_STRIPE][0] == '\0' ||
                            form_product.values[FIELD_PRESCRIPTION][0] == '\0' ||
                            form_product.values[FIELD_CLASSIFICATION][0] == '\0')) {
                    snprintf(status_message, sizeof(status_message),
                             "Preencha todos os campos do medicamento.");
                    status_is_error = 1;
                } else if (form_product.type == PRODUCT_MEDICINE &&
                           !is_valid_prescription_value(
                               form_product.values[FIELD_PRESCRIPTION])) {
                    snprintf(status_message, sizeof(status_message),
                             "Receita deve ser sim/nao ou 1/0.");
                    status_is_error = 1;
                } else if (form_product.type == PRODUCT_EQUIPMENT &&
                           (form_product.values[FIELD_MODEL][0] == '\0' ||
                            form_product.values[FIELD_MANUFACTURER][0] == '\0' ||
                            form_product.values[FIELD_SPECIFICATIONS][0] == '\0' ||
                            form_product.values[FIELD_WARRANTY][0] == '\0')) {
                    snprintf(status_message, sizeof(status_message),
                             "Preencha todos os campos do equipamento.");
                    status_is_error = 1;
                } else if (form_product.type == PRODUCT_DERMOCOSMETIC &&
                           (form_product.values[FIELD_SKIN_TYPE][0] == '\0' ||
                            form_product.values[FIELD_VOLUME][0] == '\0' ||
                            form_product.values[FIELD_SUN_PROTECTION][0] == '\0' ||
                            form_product.values[FIELD_INDICATION][0] == '\0')) {
                    snprintf(status_message, sizeof(status_message),
                             "Preencha todos os campos do dermocosmetico.");
                    status_is_error = 1;
                } else if (product_count < MAX_PRODUCTS) {
                    int registration_result;
                    if (form_product.type == PRODUCT_MEDICINE) {
                        registration_result = product_med(
                            form_product.values[FIELD_NAME],
                            form_product.values[FIELD_BRAND],
                            form_product.values[FIELD_DESCRIPTION],
                            form_product.values[FIELD_PRICE],
                            form_product.values[FIELD_BARCODE],
                            form_product.values[FIELD_QUANTITY],
                            branches[form_product.branch_index].id_filial,
                            form_product.id,
                            form_product.values[FIELD_ACTIVE_INGREDIENT],
                            form_product.values[FIELD_PHARMACEUTICAL_FORM],
                            form_product.values[FIELD_CONCENTRATION],
                            form_product.values[FIELD_STRIPE],
                            form_product.values[FIELD_PRESCRIPTION],
                            form_product.values[FIELD_CLASSIFICATION],
                            user_id);
                    } else if (form_product.type == PRODUCT_EQUIPMENT) {
                        registration_result = product_equip(
                            form_product.values[FIELD_NAME],
                            form_product.values[FIELD_BRAND],
                            form_product.values[FIELD_DESCRIPTION],
                            form_product.values[FIELD_PRICE],
                            form_product.values[FIELD_BARCODE],
                            form_product.values[FIELD_QUANTITY],
                            branches[form_product.branch_index].id_filial,
                            form_product.id,
                            form_product.values[FIELD_MODEL],
                            form_product.values[FIELD_MANUFACTURER],
                            form_product.values[FIELD_SPECIFICATIONS],
                            form_product.values[FIELD_WARRANTY], user_id);
                    } else if (form_product.type == PRODUCT_DERMOCOSMETIC) {
                        registration_result = product_dermo(
                            form_product.values[FIELD_NAME],
                            form_product.values[FIELD_BRAND],
                            form_product.values[FIELD_DESCRIPTION],
                            form_product.values[FIELD_PRICE],
                            form_product.values[FIELD_BARCODE],
                            form_product.values[FIELD_QUANTITY],
                            branches[form_product.branch_index].id_filial,
                            form_product.id,
                            form_product.values[FIELD_SKIN_TYPE],
                            form_product.values[FIELD_VOLUME],
                            form_product.values[FIELD_SUN_PROTECTION],
                            form_product.values[FIELD_INDICATION], user_id);
                    } else {
                        registration_result = products_presentation(
                            form_product.values[FIELD_NAME],
                            form_product.values[FIELD_BRAND],
                            form_product.values[FIELD_DESCRIPTION],
                            form_product.values[FIELD_PRICE],
                            form_product.values[FIELD_BARCODE],
                            form_product.values[FIELD_QUANTITY],
                            branches[form_product.branch_index].id_filial,
                            form_product.id, user_id);
                    }
                    if (registration_result == 0) {
                        snprintf(form_product.values[FIELD_BRANCH],
                                 sizeof(form_product.values[FIELD_BRANCH]),
                                 "%s", branches[form_product.branch_index].cep);
                        products[product_count++] = form_product;
                        {
                            int refreshed_count = refresh_product_list(
                                products, product_items, branches, branch_count);
                            if (refreshed_count >= 0)
                                product_count = refreshed_count;
                        }
                        snprintf(status_message, sizeof(status_message),
                                 "Produto cadastrado e lista atualizada.");
                        status_is_error = 0;
                        products_scroll = product_count > PRODUCTS_PER_PAGE ?
                                          product_count - PRODUCTS_PER_PAGE : 0;
                        selected_product = product_count - 1;
                        mode = SCREEN_MENU;
                        menu_focus = 0;
                    } else {
                        snprintf(status_message, sizeof(status_message),
                                 "Falha no cadastro. Confira os campos e tente novamente.");
                        status_is_error = 1;
                    }
                } else {
                    snprintf(status_message, sizeof(status_message),
                             "Limite de produtos desta sessao atingido.");
                    status_is_error = 1;
                }
            } else if (character == 27) {
                form_editing = 0;
                mode = SCREEN_MENU;
            }
        } else if (mode == SCREEN_ACTIONS) {
            if (character == 27 || character == KEY_LEFT) {
                mode = SCREEN_MENU;
                menu_focus = 2;
            } else if (character == KEY_UP || character == KEY_DOWN) {
                action_focus = (action_focus +
                    (character == KEY_DOWN ? 1 : 2)) % 3;
            } else if (character == '\n' || character == KEY_ENTER) {
                if (action_focus == 0) {
                    sale_quantity[0] = '\0';
                    sale_quantity_length = 0;
                    status_message[0] = '\0';
                    mode = SCREEN_SALE;
                } else {
                    snprintf(status_message, sizeof(status_message),
                             "%s ainda nao esta disponivel.",
                             action_focus == 1 ? "Edicao" : "Exclusao");
                    status_is_error = 1;
                    mode = SCREEN_MENU;
                    menu_focus = 2;
                }
            }
        } else if (mode == SCREEN_SALE) {
            if (character == 27) {
                mode = SCREEN_ACTIONS;
                status_message[0] = '\0';
            } else if (character == KEY_BACKSPACE || character == 127 ||
                       character == 8) {
                if (sale_quantity_length > 0)
                    sale_quantity[--sale_quantity_length] = '\0';
            } else if (character >= '0' && character <= '9' &&
                       sale_quantity_length <
                           (int)sizeof(sale_quantity) - 1) {
                sale_quantity[sale_quantity_length++] = (char)character;
                sale_quantity[sale_quantity_length] = '\0';
            } else if (character == '\n' || character == KEY_ENTER) {
                if (sale_quantity_length == 0 || atoi(sale_quantity) <= 0) {
                    snprintf(status_message, sizeof(status_message),
                             "Informe uma quantidade maior que zero.");
                    status_is_error = 1;
                } else if (products[selected_product].branch_index < 0 ||
                           products[selected_product].branch_index >=
                               branch_count) {
                    snprintf(status_message, sizeof(status_message),
                             "Nao foi possivel identificar a filial do produto.");
                    status_is_error = 1;
                } else if (baixa_produto_estoque_presentation(
                               products[selected_product].id, sale_quantity,
                               branches[products[selected_product].branch_index]
                                   .id_filial, user_id) == 0) {
                    int remaining =
                        atoi(products[selected_product].values[FIELD_QUANTITY]) -
                        atoi(sale_quantity);
                    snprintf(products[selected_product].values[FIELD_QUANTITY],
                             sizeof(products[selected_product].values[FIELD_QUANTITY]),
                             "%d", remaining);
                    snprintf(status_message, sizeof(status_message),
                             "Venda registrada. Estoque atualizado.");
                    status_is_error = 0;
                    mode = SCREEN_MENU;
                    menu_focus = 2;
                } else {
                    snprintf(status_message, sizeof(status_message),
                             "Falha na venda. Verifique o estoque disponivel.");
                    status_is_error = 1;
                }
            }
        } else {
            if (form_editing) {
                if (character == '\n' || character == KEY_ENTER ||
                    character == 27) {
                    form_editing = 0;
                } else if (character == KEY_BACKSPACE || character == 127 ||
                           character == 8) {
                    if (stock_search_length > 0)
                        stock_search[--stock_search_length] = '\0';
                } else if (character >= 32 && character <= 255 &&
                           stock_search_length < (int)sizeof(stock_search) - 1) {
                    stock_search[stock_search_length++] = (char)character;
                    stock_search[stock_search_length] = '\0';
                }
            } else if (character == KEY_LEFT && stock_focus == 0) {
                is_in_sidebar = 1;
            } else if (character == KEY_UP) {
                stock_focus = stock_focus > 0 ? stock_focus - 1 : 0;
            } else if (character == KEY_DOWN) {
                stock_focus++;
                if (stock_focus > result_count + 2) stock_focus = result_count + 2;
                if (stock_focus >= 3) {
                    if (stock_focus - 3 >= stock_scroll + PRODUCTS_PER_PAGE)
                        stock_scroll++;
                }
            } else if (character == '\n' || character == KEY_ENTER) {
                if (stock_focus == 0) {
                    stock_search_length = (int)strlen(stock_search);
                    form_editing = 1;
                } else if (stock_focus == 1) {
                    branch_filter = branch_count == 0 ? -1 :
                        (branch_filter + 2) % (branch_count + 1) - 1;
                } else if (stock_focus == 2) {
                    int matched_count = search_products(
                        product_items, stock_search, branch_filter, branches);
                    if (matched_count < 0) {
                        snprintf(status_message, sizeof(status_message),
                                 "Falha ao pesquisar produtos.");
                        status_is_error = 1;
                        menu_focus = 1;
                    } else {
                        map_product_items(products, product_items,
                                          matched_count, branches, branch_count);
                        product_count = matched_count;
                        products_scroll = 0;
                        selected_product = product_count > 0 ? 0 : -1;
                        menu_focus = product_count > 0 ? 2 : 1;
                        applied_search[0] = '\0';
                        applied_branch_filter = -1;
                        snprintf(status_message, sizeof(status_message),
                                 "Pesquisa concluida: %d produto(s).",
                                 product_count);
                        status_is_error = 0;
                    }
                    mode = SCREEN_MENU;
                } else if (stock_focus >= 3 &&
                           stock_focus - 3 < result_count) {
                    selected_product = result_indexes[stock_focus - 3];
                    action_focus = 0;
                    status_message[0] = '\0';
                    mode = SCREEN_ACTIONS;
                }
            } else if (character == 27) {
                mode = SCREEN_MENU;
            }
        }
    }
}
