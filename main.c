#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "obfs.h"

_Static_assert(CHAR_BIT == 8, "Unsupported platform: byte sizes must be exactly 8 bits.");

void print_buf(uint8_t *buf, size_t size) {
    for (int i = 0; i < (int)size; i++) {
        printf("%d ", (int)buf[i]);
    }
    printf("\n");
}
void check_repeats_and_missing(
    const uint8_t *buf,
    size_t len,
    uint8_t expected_min,
    uint8_t expected_max
) {
    uint16_t counts[256];  // uint16_t to safely count repeats
    memset(counts, 0, sizeof(counts));

    // Count occurrences
    for (size_t i = 0; i < len; i++) {
        counts[buf[i]]++;
    }

    // Check for repeats
    printf("Repeated values:\n");
    int found_repeat = 0;
    for (uint16_t v = 0; v <= 255; v++) {
        if (counts[v] > 1) {
            printf("  %u (count=%u)\n", v, counts[v]);
            found_repeat = 1;
        }
    }
    if (!found_repeat) {
        printf("  none\n");
    }

    // Check for missing values
    printf("Missing values:\n");
    int found_missing = 0;
    for (uint16_t v = expected_min; v <= expected_max; v++) {
        if (counts[v] == 0) {
            printf("  %u\n", v);
            found_missing = 1;
        }
    }
    if (!found_missing) {
        printf("  none\n");
    }
}


int main(void) {
    substitution_t *buf = malloc(sizeof(uint8_t) * 256);
    gen_mapping(buf);

    check_repeats_and_missing(buf, 256, 0, 255);

    const char *hi = "Hello, world!";
    const int len = 13;
    uint8_t *original = (uint8_t *)hi;

    printf("Original: ");
    print_buf(original, len);

    printf("Scrambled: ");
    uint8_t *scrambled = malloc(len * sizeof(uint8_t));
    unsigned int perm = 5;
    apply(original, scrambled, len, buf, &perm, FORWARD);
    print_buf(scrambled, len);

    printf("Unscrambled: ");
    uint8_t *unscramble = malloc(len * sizeof(uint8_t));
    apply(scrambled, unscramble, len, buf, &perm, INVERSE);
    print_buf(unscramble, len);

    free(buf);
    free(scrambled);
    free(unscramble);
    return 0;
}
