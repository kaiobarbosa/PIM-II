#include <ncurses.h>
#include "frontend/components/unicode_input.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "backend/src/presentation/branch_presentation.h"
#include "backend/src/presentation/products_presentation.h"
#include "backend/src/presentation/profile_presentation.h"
#include "frontend/components/sidebar.h"
#include "frontend/screens/home_screen.h"
#include "frontend/screens/profile_screen.h"

#define MIN_PROFILE_HEIGHT 24
#define MIN_PROFILE_WIDTH 70
#define MAX_PROFILE_SALES 128
#define MAX_PROFILE_BRANCHES 128
#define PROFILE_HISTORY_PAGE 6

typedef enum {
    PROFILE_HISTORY,
    PROFILE_EDIT,
    PROFILE_CONFIRMATION
} ProfileScreenMode;

typedef enum {
    EDIT_FIRST_NAME,
    EDIT_LAST_NAME,
    EDIT_INTERNAL_CODE,
    EDIT_CRF,
    EDIT_EMAIL,
    EDIT_NEW_PASSWORD,
    EDIT_CONFIRM_PASSWORD,
    EDIT_CANCEL,
    EDIT_SAVE
} ProfileEditField;

static const char *role_label(const char *role) {
    if (strcmp(role, "diretor") == 0) return "Diretor";
    if (strcmp(role, "gerente") == 0) return "Gerente";
    if (strcmp(role, "farmaceutico") == 0) return "Farmaceutico";
    if (strcmp(role, "atendente") == 0) return "Atendente";
    return "Usuario";
}

static int get_edit_fields(const char *role, ProfileEditField fields[]) {
    int count = 0;
    fields[count++] = EDIT_FIRST_NAME;
    fields[count++] = EDIT_LAST_NAME;
    if (strcmp(role, "atendente") != 0) {
        fields[count++] = EDIT_INTERNAL_CODE;
    }
    if (strcmp(role, "farmaceutico") == 0) {
        fields[count++] = EDIT_CRF;
    }
    fields[count++] = EDIT_EMAIL;
    fields[count++] = EDIT_NEW_PASSWORD;
    fields[count++] = EDIT_CONFIRM_PASSWORD;
    return count;
}

static const char *field_label(ProfileEditField field) {
    switch (field) {
        case EDIT_FIRST_NAME: return "Nome";
        case EDIT_LAST_NAME: return "Sobrenome";
        case EDIT_INTERNAL_CODE: return "Codigo interno";
        case EDIT_CRF: return "CRF";
        case EDIT_EMAIL: return "E-mail";
        case EDIT_NEW_PASSWORD: return "Nova senha (opcional)";
        case EDIT_CONFIRM_PASSWORD: return "Confirmar nova senha";
        case EDIT_CANCEL: return "Cancelar";
        case EDIT_SAVE: return "Confirmar";
    }
    return "";
}

static char *field_value(ProfileInfo *info, ProfileEditField field,
                         char new_password[], char confirm_password[],
                         size_t *capacity) {
    switch (field) {
        case EDIT_FIRST_NAME:
            *capacity = sizeof(info->first_name);
            return info->first_name;
        case EDIT_LAST_NAME:
            *capacity = sizeof(info->last_name);
            return info->last_name;
        case EDIT_INTERNAL_CODE:
            *capacity = sizeof(info->internal_code);
            return info->internal_code;
        case EDIT_CRF:
            *capacity = sizeof(info->crf);
            return info->crf;
        case EDIT_EMAIL:
            *capacity = sizeof(info->email);
            return info->email;
        case EDIT_NEW_PASSWORD:
            *capacity = 128;
            return new_password;
        case EDIT_CONFIRM_PASSWORD:
            *capacity = 128;
            return confirm_password;
        default:
            *capacity = 0;
            return NULL;
    }
}

