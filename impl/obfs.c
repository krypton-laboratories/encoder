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

static inline uint32_t xorshift32(uint32_t *state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

int permute_bytes(const uint8_t *src, uint8_t *dest,
                  const permutation_t key, const size_t size,
                  direction_t direction) {
    memcpy(dest, src, size);
    permutation_t rng = key;
    if (direction == FORWARD) {
        for (unsigned int i = size - 1; i > 0; i--) {
            unsigned int j = xorshift32(&rng) % (i + 1); 
            int tmp = dest[i];
            dest[i] = dest[j];
            dest[j] = tmp;
        }
    } else {
        uint32_t *swap_store = malloc(sizeof(uint32_t) * size);
        for (size_t i = size - 1; i > 0; i--) {
            swap_store[size - 1 - i] = xorshift32(&rng) % (i + 1);
        }
        for (size_t i = 1; i < size; i++) {
            size_t j = swap_store[i - 1];
            int tmp = dest[i];
            dest[i] = dest[j];
            dest[j] = tmp;
        }

        free(swap_store);
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
