#define _XOPEN_SOURCE_EXTENDED 1

#include "frontend/components/unicode_input.h"

#include <limits.h>
#include <ncurses.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>

#define UNICODE_INPUT_BASE 0x200000
#define MIN_TERMINAL_ROWS 34
#define MIN_TERMINAL_COLUMNS 80

static int synchronize_terminal_size(void) {
    struct winsize terminal_size;
    int rows;
    int columns;

    getmaxyx(stdscr, rows, columns);
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &terminal_size) == 0 &&
        terminal_size.ws_row > 0 && terminal_size.ws_col > 0 &&
        (terminal_size.ws_row != rows || terminal_size.ws_col != columns)) {
        resizeterm(terminal_size.ws_row, terminal_size.ws_col);
        return 1;
    }
    return 0;
}

static void draw_terminal_resize_message(int rows, int columns) {
    const char *message = "Aumente o terminal para continuar";
    const char *required_size = "Tamanho minimo: 80 colunas x 34 linhas";

    erase();
    if (rows > 1 && columns > 1) box(stdscr, 0, 0);
    if (columns > (int)strlen(message) + 2 && rows > 2) {
        mvaddstr(rows / 2 - 1, (columns - (int)strlen(message)) / 2,
                 message);
    }
    if (columns > (int)strlen(required_size) + 2 && rows > 3) {
        mvaddstr(rows / 2 + 1, (columns - (int)strlen(required_size)) / 2,
                 required_size);
    }
    refresh();
}

int wait_for_minimum_terminal_size(void) {
    int rows;
    int columns;
    int previous_rows = -1;
    int previous_columns = -1;
    int waited = 0;

    while (1) {
        wint_t ignored_character;
        int result;

        synchronize_terminal_size();
        getmaxyx(stdscr, rows, columns);
        if (rows >= MIN_TERMINAL_ROWS &&
            columns >= MIN_TERMINAL_COLUMNS) {
            if (waited) {
                clear();
                refresh();
            }
            return waited;
        }

        if (rows != previous_rows || columns != previous_columns) {
            draw_terminal_resize_message(rows, columns);
            previous_rows = rows;
            previous_columns = columns;
        }
        waited = 1;
        wtimeout(stdscr, 200);
        result = get_wch(&ignored_character);
        wtimeout(stdscr, -1);
        if (result == KEY_CODE_YES && ignored_character == KEY_RESIZE)
            resizeterm(0, 0);
    }
}

static wchar_t input_codepoint(int character) {
    if (character >= UNICODE_INPUT_BASE)
        return (wchar_t)(character - UNICODE_INPUT_BASE);
    return (wchar_t)character;
}

int read_input_character(void) {
    wint_t character = 0;
    int result;

    if (wait_for_minimum_terminal_size()) return KEY_RESIZE;

    while (1) {
        wtimeout(stdscr, 200);
        result = get_wch(&character);
        wtimeout(stdscr, -1);

        if (result == KEY_CODE_YES && character == KEY_RESIZE) {
            synchronize_terminal_size();
            wait_for_minimum_terminal_size();
            return KEY_RESIZE;
        }
        if (result == ERR) {
            if (synchronize_terminal_size()) {
                wait_for_minimum_terminal_size();
                return KEY_RESIZE;
            }
            if (wait_for_minimum_terminal_size()) return KEY_RESIZE;
            continue;
        }
        if (synchronize_terminal_size()) {
            wait_for_minimum_terminal_size();
            return KEY_RESIZE;
        }
        break;
    }

    if (result == KEY_CODE_YES) return (int)character;
    if (character > 0x7f)
        return UNICODE_INPUT_BASE + (int)character;
    return (int)character;
}

int input_character_is_printable(int character) {
    if (character < 0 ||
        (character > 0x7f && character < UNICODE_INPUT_BASE) ||
        character >= UNICODE_INPUT_BASE + 0x110000)
        return 0;
    return iswprint((wint_t)input_codepoint(character)) != 0;
}

int input_character_display_width(int character) {
    int width;
    if (!input_character_is_printable(character)) return -1;
    width = wcwidth(input_codepoint(character));
    return width > 0 ? width : 0;
}

int input_string_display_width(const char *text) {
    mbstate_t state = {0};
    const char *cursor = text;
    int width = 0;

    if (text == NULL) return 0;
    while (*cursor != '\0') {
        wchar_t character;
        size_t length = mbrtowc(&character, cursor, strlen(cursor), &state);
        int character_width;
        if (length == (size_t)-1 || length == (size_t)-2 || length == 0)
            break;
        character_width = wcwidth(character);
        if (character_width > 0) width += character_width;
        cursor += length;
    }
    return width;
}

int input_string_character_count(const char *text) {
    mbstate_t state = {0};
    const char *cursor = text;
    int count = 0;

    if (text == NULL) return 0;
    while (*cursor != '\0') {
        wchar_t character;
        size_t length = mbrtowc(&character, cursor, strlen(cursor), &state);
        if (length == (size_t)-1 || length == (size_t)-2 || length == 0)
            break;
        count++;
        cursor += length;
    }
    return count;
}

int input_character_append(char *buffer, size_t capacity, size_t *length,
                           int character) {
    wchar_t codepoint = input_codepoint(character);
    char encoded[MB_LEN_MAX];
    mbstate_t state = {0};
    size_t encoded_length;

    if (buffer == NULL || length == NULL ||
        !input_character_is_printable(character))
        return 0;

    encoded_length = wcrtomb(encoded, codepoint, &state);
    if (encoded_length == (size_t)-1 || *length + encoded_length >= capacity)
        return 0;

    memcpy(buffer + *length, encoded, encoded_length);
    *length += encoded_length;
    buffer[*length] = '\0';
    return 1;
}

int mvadd_utf8_clipped(int y, int x, const char *text, int max_columns) {
    wchar_t wide_text[1024];
    const char *source = text;
    mbstate_t state = {0};
    size_t character_count;
    size_t visible_count = 0;
    int display_width = 0;

    if (text == NULL || max_columns <= 0) return OK;
    character_count = mbsrtowcs(wide_text, &source,
                                sizeof(wide_text) / sizeof(wide_text[0]) - 1,
                                &state);
    if (character_count == (size_t)-1) return ERR;

    while (visible_count < character_count) {
        int character_width = wcwidth(wide_text[visible_count]);
        if (character_width < 0 ||
            display_width + character_width > max_columns)
            break;
        display_width += character_width;
        visible_count++;
    }
    wide_text[visible_count] = L'\0';
    return mvaddwstr(y, x, wide_text);
}

void input_backspace(char *buffer) {
    size_t length;

    if (buffer == NULL) return;
    length = strlen(buffer);
    if (length == 0) return;

    length--;
    while (length > 0 &&
           ((unsigned char)buffer[length] & 0xc0) == 0x80) {
        length--;
    }
    buffer[length] = '\0';
}
