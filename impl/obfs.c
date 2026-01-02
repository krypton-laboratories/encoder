#include "obfs.h"

#include <stdlib.h>
#include <string.h>

void gen_mapping(substitution_t *dest) {
    for (int i = 0; i < 256; i++) {
        dest[i] = (uint8_t)i;
    }

    for (size_t i = 0; i < 255; i++) {
        size_t j = i + rand() / (RAND_MAX / (256 - i) + 1);
        
        uint8_t t = dest[j];
        dest[j] = dest[i];
        dest[i] = t;
    }
}

static inline int indexof(const substitution_t *buf, uint8_t byte) {
    for (int i = 0; i < 256; i++) {
        if (buf[i] == byte) {
            return i;
        }
    }
    return -1;
}

int substitute_bytes(const uint8_t *src, uint8_t *dest,
                     const substitution_t *mapping, const size_t size,
                     direction_t direction) {
    if (direction == FORWARD) {
        for (size_t i = 0; i < size; i++) {
            dest[i] = mapping[src[i]];
        }
    } else {
        for (size_t i = 0; i < size; i++) {
            dest[i] = indexof(mapping, src[i]);
        }
    }
    return 0;
}

static inline unsigned int modinv(unsigned int a, unsigned int n) {
    long t = 0, newT = 1;
    long r = n, newR = a;

    while (newR != 0) {
        long quotient = r / newR;
        long oldT = t, oldR = r;
        t = newT;
        newT = oldT - quotient * newT;

        r = newR;
        newR = oldR - quotient * newR;

    }

    if (r != 1) {
        return -1;
    }
    if (t < 0) {
        t += n;
    }
    return t % n;
}

int permute_bytes(const uint8_t *src, uint8_t *dest,
                  const permutation_t key, const size_t size,
                  direction_t direction) {
    unsigned int n = size;
    unsigned int a = (key * 2 + 1) % n;
    if (a == 0) a = 1;
    unsigned int b = key % n;
    if (direction == FORWARD) {
        for (unsigned int i = 0; i < n; i++) {
            unsigned int j = ((unsigned long long)a * i + b) % n; 
            dest[j] = src[i];
        }
    } else {
        unsigned int a_inv = modinv(a, n);

        for (unsigned int j = 0; j < n; j++) {
            unsigned int i = (a_inv * ((j + n - b) % n)) % n;
            dest[i] = src[j];
        }
    }

    return 0; 
}

int apply(const uint8_t *src, uint8_t *dest, const size_t size,
          const substitution_t *substitution, const permutation_t *permutation,
          direction_t direction) {
    uint8_t *temp = malloc(sizeof(uint8_t) * size);

    if (direction == FORWARD) {
        substitute_bytes(src, temp, substitution, size, direction);
        permute_bytes(temp, dest, *permutation, size, direction);
    } else {
        permute_bytes(src, temp, *permutation, size, direction);
        substitute_bytes(temp, dest, substitution, size, direction);
    }

    free(temp);
    return 0;
}
