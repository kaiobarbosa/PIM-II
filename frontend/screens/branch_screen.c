#include <ncurses.h>
#include "frontend/components/unicode_input.h"
#include <stdio.h>
#include <string.h>

#include "frontend/components/sidebar.h"
#include "frontend/screens/branch_screen.h"
#include "frontend/screens/home_screen.h"
#include "backend/src/presentation/branch_presentation.h"

#define MAX_BRANCHES 10
#define MAX_BRANCH_FIELDS 8

typedef struct {
    char id[16];
    char postal_code[12];
    char street[64];
    char number[12];
    char complement[32];
    char neighborhood[40];
    char city[40];
    char state[3];
    int active;
} Branch;

typedef enum {
    FIELD_ID,
    FIELD_POSTAL_CODE,
    FIELD_STREET,
    FIELD_NUMBER,
    FIELD_COMPLEMENT,
    FIELD_NEIGHBORHOOD,
    FIELD_CITY,
    FIELD_STATE
} BranchField;

typedef enum {
    SCREEN_LIST,
    SCREEN_NEW_FORM,
    SCREEN_ACTIONS,
    SCREEN_EDIT_FORM,
    SCREEN_INACTIVATE_CONFIRM
} BranchScreen;

static const char *field_labels[] = {
    "ID da filial", "CEP", "Logradouro", "Numero", "Complemento",
    "Bairro", "Cidade", "Estado"
};

static void clear_branch(Branch *branch) {
    memset(branch, 0, sizeof(*branch));
    branch->active = 1;
}

static void get_branch_fields(int editing, BranchField fields[],
                              int *field_count) {
    int count = 0;
    int field;

    if (editing) {
        fields[count++] = FIELD_ID;
    }
    for (field = FIELD_POSTAL_CODE; field <= FIELD_STATE; field++) {
        fields[count++] = (BranchField) field;
    }
    *field_count = count;
}

static char *get_branch_value(Branch *branch, BranchField field,
                              int *capacity) {
    switch (field) {
        case FIELD_ID:
            *capacity = sizeof(branch->id);
            return branch->id;
        case FIELD_POSTAL_CODE:
            *capacity = sizeof(branch->postal_code);
            return branch->postal_code;
        case FIELD_STREET:
            *capacity = sizeof(branch->street);
            return branch->street;
        case FIELD_NUMBER:
            *capacity = sizeof(branch->number);
            return branch->number;
        case FIELD_COMPLEMENT:
            *capacity = sizeof(branch->complement);
            return branch->complement;
        case FIELD_NEIGHBORHOOD:
            *capacity = sizeof(branch->neighborhood);
            return branch->neighborhood;
        case FIELD_CITY:
            *capacity = sizeof(branch->city);
            return branch->city;
        case FIELD_STATE:
            *capacity = sizeof(branch->state);
            return branch->state;
        default:
            *capacity = 0;
            return NULL;
    }
}

static void draw_branch_list(int screen_height, int hero_left, int hero_width,
                             int divider_y, Branch branches[],
                             int branch_count, int focus, int scroll_offset,
                             int can_create) {
    int row;
    int content_width = hero_width - 8;
    int visible_count = (screen_height - divider_y - 9) / 2;

    attron(COLOR_PAIR(1) | A_BOLD);
    mvadd_utf8_clipped(2, hero_left + 4, "GERENCIAMENTO DE FILIAIS", hero_width - 8);
    attroff(COLOR_PAIR(1) | A_BOLD);
    mvhline(4, hero_left + 2, ACS_HLINE, hero_width - 4);

    if (can_create) {
        if (focus == 0) attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
        mvadd_utf8_clipped(6, hero_left + 4, "+ Cadastrar nova filial", hero_width - 8);
        if (focus == 0) attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    }

    mvhline(divider_y, hero_left, ACS_HLINE, hero_width);
    attron(COLOR_PAIR(1) | A_BOLD);
    mvadd_utf8_clipped(divider_y + 2, hero_left + 4, "FILIAIS CADASTRADAS",
              hero_width - 8);
    attroff(COLOR_PAIR(1) | A_BOLD);
    mvadd_utf8_clipped(divider_y + 4, hero_left + 4, "CEP", content_width / 2);
    mvadd_utf8_clipped(divider_y + 4, hero_left + 4 + content_width / 2,
              "CIDADE / UF", content_width / 2);
    mvhline(divider_y + 5, hero_left + 4, ACS_HLINE, content_width);

    for (row = 0; row < visible_count && row + scroll_offset < branch_count;
         row++) {
        int y = divider_y + 7 + row * 2;
        int branch_index = row + scroll_offset;
        char location[48];

        snprintf(location, sizeof(location), "%s / %s",
                 branches[branch_index].city, branches[branch_index].state);

        if (focus == branch_index + 1) {
            attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
            mvhline(y, hero_left + 3, ' ', content_width + 2);
        } else if (!branches[branch_index].active) {
            attron(A_DIM);
        }
        mvadd_utf8_clipped(y, hero_left + 4, branches[branch_index].postal_code,
                  content_width / 2);
        mvadd_utf8_clipped(y, hero_left + 4 + content_width / 2, location,
                  content_width / 2);
        if (focus == branch_index + 1) {
            attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
        } else if (!branches[branch_index].active) {
            attroff(A_DIM);
        }
    }
}

