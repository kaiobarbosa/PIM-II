#include <ncurses.h>
#include <stdio.h>
#include <string.h>

// Importações dos componentes visuais
#include "frontend/components/sidebar.h"
#include "frontend/screens/home_screen.h" // Para usar a função genérica draw_resize_message

#define MIN_SCREEN_HEIGHT 24
#define MIN_SCREEN_WIDTH 70

void branch_screen(void) {
    int screen_height;
    int screen_width;
    int sidebar_width;
    int hero_left;
    int hero_width;
    int character;

    // Variáveis de controle de navegação
    int sidebar_focus = 3; // Ajuste este índice se "Filiais" tiver uma posição específica no seu sidebar.c
    int is_in_sidebar = 1; // 1 = Teclado controla a sidebar | 0 = Teclado controla o conteúdo principal

    getmaxyx(stdscr, screen_height, screen_width);
    if (screen_height < MIN_SCREEN_HEIGHT || screen_width < MIN_SCREEN_WIDTH) {
        draw_resize_message(screen_height, screen_width);
        return;
    }

    // Delimitações de layout (1/3 para sidebar, 2/3 para conteúdo)
    sidebar_width = screen_width / 3;
    hero_left = sidebar_width + 1;
    hero_width = screen_width - hero_left - 1;

    keypad(stdscr, TRUE);

    while (1) {
        erase();
        box(stdscr, 0, 0);

        // Desenha a linha vertical separadora
        mvvline(1, sidebar_width, ACS_VLINE, screen_height - 2);

        // Desenha a sidebar (-1 remove o fundo branco se o usuário pular para a área de cadastro)
        draw_sidebar(screen_height, sidebar_width, is_in_sidebar ? sidebar_focus : -1);

        // --- INÍCIO DA ÁREA ÚTIL (HERO) ---
        attron(COLOR_PAIR(1) | A_BOLD);
        mvaddnstr(2, hero_left + 4, "CADASTRO DE NOVA FILIAL", hero_width - 8);
        attroff(COLOR_PAIR(1) | A_BOLD);
        
        mvhline(4, hero_left + 2, ACS_HLINE, hero_width - 4);
        
        // Simulação de um botão de ação no conteúdo principal
        if (!is_in_sidebar) {
            attron(COLOR_PAIR(2) | A_REVERSE);
            mvaddnstr(6, hero_left + 4, "[ Preencher Dados da Filial ]", hero_width - 8);
            attroff(COLOR_PAIR(2) | A_REVERSE);
        } else {
            mvaddnstr(6, hero_left + 4, "[ Preencher Dados da Filial ]", hero_width - 8);
        }

        // Rodapé de instruções dinâmicas
        attron(A_DIM);
        mvaddnstr(screen_height - 2, hero_left + 4, 
                  "Setas: mover | Enter: selecionar | Esc: voltar", hero_width - 8);
        attroff(A_DIM);
        // ----------------------------------

        refresh();

        character = getch();

        // --- MÁQUINA DE ESTADOS DO TECLADO ---
        if (is_in_sidebar) {
            if (character == 27) { // ESC
                return; // Volta para a tela anterior
            } else if (character == KEY_UP) {
                sidebar_focus = sidebar_focus == 0 ? 4 : sidebar_focus - 1;
            } else if (character == KEY_DOWN) {
                sidebar_focus = sidebar_focus == 4 ? 0 : sidebar_focus + 1;
            } else if (character == KEY_RIGHT) {
                is_in_sidebar = 0; // Transfere o foco para o formulário de filiais
            } else if (character == '\n' || character == KEY_ENTER) {
                if (sidebar_focus == 0) { // Clicou no botão Home (índice 0)
                    return; 
                }
            }
        } else {
            // Foco na área de conteúdo (Cadastro da Filial)
            if (character == KEY_LEFT || character == 27) {
                is_in_sidebar = 1; // Seta Esquerda ou ESC devolve o foco para a sidebar
            } else if (character == '\n' || character == KEY_ENTER) {
                // Aqui você integrará o formulário interativo de cadastro futuramente
            }
        }
    }
}
