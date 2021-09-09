#include "math.h"
#include "types.h"

double ceil(double n) {
    if (n == (int) n) {
        return n;
    } else {
        return (double)((int) n + 1);
    }
}

double floor(double n) {
    return (float)(int) n;
}

uint32_t pow(uint32_t base, uint32_t exp) {
    uint32_t ret = base;

    if (exp == 0) {
        return 1;
    }

    for (int i = 0; i < exp - 1; i++) {
        ret *= base;
    }

    return ret;
}