static void draw_branch_form(int screen_height, int hero_left, int hero_width,
                             int divider_y, Branch *branch, int editing,
                             int focus, int text_editing,
                             const BranchField fields[], int field_count) {
    int index;
    int label_x = hero_left + 4;
    int value_x = hero_left + 20;
    int field_width = hero_width - 25;
    int title_y = divider_y + 1;
    int first_field_y = divider_y + 3;
    int buttons_y = screen_height - 4;

    attron(COLOR_PAIR(1) | A_BOLD);
    mvadd_utf8_clipped(title_y, hero_left + 4,
              editing ? "EDITAR FILIAL" : "CADASTRAR FILIAL", hero_width - 8);
    attroff(COLOR_PAIR(1) | A_BOLD);
    mvhline(divider_y + 2, hero_left + 2, ACS_HLINE, hero_width - 4);

    for (index = 0; index < field_count; index++) {
        int y = first_field_y + index;
        int capacity;
        int selected = focus == index;
        char *value = get_branch_value(branch, fields[index], &capacity);

        mvadd_utf8_clipped(y, label_x, field_labels[fields[index]], 15);
        if (selected) {
            attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
        } else {
            attron(COLOR_PAIR(1));
        }
        mvaddch(y, value_x, '[');
        mvhline(y, value_x + 1, ' ', field_width - 2);
        mvaddch(y, value_x + field_width - 1, ']');
        mvadd_utf8_clipped(y, value_x + 2, value, field_width - 4);
        if (selected && text_editing) {
            int cursor_x = value_x + 2 + input_string_display_width(value);
            int max_x = value_x + field_width - 2;
            if (cursor_x > max_x) cursor_x = max_x;
            move(y, cursor_x);
        }
        if (selected) {
            attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
        } else {
            attroff(COLOR_PAIR(1));
        }
    }

    if (focus == field_count) {
        attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    }
    mvadd_utf8_clipped(buttons_y, hero_left + 4, "[ CANCELAR ]", 14);
    if (focus == field_count) {
        attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    }

    if (focus == field_count + 1) {
        attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    }
    mvadd_utf8_clipped(buttons_y, hero_left + 23,
              editing ? "[ CONFIRMAR ]" : "[ CADASTRAR ]", 16);
    if (focus == field_count + 1) {
        attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    }
}

static void draw_choice(int hero_left, int hero_width, int divider_y,
                        const char *title, const char *message,
                        const char *first_option, const char *second_option,
                        int focus) {
    int center_x = hero_left + hero_width / 2;
    int title_y = divider_y + 2;
    int option_y = divider_y + 6;

    attron(COLOR_PAIR(1) | A_BOLD);
    mvadd_utf8_clipped(title_y, hero_left + 4, title, hero_width - 8);
    attroff(COLOR_PAIR(1) | A_BOLD);
    mvadd_utf8_clipped(title_y + 2, hero_left + 4, message, hero_width - 8);
    if (focus == 0) attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    mvadd_utf8_clipped(option_y, center_x - 14, first_option, 12);
    if (focus == 0) attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    if (focus == 1) attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    mvadd_utf8_clipped(option_y, center_x + 2, second_option, 12);
    if (focus == 1) attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
}

