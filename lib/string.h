/*
Zavattaro Camillo
30/04/2021

String utilities
*/

#define STRINGH 1

void strcat(char *dest, char *src);
long strlen(char *str);
int strcmp(char *str1, char *str2);
char *int_to_string(int integer);
long strcpy(char *dest, char *src);

//catenate src to dest, returns the pointer to destination
void strcat(char *dest, char *src) {
    dest += strlen(dest); //move the pointer at the end of the string

    for (int i = 0; i < strlen(src); i++) {
        *(dest + i) = *(src + i);
    }
}

//returns the length of the string str
long strlen(char *str) {
    long i = 0;

    while (*(str + i) != 0x0) {
        i++;
    }

    return i;
}

//copy src string into dest
long strcpy(char *dest, char *src) {
    long i = 0;
    
    while (*(src + i) != 0x0) {
        *(dest + i) = *(src + i);
        i++;
    }

    return i;
}

int strcmp(char *str1, char *str2) {
    int str1_l = strlen(str1);
    int str2_l = strlen(str2);

    if (str1_l != str2_l) {
        return -1;
    }

    for (long i = 0; i < str1_l; i++) {
        if (*(str1 + i) != *(str2 + i)) {
            return -1;
        }
    }

    return 0;
}

/*char *int_to_string(int integer) {
    uint8_t negative;

    if (integer < 0) {
        negative = 1;
        integer *= -1;
    } else {
        negative = 0;
    }
}*/