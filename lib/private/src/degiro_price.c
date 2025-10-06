#include "degiro_price.h"

#include "nob.h"
#include <stdio.h>
#include "degiro_utils.h"

bool dg__parse_chart_response_time_data(cJSON *item, dg_product_chart *result)
{
    // Get series start time
    cJSON *start_time_obj = cJSON_GetObjectItem(item, "times");
    if (!cJSON_IsString(start_time_obj))
    {
        nob_log(NOB_WARNING, "Encountered \"times\" entry as non-string type");
        return false;
    }
    const char *start_time = cJSON_GetStringValue(start_time_obj); 
    // e.g. "2025-09-30T00:00:00/PT1M" when resolution is smaller than a day
    // or   "2024-10-03/P1D" otherwise
    // Determine which is encountered by checking for 'T' on index 10

    struct tm tm = {0};
    char *resolution = (char *)malloc(sizeof(char) * 8);

    if (start_time[10] == 'T') { // e.g. 2025-09-30T00:00:00/PT1M
        sscanf(start_time, "%d-%d-%dT%d:%d:%d/P%s",
        &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
        &tm.tm_hour, &tm.tm_min, &tm.tm_sec,
        resolution);
    } else { // e.g. 2025-09-30/P1D
        sscanf(start_time, "%d-%d-%d/P%s",
        &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
        resolution);
    }

    // Convert tm to unix timestamp
    tm.tm_year -= 1900;
    tm.tm_mon -= 1;
    time_t start_time_unix = mktime(&tm);

    if (start_time_unix == -1)
    {
        nob_log(NOB_ERROR, "Failed to convert to Unix timestamp");
        return false;
    }

    time_t time_resolution = 0;
    if (strcmp(resolution, "1D") == 0)        time_resolution = 60*60*24;
    else if (strcmp(resolution, "T2H") == 0)  time_resolution = 60*60*2;
    else if (strcmp(resolution, "T30M") == 0) time_resolution = 60*30;
    else if (strcmp(resolution, "T1M") == 0)  time_resolution = 60;
    else {
        nob_log(NOB_ERROR, "Time resolution not defined for \"%s\"", resolution);
        return false;
    }

    // Gather data
    cJSON *data = cJSON_GetObjectItem(item, "data");

    if (!cJSON_IsArray(data))
    {
        nob_log(NOB_ERROR, "\"data\" entry is expected to be an array, but it is not");
        return false;
    }

    result->chart.n_points = cJSON_GetArraySize(data);
    result->chart.timestamps = (double *)malloc(sizeof(double) * cJSON_GetArraySize(data));
    result->chart.prices =     (double *)malloc(sizeof(double) * cJSON_GetArraySize(data));

    for (int j = 0; j < cJSON_GetArraySize(data); j++)
    {
        cJSON *dataItem = cJSON_GetArrayItem(data, j);
        if (!cJSON_IsArray(dataItem))
        {
            nob_log(NOB_ERROR, "Item in \"data\" array is expected to be an array, but it is not");
            return false;
        }

        result->chart.timestamps[j] = (double) start_time_unix + time_resolution * cJSON_GetArrayItem(dataItem, 0)->valueint;
        result->chart.prices[j] = cJSON_GetArrayItem(dataItem, 1)->valuedouble;
    }
    return true;
}

bool dg__parse_chart_response_object_data(cJSON *item, dg_product_chart *result)
{
    cJSON *data = cJSON_GetObjectItem(item, "data");
    if (!cJSON_IsObject(data))
    {
        nob_log(NOB_WARNING, "Encountered \"data\" entry as non-object type");
        return false;
    }

    parse_string(data, "currency", &result->currency);
    parse_double(data, "windowLowPrice", &result->low_price);
    parse_double(data, "windowHighPrice", &result->high_price);

    return true;
}

bool dg__parse_chart_response(cJSON *data, dg_product_chart *result)
{
    cJSON *series = cJSON_GetObjectItem(data, "series");

    if (!cJSON_IsArray(series))
    {
        nob_log(NOB_ERROR, "\"series\" entry is expected to be an array, but it is not");
        return false;
    }

    bool object_parsed = false;
    bool time_data_parsed = false;

    for (int i = 0; i < cJSON_GetArraySize(series); i++)
    {
        cJSON *item = cJSON_GetArrayItem(series, i);

        // Check series type
        cJSON *type = cJSON_GetObjectItem(item, "type");
        if (!cJSON_IsString(type))
        {
            nob_log(NOB_WARNING, "Encountered \"type\" entry as non-string type");
            continue;
        }

        if (strcmp(cJSON_GetStringValue(type), "time") == 0)
        {
            if(!dg__parse_chart_response_time_data(item, result)) {
                nob_log(NOB_ERROR, "Failed to parse time data in response");
                return false;
            } else { time_data_parsed = true; }
        }
        else if (strcmp(cJSON_GetStringValue(type), "object") == 0)
        {
            if(!dg__parse_chart_response_object_data(item, result)) {
                nob_log(NOB_ERROR, "Failed to parse object data in response");
                return false;
            } else { object_parsed = true; }
        }
        else
        {
            nob_log(NOB_WARNING, "Encountered unexpected type in data: \"%s\"", cJSON_GetStringValue(type));
        }
    }

    if (!time_data_parsed) {
        nob_log(NOB_ERROR, "No time data processed");
        return false;
    }

    if (!object_parsed) {
        nob_log(NOB_ERROR, "No object processed");
        return false;
    }

    return true;
}