static void draw_branch_screen(int screen_height, int screen_width,
                              int sidebar_width, int hero_left, int hero_width,
                              int divider_y, BranchScreen screen,
                              Branch branches[], int branch_count,
                              int branch_focus, int list_scroll,
                              int action_focus, int confirmation_focus,
                              Branch *form_branch, int form_focus,
                              int form_editing, const BranchField fields[],
                              int field_count, int sidebar_focus,
                              int is_in_sidebar, const char *status_message,
                              int status_is_error, const char *user_profile) {
    erase();
    box(stdscr, 0, 0);
    mvvline(1, sidebar_width, ACS_VLINE, screen_height - 2);
    draw_sidebar(screen_height, sidebar_width,
                 is_in_sidebar ? sidebar_focus : -1, user_profile);

    if (screen == SCREEN_LIST) {
        draw_branch_list(screen_height, hero_left, hero_width, divider_y,
                         branches, branch_count,
                         is_in_sidebar ? -1 : branch_focus, list_scroll,
                         strcmp(user_profile, "diretor") == 0);
    } else if (screen == SCREEN_NEW_FORM || screen == SCREEN_EDIT_FORM) {
        draw_branch_form(screen_height, hero_left, hero_width, divider_y,
                         form_branch, screen == SCREEN_EDIT_FORM, form_focus,
                         form_editing, fields, field_count);
    } else if (screen == SCREEN_ACTIONS) {
        draw_choice(hero_left, hero_width, divider_y, "ACOES DA FILIAL",
                    "Escolha uma acao para a filial selecionada.",
                    "[ EDITAR ]", "[ INATIVAR ]", action_focus);
    } else if (screen == SCREEN_INACTIVATE_CONFIRM) {
        draw_choice(hero_left, hero_width, divider_y, "INATIVAR FILIAL",
                    "Tem certeza que deseja inativar esta filial?",
                    "[ SIM ]", "[ NAO ]", confirmation_focus);
    }

    if (status_message[0] != '\0') {
        attron(COLOR_PAIR(status_is_error ? 3 : 2) | A_BOLD);
        mvadd_utf8_clipped(screen_height - 3, hero_left + 4, status_message,
                  hero_width - 8);
        attroff(COLOR_PAIR(status_is_error ? 3 : 2) | A_BOLD);
    }

    attron(A_DIM);
    if (screen == SCREEN_NEW_FORM || screen == SCREEN_EDIT_FORM) {
        mvadd_utf8_clipped(screen_height - 2, 2,
                  form_editing ? "Digite o valor  |  Enter: concluir edicao  |  Esc: cancelar campo" :
                  "Setas: navegar  |  Enter: editar/selecionar  |  Esc: voltar",
                  screen_width - 4);
    } else {
        mvadd_utf8_clipped(screen_height - 2, 2,
                  "Setas: navegar  |  Enter: selecionar  |  Esc: voltar",
                  screen_width - 4);
    }
    attroff(A_DIM);
    curs_set(form_editing ? 1 : 0);
    refresh();
}

