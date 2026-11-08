#include <stdio.h>
#include <string.h>
#include <ncurses.h>

void draw_sidebar(int screen_height, int sidebar_width, int focus) {
    // 1. Adicionamos "Filiais" na lista (agora possui 5 itens)
    static const char *menu_items[] = {
        "Home", "Produtos", "Funcionarios", "Filiais", "Perfil"
    };
    int item;

    attron(COLOR_PAIR(1) | A_BOLD);
    mvaddnstr(2, 3, "FARMASTOCK", sidebar_width - 5);
    attroff(COLOR_PAIR(1) | A_BOLD);
    mvhline(4, 2, ACS_HLINE, sidebar_width - 4);

    // 2. Aumentamos o limite do loop para 5 (item < 5)
    for (item = 0; item < 5; item++) {
        int row = 8 + item * 3;
        if (focus == item) {
            attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
        }
        mvaddnstr(row, 3, menu_items[item], sidebar_width - 5);
        if (focus == item) {
            attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
        }
    }

    mvhline(screen_height - 5, 2, ACS_HLINE, sidebar_width - 4);
    attron(A_DIM);
    mvaddnstr(screen_height - 4, 3, "Usuario conectado", sidebar_width - 5);
    mvaddnstr(screen_height - 3, 3, "Sessao ativa", sidebar_width - 5);
    attroff(A_DIM);
}

void draw_profile_sidebar(int screen_height, int sidebar_width,
                          const char *first_name, const char *last_name,
                          const char *profile,
                          const char *internal_code, const char *crf,
                          const char *branch, const char *email,
                          int password_info_visible, int focus, int editing) {
    int content_width = sidebar_width - 5;
    int row = 7;
    char full_name[102];

    snprintf(full_name, sizeof(full_name), "%s %s", first_name, last_name);

    erase();
    box(stdscr, 0, 0);
    mvvline(1, sidebar_width, ACS_VLINE, screen_height - 2);

    attron(COLOR_PAIR(1) | A_BOLD);
    mvaddnstr(2, 3, "Ola,", content_width);
    mvaddnstr(3, 3, full_name, content_width);
    attroff(COLOR_PAIR(1) | A_BOLD);
    mvhline(5, 2, ACS_HLINE, sidebar_width - 4);

    attron(A_DIM);
    mvaddnstr(6, 3, profile, content_width);
    attroff(A_DIM);

#define DRAW_PROFILE_FIELD(label, value) \
    do { \
        if ((value)[0] != '\0' && row < screen_height - 9) { \
            char field_line[160]; \
            snprintf(field_line, sizeof(field_line), "%s: %s", label, value); \
            mvaddnstr(row, 3, field_line, content_width); \
            row += 2; \
        } \
    } while (0)
    DRAW_PROFILE_FIELD("Codigo interno", internal_code);
    DRAW_PROFILE_FIELD("CRF", crf);
    DRAW_PROFILE_FIELD("Filial", branch);
    DRAW_PROFILE_FIELD("E-mail", email);
#undef DRAW_PROFILE_FIELD

    if (row < screen_height - 8) {
        char password_line[80];
        snprintf(password_line, sizeof(password_line), "Senha: %s",
                 password_info_visible ? "hash protegido" : "********");
        mvaddnstr(row++, 3, password_line, content_width);
        if (focus == 0 && !editing) attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
        mvaddnstr(row, 3,
                  password_info_visible ? "[ ocultar detalhes ]" : "[ exibir senha ]",
                  content_width);
        if (focus == 0 && !editing) attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    }

    mvhline(screen_height - 6, 2, ACS_HLINE, sidebar_width - 4);
    if (focus == 1 && !editing) attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    mvaddnstr(screen_height - 5, 3, editing ? "Editando perfil" : "Editar",
              content_width);
    if (focus == 1 && !editing) attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    if (focus == 2 && !editing) attron(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    mvaddnstr(screen_height - 3, 3, "Voltar", content_width);
    if (focus == 2 && !editing) attroff(COLOR_PAIR(2) | A_BOLD | A_REVERSE);
}