static void draw_profile_history(int screen_height, int hero_left,
                                 int hero_width, const ProdutoItem sales[],
                                 int sales_count, const BranchItem branches[],
                                 int branch_count, int scroll) {
    int table_x = hero_left + 3;
    int table_width = hero_width - 6;
    int date_width = 10;
    int branch_width = 8;
    int quantity_width = 5;
    int product_width = table_width - date_width - branch_width -
                        quantity_width - 3;
    int branch_x = table_x + date_width + 1;
    int quantity_x = branch_x + branch_width + 1;
    int product_x = quantity_x + quantity_width + 1;
    int row;

    attron(COLOR_PAIR(1) | A_BOLD);
    mvadd_utf8_clipped(2, table_x, "HISTORICO DE VENDAS", table_width);
    attroff(COLOR_PAIR(1) | A_BOLD);

    attron(COLOR_PAIR(1) | A_BOLD);
    mvadd_utf8_clipped(5, table_x, "DATA", date_width);
    mvadd_utf8_clipped(5, branch_x, "FILIAL", branch_width);
    mvadd_utf8_clipped(5, quantity_x, "QTD", quantity_width);
    mvadd_utf8_clipped(5, product_x, "PRODUTO", product_width);
    attroff(COLOR_PAIR(1) | A_BOLD);
    mvhline(6, table_x, ACS_HLINE, table_width);

    if (sales_count == 0) {
        mvadd_utf8_clipped(8, table_x, "Nenhuma venda registrada.", table_width);
        return;
    }

    for (row = 0; row < PROFILE_HISTORY_PAGE && row + scroll < sales_count;
         row++) {
        const ProdutoItem *sale = &sales[row + scroll];
        char branch_name[101];
        int branch_index;
        snprintf(branch_name, sizeof(branch_name), "%s", sale->filial);
        for (branch_index = 0; branch_index < branch_count; branch_index++) {
            if (strcmp(sale->filial, branches[branch_index].id_filial) == 0) {
                snprintf(branch_name, sizeof(branch_name), "%s",
                         branches[branch_index].cep);
                break;
            }
        }
        mvadd_utf8_clipped(7 + row, table_x,
                  sale->data_venda[0] != '\0' ? sale->data_venda : "--",
                  date_width);
        mvadd_utf8_clipped(7 + row, branch_x, branch_name, branch_width);
        mvadd_utf8_clipped(7 + row, quantity_x, sale->quantidade, quantity_width);
        mvadd_utf8_clipped(7 + row, product_x, sale->nome, product_width);
    }

    if (sales_count > PROFILE_HISTORY_PAGE) {
        char page[40];
        int last = scroll + PROFILE_HISTORY_PAGE < sales_count ?
                   scroll + PROFILE_HISTORY_PAGE : sales_count;
        snprintf(page, sizeof(page), "%d-%d de %d", scroll + 1, last,
                 sales_count);
        mvadd_utf8_clipped(screen_height - 3, table_x, page, table_width);
    }
}

