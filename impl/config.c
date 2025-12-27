#include "config.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

void trim(char *str) {
    char *start = str;
    while (isspace((unsigned char)*start)) start++;
    memmove(str, start, strlen(start) + 1);

    char *end = str + strlen(str) - 1;
    while (end >= str && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
}


int read_config(FILE *file, config_t *config) {
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '\n' || line[0] == '#' || line[0] == ';')
            continue;

        char *equals = strchr(line, '=');
        if (!equals) continue;

        *equals = '\0';
        char *key = line;
        char *value = equals + 1;

        trim(key);
        trim(value);

        if (strcmp(key, "accept_threshold") == 0) {
            config->accept_threshold = atoi(value);
        } else if (strcmp(key, "url") == 0) {
            strlcpy(config->url, value, sizeof(config->url));
        } else if (strcmp(key, "test_port") == 0) {
            config->test_port = atoi(value);
        } else if (strcmp(key, "payload_tests") == 0) {
            config->payload_tests = atoi(value);
        }
    }

    return 0;
}
