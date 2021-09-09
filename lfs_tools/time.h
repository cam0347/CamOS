/*
Zavattaro Camillo
03/05/2021

Time and date structures and functions
*/

#define TIMEH 1

#ifndef TYPESH
    #include "types.h"
#endif

//date structure (4 byte)
struct date_t {
    uint16_t y;
    uint8_t m;
    uint8_t d;
};

//time structure (4 byte)
struct time_t {
    uint8_t h;
    uint8_t m;
    uint8_t s;
    uint8_t h24;
};

//full date and time structure (8 byte)
struct full_date_t {
    struct date_t date;
    struct time_t time;
};