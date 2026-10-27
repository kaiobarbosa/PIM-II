#include <ncurses.h>
#include <stdio.h>
#include <string.h>

#include "frontend/components/sidebar.h"
#include "frontend/screens/employees_screen.h"
#include "frontend/screens/home_screen.h"

#define MIN_SCREEN_HEIGHT 24
#define MIN_SCREEN_WIDTH 70
#define MAX_EMPLOYEES 10
#define MAX_FORM_FIELDS 8

typedef enum {
    ROLE_DIRECTOR,
    ROLE_MANAGER,
    ROLE_PHARMACIST,
    ROLE_CLERK,
    ROLE_COUNT
} EmployeeRole;

typedef enum {
    FIELD_ROLE,
    FIELD_FIRST_NAME,
    FIELD_LAST_NAME,
    FIELD_CODE,
    FIELD_BRANCH,
    FIELD_CRF,
    FIELD_EMAIL,
    FIELD_PASSWORD
} FormField;

typedef enum {
    SCREEN_LIST,
    SCREEN_NEW_FORM,
    SCREEN_ACTIONS,
    SCREEN_EDIT_FORM,
    SCREEN_INACTIVATE_CONFIRM,
    SCREEN_DELETE_CONFIRM
} EmployeeScreen;

typedef struct {
    char first_name[40];
    char last_name[40];
    char code[32];
    char branch[40];
    char crf[32];
    char email[64];
    char password[64];
    EmployeeRole role;
    int active;
} Employee;

static const char *role_names[ROLE_COUNT] = {
    "Diretor", "Gerente", "Farmaceutico", "Atendente"
};

static void get_form_fields(EmployeeRole role, FormField fields[],
                            int *field_count) {
    int count = 0;

    fields[count++] = FIELD_ROLE;
    fields[count++] = FIELD_FIRST_NAME;
    fields[count++] = FIELD_LAST_NAME;
    fields[count++] = FIELD_CODE;
    if (role != ROLE_DIRECTOR) {
        fields[count++] = FIELD_BRANCH;
    }
    if (role == ROLE_PHARMACIST) {
        fields[count++] = FIELD_CRF;
    }
    fields[count++] = FIELD_EMAIL;
    fields[count++] = FIELD_PASSWORD;
    *field_count = count;
}

static char *get_field_value(Employee *employee, FormField field,
                             int *capacity) {
    switch (field) {
        case FIELD_FIRST_NAME:
            *capacity = sizeof(employee->first_name);
            return employee->first_name;
        case FIELD_LAST_NAME:
            *capacity = sizeof(employee->last_name);
            return employee->last_name;
        case FIELD_CODE:
            *capacity = sizeof(employee->code);
            return employee->code;
        case FIELD_BRANCH:
            *capacity = sizeof(employee->branch);
            return employee->branch;
        case FIELD_CRF:
            *capacity = sizeof(employee->crf);
            return employee->crf;
        case FIELD_EMAIL:
            *capacity = sizeof(employee->email);
            return employee->email;
        case FIELD_PASSWORD:
            *capacity = sizeof(employee->password);
            return employee->password;
        case FIELD_ROLE:
        default:
            *capacity = 0;
            return NULL;
    }
}

static const char *get_field_label(FormField field) {
    switch (field) {
        case FIELD_ROLE: return "Cargo";
        case FIELD_FIRST_NAME: return "Nome";
        case FIELD_LAST_NAME: return "Sobrenome";
        case FIELD_CODE: return "Codigo interno";
        case FIELD_BRANCH: return "Filial";
        case FIELD_CRF: return "CRF";
        case FIELD_EMAIL: return "Email";
        case FIELD_PASSWORD: return "Senha";
        default: return "";
    }
}

static void clear_employee(Employee *employee) {
    memset(employee, 0, sizeof(*employee));
    employee->role = ROLE_CLERK;
    employee->active = 1;
}

