#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include "defines.h"
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include "nob.h"

const char *format_string(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    size_t buffer_size = vsnprintf(NULL, 0, format, args) + 1;
    va_end(args);

    char *result = (char *)malloc(buffer_size);

    va_start(args, format);
    vsnprintf(result, buffer_size, format, args);
    va_end(args);

    return result;
}

bool parse_string(cJSON *root, const char *json_key, char **destination)
{
    cJSON *obj = cJSON_GetObjectItem(root, json_key);

    if (cJSON_IsString(obj) && (obj->valuestring != NULL))
    {
        *(destination) = (char *)malloc(strlen(obj->valuestring) + 1);
        strcpy(*(destination), obj->valuestring);
    }
    else
    {
        nob_log(NOB_WARNING, "Tried to parse \"%s\" as string, but it is a different type", json_key);
        return false;
    }

    return true;
}

bool parse_int(cJSON *root, const char *json_key, int *destination)
{
    cJSON *obj = cJSON_GetObjectItem(root, json_key);

    if (cJSON_IsNumber(obj))
    {
        *(destination) = obj->valueint;
    }
    else
    {
        nob_log(NOB_WARNING, "Tried to parse \"%s\" as int, but it is a different type", json_key);
        return false;
    }

    return true;
}

bool parse_string_to_int(cJSON *root, const char *json_key, int *destination)
{
    cJSON *obj = cJSON_GetObjectItem(root, json_key);

    char *temp;
    if (cJSON_IsString(obj) && (obj->valuestring != NULL))
    {
        temp = (char *)malloc(strlen(obj->valuestring) + 1);
        strcpy(temp, obj->valuestring);
    }
    else
    {
        nob_log(NOB_WARNING, "Tried to parse \"%s\" as string, but it is a different type", json_key);
        return false;
    }

    char *end_ptr;
    *destination = (int)strtol(temp, &end_ptr, 10);
    if (*end_ptr != '\0')
    {
        nob_log(NOB_ERROR, "Failed to convert string to int");
        return false;
    }

    return true;
}

bool parse_bool(cJSON *root, const char *json_key, bool *destination)
{
    cJSON *obj = cJSON_GetObjectItem(root, json_key);

    if (cJSON_IsBool(obj))
    {
        *(destination) = cJSON_IsTrue(obj);
    }
    else
    {
        nob_log(NOB_WARNING, "Tried to parse \"%s\" as bool, but it is a different type", json_key);
        return false;
    }

    return true;
}

bool parse_double(cJSON *root, const char *json_key, double *destination)
{
    cJSON *obj = cJSON_GetObjectItem(root, json_key);

    if (cJSON_IsNumber(obj))
    {
        *(destination) = obj->valuedouble;
    }
    else
    {
        nob_log(NOB_WARNING, "Tried to parse \"%s\" as double, but it is a different type", json_key);
        return false;
    }

    return true;
}

bool is_only_numbers(const char *str) {
    if (str == NULL) {
        return false; // Not a valid string
    }

    while (*str) {
        if (!isdigit((unsigned char)*str)) {
            return false;
        }
        str++;
    }
    return true;
}
