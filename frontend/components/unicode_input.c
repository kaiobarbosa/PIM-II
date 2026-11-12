#define _XOPEN_SOURCE_EXTENDED 1

#include "frontend/components/unicode_input.h"

#include <limits.h>
#include <ncurses.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

#define UNICODE_INPUT_BASE 0x200000

static wchar_t input_codepoint(int character) {
    if (character >= UNICODE_INPUT_BASE)
        return (wchar_t)(character - UNICODE_INPUT_BASE);
    return (wchar_t)character;
}

int read_input_character(void) {
    wint_t character = 0;
    int result = get_wch(&character);

    if (result == ERR) return ERR;
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
