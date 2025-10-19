#include "chart.h"

#include <cjson/cJSON.h>
#include <time.h>

#include "defines.h"
#include "dg_curl.h"
#include "dg_utils.h"
#include "nob.h"

bool dg__parse_chart_response_time_data(cJSON *item, dg_product_chart *chart) {
    // Get series start time
    cJSON *start_time_obj = cJSON_GetObjectItem(item, "times");
    if (!cJSON_IsString(start_time_obj)) {
        nob_log(NOB_WARNING, "Encountered \"times\" entry as non-string type");
        return false;
    }
    const char *start_time = cJSON_GetStringValue(start_time_obj);
    // e.g. "2025-09-30T00:00:00/PT1M" when resolution is smaller than a day
    // or   "2024-10-03/P1D" otherwise
    // Determine which is encountered by checking for 'T' on index 10

    struct tm tm = {0};
    char *resolution = (char *)malloc(sizeof(char) * 8);

    if (start_time[10] == 'T') {  // e.g. 2025-09-30T00:00:00/PT1M
        sscanf(start_time, "%d-%d-%dT%d:%d:%d/P%s",
               &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
               &tm.tm_hour, &tm.tm_min, &tm.tm_sec,
               resolution);
    } else {  // e.g. 2025-09-30/P1D
        sscanf(start_time, "%d-%d-%d/P%s",
               &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
               resolution);
    }

    // Convert tm to unix timestamp
    tm.tm_year -= 1900;
    tm.tm_mon -= 1;
    time_t start_time_unix = mktime(&tm);

    if (start_time_unix == -1) {
        nob_log(NOB_ERROR, "Failed to convert to Unix timestamp");
        return false;
    }

    time_t time_resolution = 0;
    if (strcmp(resolution, "7D") == 0)
        time_resolution = 60 * 60 * 24 * 7;
    else if (strcmp(resolution, "3D") == 0)
        time_resolution = 60 * 60 * 24 * 3;
    else if (strcmp(resolution, "1D") == 0)
        time_resolution = 60 * 60 * 24;
    else if (strcmp(resolution, "T2H") == 0)
        time_resolution = 60 * 60 * 2;
    else if (strcmp(resolution, "T30M") == 0)
        time_resolution = 60 * 30;
    else if (strcmp(resolution, "T1M") == 0)
        time_resolution = 60;
    else {
        nob_log(NOB_ERROR, "Time resolution not defined for \"%s\"", resolution);
        return false;
    }

    // Gather data
    cJSON *data = cJSON_GetObjectItem(item, "data");

    if (!cJSON_IsArray(data)) {
        nob_log(NOB_ERROR, "\"data\" entry is expected to be an array, but it is not");
        return false;
    }

    chart->chart_data.n_points = cJSON_GetArraySize(data);
    chart->chart_data.timestamps = (double *)malloc(sizeof(double) * cJSON_GetArraySize(data));
    chart->chart_data.prices = (double *)malloc(sizeof(double) * cJSON_GetArraySize(data));

    for (int j = 0; j < cJSON_GetArraySize(data); j++) {
        cJSON *dataItem = cJSON_GetArrayItem(data, j);
        if (!cJSON_IsArray(dataItem)) {
            nob_log(NOB_ERROR, "Item in \"data\" array is expected to be an array, but it is not");
            return false;
        }

        chart->chart_data.timestamps[j] = (double)start_time_unix + time_resolution * cJSON_GetArrayItem(dataItem, 0)->valueint;
        chart->chart_data.prices[j] = cJSON_GetArrayItem(dataItem, 1)->valuedouble;
    }
    return true;
}

bool dg__parse_chart_response_object_data(cJSON *item, dg_product_chart *chart) {
    cJSON *data = cJSON_GetObjectItem(item, "data");
    if (!cJSON_IsObject(data)) {
        nob_log(NOB_WARNING, "Encountered \"data\" entry as non-object type");
        return false;
    }

    dg__parse_string(data, "currency", &chart->currency);
    dg__parse_double(data, "windowLowPrice", &chart->low_price);
    dg__parse_double(data, "windowHighPrice", &chart->high_price);

    return true;
}

