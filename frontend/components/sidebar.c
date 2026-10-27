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