static void draw_profile_edit(int screen_height, int hero_left, int hero_width,
                              ProfileInfo *edit_info,
                              const ProfileEditField fields[], int field_count,
                              int focus, int editing, char new_password[],
                              char confirm_password[], int password_visible,
                              const char *status,
                              int status_is_error) {
    int index;
    int field_x = hero_left + 23;
    int field_width = hero_width - 28;
    int button_y = screen_height - 5;

    attron(COLOR_PAIR(1) | A_BOLD);
    mvadd_utf8_clipped(2, hero_left + 3, "EDITAR INFORMACOES PESSOAIS",
              hero_width - 6);
    attroff(COLOR_PAIR(1) | A_BOLD);

    for (index = 0; index < field_count; index++) {
        int y = 5 + index * 2;
        size_t capacity;
        char *value = field_value(edit_info, fields[index], new_password,
                                  confirm_password, &capacity);
        mvadd_utf8_clipped(y, hero_left + 3, field_label(fields[index]), 19);
        if (focus == index) attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
        mvaddch(y, field_x, '[');
        mvhline(y, field_x + 1, ' ', field_width - 2);
        mvaddch(y, field_x + field_width - 1, ']');
        if ((fields[index] == EDIT_NEW_PASSWORD ||
             fields[index] == EDIT_CONFIRM_PASSWORD) && !password_visible) {
            char masked[128];
            size_t mask_length = (size_t)input_string_character_count(value);
            memset(masked, '*', mask_length);
            masked[mask_length] = '\0';
            mvadd_utf8_clipped(y, field_x + 2, masked, field_width - 4);
        } else {
            mvadd_utf8_clipped(y, field_x + 2, value, field_width - 4);
        }
        if (focus == index && editing) {
            int cursor_x = field_x + 2 + input_string_display_width(value);
            if (cursor_x > field_x + field_width - 2)
                cursor_x = field_x + field_width - 2;
            move(y, cursor_x);
        }
        if (focus == index) attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
        (void)capacity;
    }

    if (focus == field_count) attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    mvadd_utf8_clipped(button_y, hero_left + 4, "[ CANCELAR ]", 14);
    if (focus == field_count) attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    if (focus == field_count + 1) attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    mvadd_utf8_clipped(button_y, hero_left + 23, "[ CONFIRMAR ]", 16);
    if (focus == field_count + 1) attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);

    if (status[0] != '\0') {
        attron(COLOR_PAIR(status_is_error ? 3 : 2) | A_BOLD);
        mvadd_utf8_clipped(screen_height - 3, hero_left + 3, status, hero_width - 6);
        attroff(COLOR_PAIR(status_is_error ? 3 : 2) | A_BOLD);
    }
}

static void draw_profile_confirmation(int screen_height, int hero_left,
                                      int hero_width,
                                      const char verification_code[],
                                      int focus, int editing,
                                      const char *status,
                                      int status_is_error) {
    int field_x = hero_left + 23;
    int field_width = hero_width - 28;
    int button_y = screen_height - 5;

    attron(COLOR_PAIR(1) | A_BOLD);
    mvadd_utf8_clipped(2, hero_left + 3, "CONFIRMAR ALTERACAO DE SENHA",
              hero_width - 6);
    attroff(COLOR_PAIR(1) | A_BOLD);

    mvadd_utf8_clipped(5, hero_left + 3,
              "Enviamos um codigo de 6 caracteres para o e-mail cadastrado.",
              hero_width - 6);
    mvadd_utf8_clipped(7, hero_left + 3, "Codigo", 19);
    if (focus == 0) attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    mvaddch(7, field_x, '[');
    mvhline(7, field_x + 1, ' ', field_width - 2);
    mvaddch(7, field_x + field_width - 1, ']');
    mvadd_utf8_clipped(7, field_x + 2, verification_code, field_width - 4);
    if (focus == 0 && editing) {
        int cursor_x = field_x + 2 + (int)strlen(verification_code);
        if (cursor_x > field_x + field_width - 2)
            cursor_x = field_x + field_width - 2;
        move(7, cursor_x);
    }
    if (focus == 0) attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);

    if (focus == 1) attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    mvadd_utf8_clipped(button_y, hero_left + 4, "[ VOLTAR ]", 12);
    if (focus == 1) attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    if (focus == 2) attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    mvadd_utf8_clipped(button_y, hero_left + 23, "[ CONFIRMAR ]", 16);
    if (focus == 2) attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);

    if (status[0] != '\0') {
        attron(COLOR_PAIR(status_is_error ? 3 : 2) | A_BOLD);
        mvadd_utf8_clipped(screen_height - 3, hero_left + 3, status, hero_width - 6);
        attroff(COLOR_PAIR(status_is_error ? 3 : 2) | A_BOLD);
    }
}

