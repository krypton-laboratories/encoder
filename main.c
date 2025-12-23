#include <stdio.h>
#include <stdlib.h>
#include "obfs.h"

int main(void) {
    substitution_t *buf = malloc(sizeof(uint8_t) * 256);
    gen_mapping(buf);

    for (int i = 0; i < 256; i++) {
        // Verify that there are no repeats
        printf("%d ", (int)buf[i]);
    }

    free(buf);
    return 0;
}
