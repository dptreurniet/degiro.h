#pragma once

#include <stdarg.h>
#include <stdbool.h>

#include "cjson/cJSON.h"

void dg__dump_to_file(const char *str, const char *filename);
const char *dg__format_string(const char *format, ...);

bool dg__parse_string(cJSON *root, const char *json_key, char **destination);
bool dg__parse_string_to_int(cJSON *root, const char *json_key, int *destination);
bool dg__parse_int(cJSON *root, const char *json_key, int *destination);
bool dg__parse_long_long_int(cJSON *root, const char *json_key, long long int *destination);
bool dg__parse_bool(cJSON *root, const char *json_key, bool *destination);
bool dg__parse_double(cJSON *root, const char *json_key, double *destination);