int profile_screen(char user_id[7], char *user_profile) {
    ProfileInfo profile = {0};
    ProfileInfo edit_info = {0};
    ProdutoItem sales[MAX_PROFILE_SALES] = {{0}};
    BranchItem branches[MAX_PROFILE_BRANCHES] = {{0}};
    ProfileEditField fields[7];
    int field_count;
    int sales_count;
    int branch_count;
    int screen_height;
    int screen_width;
    int sidebar_width;
    int hero_left;
    int hero_width;
    int sidebar_focus = 1;
    int is_in_sidebar = 1;
    int history_scroll = 0;
    int password_info_visible = 0;
    int mode = PROFILE_HISTORY;
    int focus = 0;
    int editing = 0;
    int password_visible = 0;
    int status_is_error = 0;
    int character;
    char new_password[128] = "";
    char confirm_password[128] = "";
    char verification_code[7] = "";
    char status_message[120] = "";

    if (get_profile_presentation(user_id, user_profile, &profile) != 1) {
        return APP_SCREEN_HOME;
    }
    sales_count = select_produtos_vendidos_by_user_id_presentation(
        user_id, sales, MAX_PROFILE_SALES);
    if (sales_count < 0) sales_count = 0;
    branch_count = select_all_branches_presentation(branches,
                                                     MAX_PROFILE_BRANCHES);
    if (branch_count < 0) branch_count = 0;
    field_count = get_edit_fields(profile.profile, fields);

    getmaxyx(stdscr, screen_height, screen_width);
    if (screen_height < MIN_PROFILE_HEIGHT || screen_width < MIN_PROFILE_WIDTH) {
        draw_resize_message(screen_height, screen_width);
        return APP_SCREEN_HOME;
    }
    sidebar_width = screen_width * 2 / 5;
    hero_left = sidebar_width + 1;
    hero_width = screen_width - hero_left - 1;
    keypad(stdscr, TRUE);

    while (1) {
        erase();
        box(stdscr, 0, 0);
        draw_profile_sidebar(screen_height, sidebar_width,
                             profile.first_name, profile.last_name,
                             role_label(profile.profile),
                             profile.internal_code, profile.crf, profile.branch,
                             profile.email, password_info_visible,
                             is_in_sidebar ? sidebar_focus : -1,
                             mode != PROFILE_HISTORY);

        if (mode == PROFILE_HISTORY) {
            draw_profile_history(screen_height, hero_left, hero_width, sales,
                                 sales_count, branches, branch_count,
                                 history_scroll);
            if (status_message[0] != '\0') {
                attron(COLOR_PAIR(status_is_error ? 3 : 2) | A_BOLD);
                mvadd_utf8_clipped(screen_height - 3, hero_left + 3, status_message,
                          hero_width - 6);
                attroff(COLOR_PAIR(status_is_error ? 3 : 2) | A_BOLD);
            }
        } else if (mode == PROFILE_EDIT) {
            draw_profile_edit(screen_height, hero_left, hero_width, &edit_info,
                              fields, field_count, focus, editing, new_password,
                              confirm_password, password_visible, status_message,
                              status_is_error);
        } else {
            draw_profile_confirmation(screen_height, hero_left, hero_width,
                                      verification_code, focus, editing,
                                      status_message, status_is_error);
        }

        attron(A_DIM);
        if (mode == PROFILE_CONFIRMATION && editing) {
            mvadd_utf8_clipped(screen_height - 2, 2,
                      "Digite o codigo | Enter: concluir campo | Esc: voltar",
                      screen_width - 4);
        } else if (mode == PROFILE_CONFIRMATION) {
            mvadd_utf8_clipped(screen_height - 2, 2,
                      "Setas: navegar | Enter: editar/selecionar | Esc: voltar",
                      screen_width - 4);
        } else if (mode == PROFILE_EDIT && editing) {
            mvadd_utf8_clipped(screen_height - 2, 2,
                      "Digite o campo | Enter: concluir campo | Esc: parar",
                      screen_width - 4);
        } else if (mode == PROFILE_EDIT) {
            mvadd_utf8_clipped(screen_height - 2, 2,
                      "Setas: navegar | Enter: editar/selecionar | Esc: cancelar",
                      screen_width - 4);
        } else {
            mvadd_utf8_clipped(screen_height - 2, 2,
                      "Setas: navegar | PgUp/PgDn: historico | Enter: selecionar | Esc: voltar",
                      screen_width - 4);
        }
        attroff(A_DIM);
        curs_set(editing ? 1 : 0);
        refresh();

        character = read_input_character();
        if (mode == PROFILE_HISTORY && is_in_sidebar) {
            if (character == 27) return APP_SCREEN_HOME;
            if (character == KEY_UP || character == KEY_DOWN) {
                sidebar_focus = (sidebar_focus +
                    (character == KEY_DOWN ? 1 : 2)) % 3;
            } else if (character == KEY_RIGHT) {
                is_in_sidebar = 0;
            } else if (character == '\n' || character == KEY_ENTER) {
                if (sidebar_focus == 0) {
                    password_info_visible = !password_info_visible;
                    snprintf(status_message, sizeof(status_message),
                             "A senha original nao pode ser recuperada; "
                             "o banco armazena hash bcrypt.");
                    status_is_error = 0;
                } else if (sidebar_focus == 1) {
                    edit_info = profile;
                    new_password[0] = '\0';
                    confirm_password[0] = '\0';
                    password_visible = 0;
                    focus = 0;
                    editing = 0;
                    mode = PROFILE_EDIT;
                    status_message[0] = '\0';
                } else {
                    return APP_SCREEN_HOME;
                }
            }
            continue;
        }

        if (mode == PROFILE_CONFIRMATION) {
            if (editing) {
                size_t length = strlen(verification_code);
                if (character == '\n' || character == KEY_ENTER ||
                    character == 27) {
                    editing = 0;
                } else if ((character == KEY_BACKSPACE || character == 127 ||
                            character == 8) && length > 0) {
                    verification_code[length - 1] = '\0';
                } else if (length < 6 && character >= 32 && character <= 126 &&
                           isalnum((unsigned char)character)) {
                    verification_code[length] =
                        (char)toupper((unsigned char)character);
                    verification_code[length + 1] = '\0';
                }
            } else if (character == 27) {
                mode = PROFILE_EDIT;
                verification_code[0] = '\0';
                status_message[0] = '\0';
            } else if (character == KEY_UP || character == KEY_DOWN) {
                focus = (focus + (character == KEY_DOWN ? 1 : 2)) % 3;
            } else if ((character == '\n' || character == KEY_ENTER) &&
                       focus == 0) {
                editing = 1;
            } else if ((character == '\n' || character == KEY_ENTER) &&
                       focus == 1) {
                mode = PROFILE_EDIT;
                verification_code[0] = '\0';
                status_message[0] = '\0';
            } else if ((character == '\n' || character == KEY_ENTER) &&
                       focus == 2) {
                int confirmation_result =
                    confirm_profile_password_change_presentation(
                        &edit_info, profile.email, verification_code,
                        new_password);
                if (confirmation_result == 0) {
                    profile = edit_info;
                    get_profile_presentation(user_id, user_profile, &profile);
                    sales_count = select_produtos_vendidos_by_user_id_presentation(
                        user_id, sales, MAX_PROFILE_SALES);
                    if (sales_count < 0) sales_count = 0;
                    history_scroll = 0;
                    new_password[0] = '\0';
                    confirm_password[0] = '\0';
                    verification_code[0] = '\0';
                    mode = PROFILE_HISTORY;
                    snprintf(status_message, sizeof(status_message),
                             "Perfil atualizado com sucesso.");
                    status_is_error = 0;
                } else if (confirmation_result == -2) {
                    mode = PROFILE_EDIT;
                    verification_code[0] = '\0';
                    snprintf(status_message, sizeof(status_message),
                             "Falha ao salvar. Revise os dados e confirme novamente.");
                    status_is_error = 1;
                } else {
                    verification_code[0] = '\0';
                    focus = 0;
                    snprintf(status_message, sizeof(status_message),
                             "Codigo invalido ou expirado. Tente novamente.");
                    status_is_error = 1;
                }
            }
        } else if (mode == PROFILE_EDIT) {
            if (editing) {
                size_t capacity;
                char *value = field_value(&edit_info, fields[focus],
                                          new_password, confirm_password,
                                          &capacity);
                if (character == '\n' || character == KEY_ENTER ||
                    character == 27) {
                    editing = 0;
                } else if (character == KEY_BACKSPACE || character == 127 ||
                           character == 8) {
                    input_backspace(value);
                } else if (input_character_is_printable(character)) {
                    size_t length = strlen(value);
                    input_character_append(value, capacity, &length,
                                           character);
                }
            } else if (character == 27) {
                mode = PROFILE_HISTORY;
                status_message[0] = '\0';
            } else if (character == KEY_LEFT && focus == 0) {
                is_in_sidebar = 1;
            } else if (character == KEY_UP) {
                focus = focus == 0 ? field_count + 1 : focus - 1;
            } else if (character == KEY_DOWN) {
                focus = (focus + 1) % (field_count + 2);
            } else if (character == KEY_RIGHT && focus < field_count &&
                       (fields[focus] == EDIT_NEW_PASSWORD ||
                        fields[focus] == EDIT_CONFIRM_PASSWORD)) {
                password_visible = !password_visible;
            } else if (character == '\n' || character == KEY_ENTER) {
                if (focus < field_count) {
                    editing = 1;
                } else if (focus == field_count) {
                    mode = PROFILE_HISTORY;
                    status_message[0] = '\0';
                } else if ((new_password[0] != '\0' ||
                            confirm_password[0] != '\0') &&
                           strcmp(new_password, confirm_password) != 0) {
                    new_password[0] = '\0';
                    confirm_password[0] = '\0';
                    focus = field_count - 2;
                    snprintf(status_message, sizeof(status_message),
                             "As senhas precisam coincidir. Preencha novamente.");
                    status_is_error = 1;
                } else if (new_password[0] != '\0') {
                    if (request_profile_password_change_code_presentation(
                            &edit_info, profile.email) == 0) {
                        verification_code[0] = '\0';
                        focus = 0;
                        editing = 0;
                        mode = PROFILE_CONFIRMATION;
                        snprintf(status_message, sizeof(status_message),
                                 "Codigo enviado para o e-mail cadastrado.");
                        status_is_error = 0;
                    } else {
                        snprintf(status_message, sizeof(status_message),
                                 "Nao foi possivel enviar. Verifique os campos e a configuracao SMTP.");
                        status_is_error = 1;
                    }
                } else if (update_profile_presentation(
                               &edit_info, "") == 0) {
                    profile = edit_info;
                    get_profile_presentation(user_id, user_profile, &profile);
                    sales_count = select_produtos_vendidos_by_user_id_presentation(
                        user_id, sales, MAX_PROFILE_SALES);
                    if (sales_count < 0) sales_count = 0;
                    history_scroll = 0;
                    new_password[0] = '\0';
                    confirm_password[0] = '\0';
                    mode = PROFILE_HISTORY;
                    snprintf(status_message, sizeof(status_message),
                             "Perfil atualizado com sucesso.");
                    status_is_error = 0;
                } else {
                    snprintf(status_message, sizeof(status_message),
                             "Nao foi possivel salvar. Revise os campos.");
                    status_is_error = 1;
                }
            }
        } else if (character == 27) {
            return APP_SCREEN_HOME;
        } else if (character == KEY_LEFT) {
            is_in_sidebar = 1;
        } else if (character == KEY_UP) {
            if (history_scroll > 0) history_scroll--;
        } else if (character == KEY_DOWN) {
            if (history_scroll + PROFILE_HISTORY_PAGE < sales_count)
                history_scroll++;
        } else if (character == KEY_NPAGE) {
            if (history_scroll + PROFILE_HISTORY_PAGE < sales_count)
                history_scroll += PROFILE_HISTORY_PAGE;
        } else if (character == KEY_PPAGE) {
            history_scroll = history_scroll >= PROFILE_HISTORY_PAGE ?
                             history_scroll - PROFILE_HISTORY_PAGE : 0;
        }
    }
}
