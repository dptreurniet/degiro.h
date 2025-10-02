#pragma once

#include "cjson/cJSON.h"
#include <stdbool.h>

const char *format_string(const char *format, ...);
bool parse_string(cJSON *root, const char *json_key, char **destination);
bool parse_int(cJSON *root, const char *json_key, int *destination);
bool parse_string_to_int(cJSON *root, const char *json_key, int *destination);
bool parse_bool(cJSON *root, const char *json_key, bool *destination);
bool parse_double(cJSON *root, const char *json_key, double *destination);
bool is_only_numbers(const char *str);