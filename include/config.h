#pragma once

#include <stdio.h>

typedef struct {
    int accept_threshold;
    char url[128];
} config_t;

int read_config(FILE *file, config_t *config);
