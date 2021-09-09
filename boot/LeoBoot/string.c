#include "string.h"
#include "types.h"
#include "mem.h"
#include "tty.h"
#include "com.h"
#include "math.h"

//catenate src to dest, returns the pointer to destination
void strcat(char *dest, const char *src) {
    dest += strlen(dest); //move the pointer at the end of the string

    for (uint32_t i = 0; i < strlen(src); i++) {
        *(dest + i) = *(src + i);
    }
}

//copy src string into dest
void strcpy(char *dest, const char *src) {
    uint32_t i = 0;
    
    while (*(src + i) != 0x0) {
        *(dest + i) = *(src + i);
        i++;
    }
}

//returns the length of the string str (terminator is not included in the output)
uint32_t strlen(const char *str) {
    uint32_t i = 0;

    while(*(str + i) != 0x00) {
        i++;
    }

    return i;
}

uint32_t strcmp(const char *str1, const char *str2) {
    uint32_t str1_l = strlen(str1);
    uint32_t str2_l = strlen(str2);

    if (str1_l != str2_l) {
        return -1;
    }

    for (uint32_t i = 0; i < str1_l; i++) {
        if (*(str1 + i) != *(str2 + i)) {
            return -1;
        }
    }

    return 0;
}

//returns the first substring starting with the token or null if there's an error
char *strtok(const char *str, const char *token) {
    uint32_t tok_l = strlen(token);
    uint32_t str_l = strlen(str);
    char buffer[tok_l + 1];
    buffer[tok_l] = 0x00;

    if (tok_l == 0 || str_l == 0) {
        return null;
    }

    if (str_l < tok_l) {
        return null;
    } else if (str_l == tok_l) {
        return (char *) str;
    }

    for (uint32_t i = 0; i < str_l - tok_l; i++) {
        memcpy(buffer, (void *)(str + i), tok_l);

        if (strcmp(buffer, token) == STR_EQUAL) {
            return (char *)(str + i);
        }
    }

    return null;
}

uint32_t scan_s(char *buffer) {
    uint32_t i = 0;
    char c;

    do {
        c = getc();
        *(buffer + i) = c;
        i++;
    } while (c != '\n');

    *(buffer + i - 1) = 0x00;

    return i;
}

uint32_t scan_s_limit(char *buffer, uint32_t limit) {
    uint32_t i = 0;
    char c;

    do {
        c = getc();
        *(buffer + i) = c;
        i++;
    } while (c != '\n' && i < limit);

    *(buffer + i - 1) = 0x00;

    return i;
}

int scan_int() {
    char string[11];
    scan_s_limit(string, 10) - 1;
    return stoi(string);
}

//convert string to signed integer
int stoi(char *string) {
    int ret = 0;
    uint32_t length = strlen(string);
    bool neg = false;

    if (*string == '-') {
        neg = true;
        memmove(string, string + 1, length);
        length--;
    }

    for (uint32_t i = 0; i < length; i++) {
        if (*(string + i) >= 48 && *(string + i) <= 57) {
            ret += (*(string + length - 1 - i) - 48) * pow(10, i);
        } else {
            error("invalid number");
            return 0;
        }
    }

    if (neg) {
        ret *= -1;
    }

    return ret;
}