bool dg__parse_chart_response(cJSON *data, dg_product_chart *chart) {
    cJSON *series = cJSON_GetObjectItem(data, "series");

    if (!cJSON_IsArray(series)) {
        nob_log(NOB_ERROR, "\"series\" entry is expected to be an array, but it is not");
        return false;
    }

    bool object_parsed = false;
    bool time_data_parsed = false;

    for (int i = 0; i < cJSON_GetArraySize(series); i++) {
        cJSON *item = cJSON_GetArrayItem(series, i);

        // Check series type
        cJSON *type = cJSON_GetObjectItem(item, "type");
        if (!cJSON_IsString(type)) {
            nob_log(NOB_WARNING, "Encountered \"type\" entry as non-string type");
            continue;
        }

        if (strcmp(cJSON_GetStringValue(type), "time") == 0) {
            if (!dg__parse_chart_response_time_data(item, chart)) {
                nob_log(NOB_ERROR, "Failed to parse time data in response");
                return false;
            } else {
                time_data_parsed = true;
            }
        } else if (strcmp(cJSON_GetStringValue(type), "object") == 0) {
            if (!dg__parse_chart_response_object_data(item, chart)) {
                nob_log(NOB_ERROR, "Failed to parse object data in response");
                return false;
            } else {
                object_parsed = true;
            }
        } else {
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

bool dg_get_product_info_chart(dg_context *ctx, dg_product_chart_options opts, dg_product_chart *chart) {
    // https://charting.vwdservices.com/hchart/v1/deGiro/data.js?requestid=1&resolution=P1D&culture=nl-NL&period=P50Y&series=issueid%3A485013849&series=price%3Aissueid%3A485013849&format=json&callback=vwd.hchart.seriesRequestManager.sync_response&userToken=4626342&tz=Europe%2FAmsterdam
    // https://charting.vwdservices.com/hchart/v1/deGiro/data.js?requestid=1&resolution=P1D&culture=nl-NL&start=2025-01-01&end=2025-10-18&series=vwdkey:IE0007Y8Y157.TRADE,E&series=price:vwdkey:IE0007Y8Y157.TRADE,E&format=json&callback=vwd.hchart.seriesRequestManager.sync_response&userToken=4626342&tz=Europe/Amsterdam

    nob_log(NOB_INFO, "Getting price info of %s", opts.product.name);

    chart->product = opts.product;

    Nob_String_Builder url = {0};
    nob_sb_appendf(&url, "%s", DEGIRO_GET_CHART_URL);
    nob_sb_appendf(&url, "?series=%s:%s", opts.product.vwd_identifier_type, opts.product.vwd_id);
    nob_sb_appendf(&url, "&series=price:%s:%s", opts.product.vwd_identifier_type, opts.product.vwd_id);

    switch (opts.period) {
        case PERIOD_1D:
            nob_sb_appendf(&url, "&period=P1D");
            nob_sb_appendf(&url, "&resolution=PT1M");
            break;
        case PERIOD_1W:
            nob_sb_appendf(&url, "&period=P1W");
            nob_sb_appendf(&url, "&resolution=PT30M");
            break;
        case PERIOD_1M:
            nob_sb_appendf(&url, "&period=P1M");
            nob_sb_appendf(&url, "&resolution=PT2H");
            break;
        case PERIOD_6M:
            nob_sb_appendf(&url, "&period=P6M");
            nob_sb_appendf(&url, "&resolution=P1D");
            break;
        case PERIOD_1Y:
            nob_sb_appendf(&url, "&period=P1Y");
            nob_sb_appendf(&url, "&resolution=P1D");
            break;
        case PERIOD_3Y:
            nob_sb_appendf(&url, "&period=P3Y");
            nob_sb_appendf(&url, "&resolution=P3D");
            break;
        case PERIOD_5Y:
            nob_sb_appendf(&url, "&period=P5Y");
            nob_sb_appendf(&url, "&resolution=P1W");
            break;
        case PERIOD_MAX:
            nob_sb_appendf(&url, "&period=P50Y");
            nob_sb_appendf(&url, "&resolution=P1W");
            break;
        case PERIOD_YTD: {
            time_t t_now = time(NULL);
            struct tm *now = localtime(&t_now);
            nob_sb_appendf(&url, "&start=%04d-01-01", now->tm_year + 1900);
            nob_sb_appendf(&url, "&end=%04d-%02d-%02d", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday);
            break;
        }
        default:
            NOB_UNREACHABLE("Undefined period");
    }

    nob_sb_appendf(&url, "&userToken=%d", ctx->user_config.client_id);
    nob_sb_appendf(&url, "&culture=nl-NL");
    nob_sb_appendf(&url, "&format=json");
    nob_sb_appendf(&url, "&tz=Europe/Amsterdam");
    nob_sb_append_null(&url);

    dg__set_curl_url(ctx, url.items);
    dg__set_curl_GET(ctx);
    dg__make_request(ctx);

    cJSON *json = cJSON_Parse(ctx->curl.response.data);
    if (json == NULL) {
        nob_log(NOB_ERROR, "Error parsing JSON");
        return false;
    }

    return dg__parse_chart_response(json, chart);
}