#include "degiro.h"
#include "nob.h"

const char *dg_chart_period_to_str(dg_chart_period p) {
    switch (p) {
        case PERIOD_1D:
            return "1D";
        case PERIOD_1W:
            return "1W";
        case PERIOD_1M:
            return "1M";
        case PERIOD_1Y:
            return "1Y";
        default:
            NOB_UNREACHABLE("Undefined chart period");
    }
    return "";
}

bool dg_get_product_by_id(degiro *dg, int id, dg_product *result) {
    for (size_t i = 0; i < dg->products.count; ++i) {
        if (dg->products.items[i].id == id) {
            *result = dg->products.items[i];
            return true;
        }
    }
    return false;
}

time_t dg_time_string_to_unix_timestamp(const char *str) {
    // Possible formats to recognize:
    // - 2025-09-30
    // - 2025-09-30/P1D
    // - 2025-09-30T00:00:00+01:00
    // - 2025-09-30T00:00:00/PT1M

    struct tm tm = {0};

    if (strlen(str) == 10) {  // 2025-09-30
        sscanf(str, "%d-%d-%d",
               &tm.tm_year, &tm.tm_mon, &tm.tm_mday);

    } else if (strlen(str) == 14) {  // 2025-09-30/P1D
        sscanf(str, "%d-%d-%d/",
               &tm.tm_year, &tm.tm_mon, &tm.tm_mday);

    } else if (strlen(str) == 25) {  // 2025-09-30T00:00:00+01:00
        char timezone_sign[2];
        int timezone_hour, timezone_min;
        sscanf(str, "%d-%d-%dT%d:%d:%d%1s%d:%d",
               &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
               &tm.tm_hour, &tm.tm_min, &tm.tm_sec,
               timezone_sign, &timezone_hour, &timezone_min);
        tm.tm_hour += (strcmp(timezone_sign, "+") == 0 ? 1 : -1) * timezone_hour;
        tm.tm_min += (strcmp(timezone_sign, "+") == 0 ? 1 : -1) * timezone_min;

    } else if (strlen(str) == 24) {  // 2025-09-30T00:00:00/PT1M
        sscanf(str, "%d-%d-%dT%d:%d:%d/",
               &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
               &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
    }

    tm.tm_year -= 1900;
    tm.tm_mon -= 1;
    time_t start_time_unix = mktime(&tm);

    if (start_time_unix == -1) {
        nob_log(NOB_ERROR, "Failed to convert to Unix timestamp");
        return 0;
    }

    return start_time_unix;
}