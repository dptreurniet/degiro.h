#pragma once

#include <stdarg.h>
#include <stdbool.h>

#include "cjson/cJSON.h"

void dump_to_file(const char *str, const char *filename);
const char *format_string(const char *format, ...);

bool parse_string(cJSON *root, const char *json_key, char **destination);
bool parse_string_to_int(cJSON *root, const char *json_key, int *destination);
bool parse_int(cJSON *root, const char *json_key, int *destination);
bool parse_long_long_int(cJSON *root, const char *json_key, long long int *destination);
bool parse_bool(cJSON *root, const char *json_key, bool *destination);
bool parse_double(cJSON *root, const char *json_key, double *destination);
