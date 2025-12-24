#pragma once

#include <stddef.h>
#include <stdint.h>

#define MTU 1420

typedef enum { FORWARD, INVERSE } direction_t;
typedef uint8_t substitution_t;
typedef unsigned int permutation_t;

void gen_mapping(substitution_t *dest);

int substitute_bytes(
    const uint8_t *src,
    uint8_t *dest,
    const substitution_t *mapping,
    const size_t size,
    direction_t direction
);

int permute_bytes(
    const uint8_t *src,
    uint8_t *dest,
    const permutation_t key,
    const size_t size,
    direction_t direction
);


int apply(
  const uint8_t *src,
  uint8_t *dest,
  const size_t size,
  const substitution_t *substitution,
  const permutation_t *permutation,
  direction_t direction
);