static void draw_employee_list(int screen_height, int hero_left,
                               int hero_width, int divider_y,
                               Employee employees[], int employee_count,
                               int focus, int scroll_offset) {
    int row;
    int content_width = hero_width - 8;
    int visible_count = (screen_height - divider_y - 9) / 2;

    attron(COLOR_PAIR(1) | A_BOLD);
    mvaddnstr(2, hero_left + 4, "GERENCIAMENTO DE FUNCIONARIOS",
              hero_width - 8);
    attroff(COLOR_PAIR(1) | A_BOLD);
    mvhline(4, hero_left + 2, ACS_HLINE, hero_width - 4);

    if (focus == 0) {
        attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    }
    mvaddnstr(6, hero_left + 4, "+ Cadastrar novo funcionario",
              hero_width - 8);
    if (focus == 0) {
        attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    }

    mvhline(divider_y, hero_left, ACS_HLINE, hero_width);
    attron(COLOR_PAIR(1) | A_BOLD);
    mvaddnstr(divider_y + 2, hero_left + 4, "FUNCIONARIOS CADASTRADOS",
              hero_width - 8);
    attroff(COLOR_PAIR(1) | A_BOLD);
    mvaddnstr(divider_y + 4, hero_left + 4, "NOME", content_width / 2);
    mvaddnstr(divider_y + 4, hero_left + 4 + content_width / 2,
              "CARGO", content_width / 2);
    mvhline(divider_y + 5, hero_left + 4, ACS_HLINE, content_width);

    for (row = 0; row < visible_count && row + scroll_offset < employee_count;
         row++) {
        int y = divider_y + 7 + row * 2;
        char full_name[84];
        int employee_index = row + scroll_offset;
        snprintf(full_name, sizeof(full_name), "%s %s%s",
                 employees[employee_index].first_name,
                 employees[employee_index].last_name,
                 employees[employee_index].active ? "" : " (Inativo)");
        if (focus == employee_index + 1) {
            attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
            mvhline(y, hero_left + 3, ' ', content_width + 2);
        } else if (!employees[employee_index].active) {
            attron(A_DIM);
        }
        mvaddnstr(y, hero_left + 4, full_name, content_width / 2);
        mvaddnstr(y, hero_left + 4 + content_width / 2,
                  role_names[employees[employee_index].role], content_width / 2);
        if (focus == employee_index + 1) {
            attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
        } else if (!employees[employee_index].active) {
            attroff(A_DIM);
        }
    }
}

