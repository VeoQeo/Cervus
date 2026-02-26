#include <stdint.h>
int __unorddf2(double a, double b) {
    uint64_t au, bu;
    __builtin_memcpy(&au, &a, 8);
    __builtin_memcpy(&bu, &b, 8);
    int a_nan = ((au & 0x7FF0000000000000ULL) == 0x7FF0000000000000ULL) && (au & 0x000FFFFFFFFFFFFFULL);
    int b_nan = ((bu & 0x7FF0000000000000ULL) == 0x7FF0000000000000ULL) && (bu & 0x000FFFFFFFFFFFFFULL);
    return a_nan || b_nan;
}
