#ifndef UNICODE_INPUT_H
#define UNICODE_INPUT_H

#include <stddef.h>

int read_input_character(void);
/* Aguarda ate que o terminal tenha pelo menos 80 colunas por 34 linhas. */
int wait_for_minimum_terminal_size(void);
int input_character_is_printable(int character);
int input_character_display_width(int character);
int input_string_display_width(const char *text);
int input_string_character_count(const char *text);
int input_character_append(char *buffer, size_t capacity, size_t *length,
                           int character);
void input_backspace(char *buffer);
int mvadd_utf8_clipped(int y, int x, const char *text, int max_columns);

#endif
