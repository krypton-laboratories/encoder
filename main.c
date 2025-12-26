#include <stdio.h>
#include <limits.h>
#include "config.h"

_Static_assert(CHAR_BIT == 8, "Unsupported platform: byte sizes must be exactly 8 bits.");

int main(void) {
    printf("kryptographer v0.1\n");
    printf("reading config file /etc/kryptographer/config.cfg\n");

    FILE *fp = fopen("/etc/kryptographer/config.cfg", "r");
    if (!fp) {
        perror("Error opening file");
        return 1;
    }

    config_t config = {0};

    read_config(fp, &config);
    fclose(fp);

    printf("acceptance threshold: %d, server url: %s\n", config.accept_threshold, config.url);

    return 0;
}