int branch_screen(char user_id[7], char *user_profile) {
    Branch branches[MAX_BRANCHES] = {{0}};
    BranchItem branch_items[MAX_BRANCHES] = {{0}};
    int branch_count;
    int screen_height, screen_width;
    int sidebar_width, hero_left, hero_width, divider_y;
    int sidebar_focus = 3;
    int is_in_sidebar = 1;
    int branch_focus = 0;
    int list_scroll = 0;
    int action_focus = 0;
    int confirmation_focus = 1;
    int selected_branch = -1;
    int form_focus = 0;
    int form_editing = 0;
    int character;
    Branch form_branch;
    BranchScreen screen = SCREEN_LIST;
    char status_message[80] = "";
    int status_is_error = 0;
    int director_mode;

    if (user_profile == NULL ||
        (strcmp(user_profile, "diretor") != 0 &&
         strcmp(user_profile, "gerente") != 0)) return APP_SCREEN_HOME;
    director_mode = strcmp(user_profile, "diretor") == 0;

    branch_count = select_all_branches_presentation(branch_items,
                                                     MAX_BRANCHES);
    if (branch_count < 0) {
        branch_count = 0;
    }
    for (int index = 0; index < branch_count; index++) {
        snprintf(branches[index].id, sizeof(branches[index].id), "%s",
                 branch_items[index].id_filial);
        snprintf(branches[index].postal_code,
                 sizeof(branches[index].postal_code), "%s",
                 branch_items[index].cep);
        snprintf(branches[index].city, sizeof(branches[index].city), "%s",
                 branch_items[index].cidade);
        snprintf(branches[index].state, sizeof(branches[index].state), "%s",
                 branch_items[index].estado);
        branches[index].active = 1;
    }

    keypad(stdscr, TRUE);
    clear_branch(&form_branch);
    if (!director_mode && branch_count > 0) branch_focus = 1;

    while (1) {
        BranchField fields[MAX_BRANCH_FIELDS];
        int field_count = 0;
        int character_is_confirm;

        wait_for_minimum_terminal_size();
        getmaxyx(stdscr, screen_height, screen_width);
        sidebar_width = screen_width / 3;
        hero_left = sidebar_width + 1;
        hero_width = screen_width - hero_left - 1;
        divider_y = 1 + (screen_height - 2) / 3;

        if (screen == SCREEN_NEW_FORM || screen == SCREEN_EDIT_FORM) {
            get_branch_fields(screen == SCREEN_EDIT_FORM, fields, &field_count);
        }
        draw_branch_screen(screen_height, screen_width, sidebar_width,
                           hero_left, hero_width, divider_y, screen,
                           branches, branch_count, branch_focus, list_scroll,
                           action_focus, confirmation_focus, &form_branch,
                           form_focus, form_editing, fields, field_count,
                           sidebar_focus, is_in_sidebar, status_message,
                           status_is_error, user_profile);

        character = read_input_character();
        if (character == KEY_RESIZE) continue;
        character_is_confirm = character == '\n' || character == KEY_ENTER;

        if (is_in_sidebar) {
            if (character == 27) {
                return APP_SCREEN_HOME;
            } else if (character == KEY_UP) {
                sidebar_focus = sidebar_move_focus(user_profile, sidebar_focus, -1);
            } else if (character == KEY_DOWN) {
                sidebar_focus = sidebar_move_focus(user_profile, sidebar_focus, 1);
            } else if (character == KEY_RIGHT ||
                       (character_is_confirm && sidebar_focus == 3)) {
                is_in_sidebar = 0;
            } else if (character_is_confirm && sidebar_focus == 0) {
                return APP_SCREEN_HOME;
            } else if (character_is_confirm && sidebar_focus == 2) {
                return APP_SCREEN_EMPLOYEES;
            } else if (character_is_confirm && sidebar_focus == 1) {
                return APP_SCREEN_PRODUCTS;
            } else if (character_is_confirm && sidebar_focus == 4) {
                return APP_SCREEN_PROFILE;
            }
            continue;
        }

        if (screen == SCREEN_LIST) {
            int visible_count = (screen_height - divider_y - 9) / 2;
            int last_focus = branch_count;

            if (character == KEY_LEFT || character == 27) {
                is_in_sidebar = 1;
                continue;
            } else if (character == KEY_UP) {
                if (!director_mode && branch_count > 0)
                    branch_focus = branch_focus <= 1 ? last_focus : branch_focus - 1;
                else branch_focus = branch_focus == 0 ? last_focus : branch_focus - 1;
            } else if (character == KEY_DOWN) {
                if (!director_mode && branch_count > 0)
                    branch_focus = branch_focus >= last_focus ? 1 : branch_focus + 1;
                else branch_focus = branch_focus == last_focus ? 0 : branch_focus + 1;
            } else if (character == KEY_RIGHT) {
                if (director_mode)
                    branch_focus = branch_focus == 0 ? last_focus : 0;
            } else if (character_is_confirm && branch_focus == 0 && director_mode) {
                clear_branch(&form_branch);
                status_message[0] = '\0';
                status_is_error = 0;
                form_focus = 0;
                form_editing = 0;
                screen = SCREEN_NEW_FORM;
            } else if (character_is_confirm && branch_focus > 0 &&
                       branch_focus <= branch_count &&
                       branches[branch_focus - 1].active) {
                if (!director_mode) {
                    snprintf(status_message, sizeof(status_message),
                             "Acesso somente para visualizacao.");
                    status_is_error = 0;
                    continue;
                }
                selected_branch = branch_focus - 1;
                action_focus = 0;
                screen = SCREEN_ACTIONS;
            }

            if (branch_focus > list_scroll + visible_count) {
                list_scroll = branch_focus - visible_count;
            } else if (branch_focus > 0 && branch_focus <= list_scroll) {
                list_scroll = branch_focus - 1;
            }
        } else if (screen == SCREEN_ACTIONS) {
            if (character == KEY_UP || character == KEY_DOWN ||
                character == KEY_LEFT || character == KEY_RIGHT) {
                action_focus = !action_focus;
            } else if (character_is_confirm) {
                if (action_focus == 0) {
                    form_branch = branches[selected_branch];
                    form_focus = 0;
                    form_editing = 0;
                    screen = SCREEN_EDIT_FORM;
                } else {
                    confirmation_focus = 1;
                    screen = SCREEN_INACTIVATE_CONFIRM;
                }
            } else if (character == 27) {
                screen = SCREEN_LIST;
            }
        } else if (screen == SCREEN_INACTIVATE_CONFIRM) {
            if (character == KEY_LEFT || character == KEY_RIGHT ||
                character == KEY_UP || character == KEY_DOWN) {
                confirmation_focus = !confirmation_focus;
            } else if (character_is_confirm) {
                if (confirmation_focus == 0) {
                    if (selected_branch >= 0) {
                        branches[selected_branch].active = 0;
                    }
                    screen = SCREEN_LIST;
                } else {
                    screen = SCREEN_ACTIONS;
                }
            } else if (character == 27) {
                screen = SCREEN_ACTIONS;
            }
        } else {
            BranchField current_fields[MAX_BRANCH_FIELDS];
            int current_field_count = 0;
            int capacity = 0;
            char *value = NULL;
            int is_new = screen == SCREEN_NEW_FORM;

            get_branch_fields(!is_new, current_fields, &current_field_count);
            if (form_editing) {
                value = get_branch_value(&form_branch,
                                         current_fields[form_focus],
                                         &capacity);
                if (character_is_confirm || character == 27) {
                    form_editing = 0;
                } else if (character == KEY_BACKSPACE || character == 127 ||
                           character == 8) {
                    input_backspace(value);
                } else if (input_character_is_printable(character)) {
                    size_t length = strlen(value);
                    input_character_append(value, (size_t)capacity, &length,
                                           character);
                }
            } else if (character == KEY_UP) {
                form_focus = form_focus == 0 ? current_field_count + 1 :
                             form_focus - 1;
            } else if (character == KEY_DOWN) {
                form_focus = (form_focus + 1) % (current_field_count + 2);
            } else if (character == KEY_LEFT || character == KEY_RIGHT) {
                if (form_focus >= current_field_count) {
                    form_focus = form_focus == current_field_count ?
                                 current_field_count + 1 : current_field_count;
                }
            } else if (character_is_confirm) {
                if (form_focus < current_field_count) {
                    form_editing = 1;
                } else if (form_focus == current_field_count) {
                    if (is_new) {
                        screen = SCREEN_LIST;
                    } else {
                        branches[selected_branch] = form_branch;
                        screen = SCREEN_LIST;
                    }
                } else if (is_new) {
                    int registration_result;

                    if (user_id == NULL || user_id[0] == '\0') {
                        registration_result = -1;
                    } else {
                        registration_result = register_new_branch(
                            user_id,
                            form_branch.postal_code,
                            form_branch.street,
                            form_branch.number,
                            form_branch.complement,
                            form_branch.neighborhood,
                            form_branch.city,
                            form_branch.state);
                    }

                    if (registration_result == 0) {
                        snprintf(status_message, sizeof(status_message),
                                 "Filial cadastrada com sucesso.");
                        status_is_error = 0;
                        clear_branch(&form_branch);
                        branch_focus = 0;
                        screen = SCREEN_LIST;
                    } else {
                        snprintf(status_message, sizeof(status_message),
                                 "Falha ao cadastrar filial. Verifique os dados e tente novamente.");
                        status_is_error = 1;
                    }
                }
            } else if (character == 27) {
                screen = SCREEN_LIST;
            }

            if (form_focus > current_field_count + 1) {
                form_focus = current_field_count + 1;
            }
        }
    }
}
