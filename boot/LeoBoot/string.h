#pragma once

#include "types.h"
#define STR_EQUAL 0
#define STR_DIFF  1

void strcat(char *dest, const char *src);
void strcpy(char *dest, const char *src);
uint32_t strlen(const char *str);
uint32_t strcmp(const char *str1, const char *str2);
char *strtok(const char *str, const char *token);
uint32_t scan_s(char *buffer);
uint32_t scan_s_limit(char *buffer, uint32_t limit);
int scan_int();
int stoi(char *string);