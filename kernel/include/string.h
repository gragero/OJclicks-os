#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdint.h>

size_t k_strlen(const char* str);
int k_strcmp(const char* a, const char* b);
void k_strcpy(char* dest, const char* src);
void* k_memset(void* ptr, int value, size_t num);
void* k_memcpy(void* dest, const void* src, size_t num);
int k_strncmp(const char* a, const char* b, size_t n);

#endif