static void draw_employee_form(int screen_height, int hero_left,
                               int hero_width, int divider_y,
                               Employee *employee,
                               int editing_existing, int focus,
                               int text_editing, const FormField fields[],
                               int field_count) {
    int index;
    int label_x = hero_left + 4;
    int value_x = hero_left + 20;
    int field_width = hero_width - 25;
    int title_y = divider_y + 1;
    int first_field_y = divider_y + 3;
    int buttons_y = screen_height - 4;

    attron(COLOR_PAIR(1) | A_BOLD);
    mvaddnstr(title_y, hero_left + 4,
              editing_existing ? "EDITAR FUNCIONARIO" : "NOVO FUNCIONARIO",
              hero_width - 8);
    attroff(COLOR_PAIR(1) | A_BOLD);
    mvhline(divider_y + 2, hero_left + 2, ACS_HLINE, hero_width - 4);

    for (index = 0; index < field_count; index++) {
        int y = first_field_y + index;
        FormField field = fields[index];
        int capacity = 0;
        char *value;
        int selected = focus == index;

        mvaddnstr(y, label_x, get_field_label(field), 15);
        if (selected) {
            attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
        } else {
            attron(COLOR_PAIR(1));
        }
        mvaddch(y, value_x, '[');
        mvhline(y, value_x + 1, ' ', field_width - 2);
        mvaddch(y, value_x + field_width - 1, ']');

        if (field == FIELD_ROLE) {
            mvaddnstr(y, value_x + 2, role_names[employee->role],
                      field_width - 4);
        } else {
            int column;
            value = get_field_value(employee, field, &capacity);
            if (field == FIELD_PASSWORD) {
                for (column = 0; column < (int) strlen(value) &&
                     column < field_width - 4; column++) {
                    mvaddch(y, value_x + 2 + column, '*');
                }
            } else {
                mvaddnstr(y, value_x + 2, value, field_width - 4);
            }
            if (selected && text_editing) {
                int cursor_x = value_x + 2 + (int) strlen(value);
                int max_x = value_x + field_width - 2;
                if (cursor_x > max_x) cursor_x = max_x;
                move(y, cursor_x);
            }
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
    mvaddnstr(buttons_y, hero_left + 4,
              editing_existing ? "[ CONFIRMAR ]" : "[ CANCELAR ]", 14);
    if (focus == field_count) {
        attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    }

    if (focus == field_count + 1) {
        attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    }
    mvaddnstr(buttons_y, hero_left + 23,
              editing_existing ? "[ EXCLUIR ]" : "[ CADASTRAR ]", 16);
    if (focus == field_count + 1) {
        attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    }
}

static void draw_choice_dialog(int hero_left, int hero_width,
                               const char *title, const char *message,
                               const char *first_option,
                               const char *second_option, int choice) {
    int center_x = hero_left + hero_width / 2;
    int option_y = 13;

    attron(COLOR_PAIR(1) | A_BOLD);
    mvaddnstr(8, hero_left + 4, title, hero_width - 8);
    attroff(COLOR_PAIR(1) | A_BOLD);
    mvaddnstr(10, hero_left + 4, message, hero_width - 8);
    if (choice == 0) attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    mvaddnstr(option_y, center_x - 14, first_option, 12);
    if (choice == 0) attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    if (choice == 1) attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    mvaddnstr(option_y, center_x + 2, second_option, 12);
    if (choice == 1) attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
}

static void draw_screen(int screen_height, int screen_width, int sidebar_width,
                        int hero_left, int hero_width, int divider_y,
                        EmployeeScreen screen, Employee employees[],
                        int employee_count, int list_focus, int action_focus,
                        int list_scroll,
                        Employee *form_employee, int form_focus,
                        int form_editing, const FormField form_fields[],
                        int form_field_count, int confirmation_focus) {
    erase();
    box(stdscr, 0, 0);
    mvvline(1, sidebar_width, ACS_VLINE, screen_height - 2);
    draw_sidebar(screen_height, sidebar_width, 2);

    if (screen == SCREEN_LIST) {
        draw_employee_list(screen_height, hero_left, hero_width, divider_y,
                           employees, employee_count, list_focus, list_scroll);
    } else if (screen == SCREEN_NEW_FORM || screen == SCREEN_EDIT_FORM) {
        draw_employee_form(screen_height, hero_left, hero_width, divider_y,
                           form_employee,
                           screen == SCREEN_EDIT_FORM, form_focus, form_editing,
                           form_fields, form_field_count);
    } else if (screen == SCREEN_ACTIONS) {
        draw_choice_dialog(hero_left, hero_width, "ACOES DO FUNCIONARIO",
                           "Escolha uma acao para o funcionario selecionado.",
                           "[ EDITAR ]", "[ INATIVAR ]", action_focus);
    } else if (screen == SCREEN_INACTIVATE_CONFIRM) {
        draw_choice_dialog(hero_left, hero_width, "INATIVAR FUNCIONARIO",
                           "Tem certeza que deseja inativar esta conta?",
                           "[ SIM ]", "[ NAO ]", confirmation_focus);
    } else if (screen == SCREEN_DELETE_CONFIRM) {
        draw_choice_dialog(hero_left, hero_width, "EXCLUIR FUNCIONARIO",
                           "Confirma a exclusao deste funcionario?",
                           "[ SIM ]", "[ NAO ]", confirmation_focus);
    }

    attron(A_DIM);
    if (screen == SCREEN_NEW_FORM || screen == SCREEN_EDIT_FORM) {
        mvaddnstr(screen_height - 2, 2,
                  form_editing ? "Digite o valor  |  Enter: concluir edicao  |  Esc: cancelar campo" :
                  "Setas: navegar  |  Enter: editar/selecionar  |  Esc: voltar",
                  screen_width - 4);
    } else {
        mvaddnstr(screen_height - 2, 2,
                  "Setas: navegar  |  Enter: selecionar  |  Esc: voltar",
                  screen_width - 4);
    }
    attroff(A_DIM);
    curs_set(form_editing ? 1 : 0);
    refresh();
}

int employee_screen(char user_id[7], char *user_profile) {
    Employee employees[MAX_EMPLOYEES] = {
        {"Ana", "Souza", "FUN-001", "Filial Centro", "", "ana@farmastock.com", "senha123", ROLE_MANAGER, 1},
        {"Carlos", "Lima", "FUN-002", "Filial Centro", "CRF-12345", "carlos@farmastock.com", "senha123", ROLE_PHARMACIST, 1},
        {"Marina", "Costa", "FUN-003", "Filial Centro", "", "marina@farmastock.com", "senha123", ROLE_CLERK, 1}
    };
    int employee_count = 3;
    int screen_height, screen_width;
    int sidebar_width, hero_left, hero_width, divider_y;
    int list_focus = 0;
    int list_scroll = 0;
    int action_focus = 0;
    int confirmation_focus = 1;
    int selected_employee = -1;
    int form_focus = 0;
    int form_editing = 0;
    int character;
    Employee form_employee;
    EmployeeScreen screen = SCREEN_LIST;

    (void) user_id;
    (void) user_profile;

    // --- VARIÁVEIS DA SIDEBAR ---
    int sidebar_focus = 2; // Começa apontando para "Funcionários"
    int is_in_sidebar = 1; // Controla se o teclado afeta o menu ou o conteúdo

    getmaxyx(stdscr, screen_height, screen_width);
    if (screen_height < MIN_SCREEN_HEIGHT || screen_width < MIN_SCREEN_WIDTH) {
        draw_resize_message(screen_height, screen_width);
        return APP_SCREEN_HOME;
    }

    sidebar_width = screen_width / 3;
    hero_left = sidebar_width + 1;
    hero_width = screen_width - hero_left - 1;
    divider_y = 1 + (screen_height - 2) / 3;
    
    keypad(stdscr, TRUE);
    clear_employee(&form_employee);

    while (1) {
        FormField visible_fields[MAX_FORM_FIELDS];
        int visible_field_count = 0;

        if (screen == SCREEN_NEW_FORM || screen == SCREEN_EDIT_FORM) {
            get_form_fields(form_employee.role, visible_fields,
                            &visible_field_count);
        }

        // 1. Desenha o conteúdo principal e o formulário
        draw_screen(screen_height, screen_width, sidebar_width, hero_left,
                    hero_width, divider_y, screen, employees, employee_count,
                    is_in_sidebar ? -1 : list_focus,
                    action_focus, list_scroll,
                    &form_employee, form_focus,
                    form_editing, visible_fields, visible_field_count,
                    confirmation_focus);

        // 2. Desenha as linhas divisórias por cima do conteúdo
        mvvline(1, sidebar_width, ACS_VLINE, screen_height - 2);
        mvhline(divider_y, hero_left, ACS_HLINE, screen_width - hero_left - 1);

        // 3. Desenha a sidebar (Envia -1 se o foco estiver na direita para tirar o destaque visual)
        draw_sidebar(screen_height, sidebar_width, is_in_sidebar ? sidebar_focus : -1);
        
        refresh(); // Atualiza a tela após todas as sobreposições

        character = getch();

        // --- INTERCEPTADOR DA SIDEBAR ---
        if (is_in_sidebar) {
            if (character == 27) { // ESC
                return APP_SCREEN_HOME;
            } else if (character == KEY_UP) {
                sidebar_focus = sidebar_focus == 0 ? 4 : sidebar_focus - 1;
            } else if (character == KEY_DOWN) {
                sidebar_focus = sidebar_focus == 4 ? 0 : sidebar_focus + 1;
            } else if (character == KEY_RIGHT) {
                is_in_sidebar = 0; // Transfere o controle do teclado para a lista de funcionários
            } else if (character == '\n' || character == KEY_ENTER) {
                if (sidebar_focus == 0) {
                    return APP_SCREEN_HOME;
                } else if (sidebar_focus == 2) {
                    is_in_sidebar = 0;
                } else if (sidebar_focus == 3) {
                    return APP_SCREEN_BRANCHES;
                }
            }
            continue; // Pula a lógica do formulário para não misturar os controles
        }

        // --- MÁQUINA DE ESTADOS DO CONTEÚDO ---
        if (screen == SCREEN_LIST) {
            int visible_count = (screen_height - divider_y - 9) / 2;
            int last_focus = employee_count;
            
            // Seta Esquerda ou ESC devolvem o usuário para a Sidebar
            if (character == KEY_LEFT || character == 27) {
                is_in_sidebar = 1;
                continue;
            }

            if (character == KEY_UP) {
                list_focus = list_focus == 0 ? last_focus : list_focus - 1;
            } else if (character == KEY_DOWN) {
                list_focus = list_focus == last_focus ? 0 : list_focus + 1;
            } else if (character == KEY_RIGHT) {
                list_focus = list_focus == 0 ? last_focus : 0;
            } else if ((character == '\n' || character == KEY_ENTER) &&
                       list_focus == 0) {
                clear_employee(&form_employee);
                form_focus = 0;
                form_editing = 0;
                screen = SCREEN_NEW_FORM;
            } else if ((character == '\n' || character == KEY_ENTER) &&
                       list_focus > 0 && list_focus <= employee_count &&
                       employees[list_focus - 1].active) {
                selected_employee = list_focus - 1;
                action_focus = 0;
                screen = SCREEN_ACTIONS;
            }
            
            if (list_focus > list_scroll + visible_count) {
                list_scroll = list_focus - visible_count;
            } else if (list_focus > 0 && list_focus <= list_scroll) {
                list_scroll = list_focus - 1;
            }
        } else if (screen == SCREEN_ACTIONS) {
            if (character == KEY_UP || character == KEY_DOWN ||
                character == KEY_LEFT || character == KEY_RIGHT) {
                action_focus = !action_focus;
            } else if (character == '\n' || character == KEY_ENTER) {
                if (action_focus == 0) {
                    form_employee = employees[selected_employee];
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
        } else if (screen == SCREEN_INACTIVATE_CONFIRM ||
                   screen == SCREEN_DELETE_CONFIRM) {
            if (character == KEY_LEFT || character == KEY_RIGHT ||
                character == KEY_UP || character == KEY_DOWN) {
                confirmation_focus = !confirmation_focus;
            } else if (character == '\n' || character == KEY_ENTER) {
                if (confirmation_focus == 0) {
                    int deleting = screen == SCREEN_DELETE_CONFIRM;
                    if (selected_employee >= 0 && deleting) {
                        int index;
                        for (index = selected_employee;
                             index < employee_count - 1; index++) {
                            employees[index] = employees[index + 1];
                        }
                        employee_count--;
                        list_focus = employee_count > 0 ? employee_count : 0;
                        selected_employee = -1;
                    } else if (selected_employee >= 0) {
                        employees[selected_employee].active = 0;
                    }
                    screen = SCREEN_LIST;
                    if (list_focus > employee_count) list_focus = employee_count;
                    if (list_scroll > 0 && list_scroll >= employee_count)
                        list_scroll = employee_count > 0 ? employee_count - 1 : 0;
                } else {
                    screen = screen == SCREEN_DELETE_CONFIRM ?
                             SCREEN_EDIT_FORM : SCREEN_LIST;
                }
            } else if (character == 27) {
                screen = screen == SCREEN_DELETE_CONFIRM ?
                         SCREEN_EDIT_FORM : SCREEN_ACTIONS;
            }
        } else {
            FormField fields[MAX_FORM_FIELDS];
            int field_count = 0;
            int field_capacity = 0;
            char *field_value = NULL;
            int is_new = screen == SCREEN_NEW_FORM;
            get_form_fields(form_employee.role, fields, &field_count);

            if (form_editing) {
                field_value = get_field_value(&form_employee,
                                             fields[form_focus],
                                             &field_capacity);
                if (character == '\n' || character == KEY_ENTER ||
                    character == 27) {
                    form_editing = 0;
                } else if (field_value != NULL &&
                           (character == KEY_BACKSPACE || character == 127 ||
                            character == 8)) {
                    int length = (int) strlen(field_value);
                    if (length > 0) field_value[length - 1] = '\0';
                } else if (field_value != NULL && character >= 32 &&
                           character <= 255 &&
                           strlen(field_value) < (size_t) field_capacity - 1) {
                    int length = (int) strlen(field_value);
                    field_value[length] = (char) character;
                    field_value[length + 1] = '\0';
                }
            } else if (character == KEY_UP) {
                form_focus = form_focus == 0 ? field_count + 1 : form_focus - 1;
            } else if (character == KEY_DOWN) {
                form_focus = (form_focus + 1) % (field_count + 2);
            } else if (character == KEY_LEFT || character == KEY_RIGHT) {
                if (form_focus == 0) {
                    int direction = character == KEY_RIGHT ? 1 : -1;
                    form_employee.role = (EmployeeRole)
                        ((form_employee.role + direction + ROLE_COUNT) % ROLE_COUNT);
                    form_focus = 0;
                } else if (form_focus >= field_count) {
                    form_focus = form_focus == field_count ?
                                 field_count + 1 : field_count;
                }
            } else if (character == '\n' || character == KEY_ENTER) {
                if (form_focus < field_count && fields[form_focus] == FIELD_ROLE) {
                    form_employee.role = (EmployeeRole)
                        ((form_employee.role + 1) % ROLE_COUNT);
                } else if (form_focus < field_count) {
                    form_editing = 1;
                } else if (form_focus == field_count) {
                    if (is_new) {
                        screen = SCREEN_LIST;
                    } else {
                        employees[selected_employee] = form_employee;
                        screen = SCREEN_LIST;
                    }
                } else if (is_new && employee_count < MAX_EMPLOYEES) {
                    employees[employee_count++] = form_employee;
                    list_focus = employee_count;
                    screen = SCREEN_LIST;
                } else if (!is_new) {
                    confirmation_focus = 1;
                    screen = SCREEN_DELETE_CONFIRM;
                }
            } else if (character == 27) {
                screen = SCREEN_LIST;
            }

            if (form_focus > field_count + 1) form_focus = field_count + 1;
        }
    }
}
