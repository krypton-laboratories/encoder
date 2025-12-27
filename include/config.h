#pragma once

#include <stdio.h>

typedef struct {
    int accept_threshold;
    char url[128];
    int test_port;
    int payload_tests;
} config_t;

int read_config(FILE *file, config_t *config);
