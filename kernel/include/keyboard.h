#ifndef KEYBOARD_H
#define KEYBOARD_H

void keyboard_install(void);
char keyboard_getchar(void);
void keyboard_read_line(char* buffer, int max_len);
int keyboard_has_key(void);
char keyboard_get_char(void);

#endif
