#include <stdio.h>
#include <ncurses.h>
#include <string.h>

void create_account_screen() {
   
    initscr();

    noecho();
    curs_set(0);

    start_color();
    use_default_colors();
    init_pair(1, COLOR_CYAN, -1);
    init_pair(2, COLOR_GREEN, -1);
    
    box(stdscr, 0, 0);

    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(2, 4, "FarmaStock | ESTOQUE UNIFICADO");
    attroff(COLOR_PAIR(1) | A_BOLD);

    refresh();
    getch();
    endwin();

}