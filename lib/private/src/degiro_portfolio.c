#include "degiro_portfolio.h"

#include <cjson/cJSON.h>
#include "nob.h"
#include "utils.h"

bool dg__portfolio_from_json_string(dg_portfolio *portfolio, const char *json_string)
{
    cJSON *json = cJSON_Parse(json_string);
    if (json == NULL)
    {
        nob_log(NOB_INFO, "Error parsing JSON");
        return false;
    }

    cJSON *pf = cJSON_GetObjectItem(json, "portfolio");
    if (!cJSON_IsObject(pf))
    {
        nob_log(NOB_INFO, "No \"portfolio\" field in JSON");
        return false;
    }

    cJSON *positions = cJSON_GetObjectItemCaseSensitive(pf, "value");
    size_t n_positions = (size_t)cJSON_GetArraySize(positions);

    portfolio->count = 0;
    nob_da_reserve(portfolio, n_positions);

    cJSON *position = NULL;
    cJSON_ArrayForEach(position, positions)
    {
        cJSON *values = cJSON_GetObjectItemCaseSensitive(position, "value");
        cJSON *value = NULL;

        dg_position pos = {0};

        cJSON_ArrayForEach(value, values)
        {
            cJSON *key = cJSON_GetObjectItem(value, "name");
            if (!cJSON_IsString(key))
            {
                nob_log(NOB_WARNING, "Encountered a key that is not a string, skipping it");
                continue;
            }

            cJSON *val = cJSON_GetObjectItem(value, "value");

            if (strcmp(key->valuestring, "id") == 0)
            {
                if (!cJSON_IsString(val))
                    continue;
                pos.id = (char *)malloc(strlen(val->valuestring) + 1);
                strcpy(pos.id, val->valuestring);
            }
            else if (strcmp(key->valuestring, "positionType") == 0)
            {
                if (!cJSON_IsString(val))
                    continue;
                pos.position_type = (char *)malloc(strlen(val->valuestring) + 1);
                strcpy(pos.position_type, val->valuestring);
            }
            else if (strcmp(key->valuestring, "size") == 0)
            {
                if (!cJSON_IsNumber(val))
                    continue;
                pos.size = val->valueint;
            }
            else if (strcmp(key->valuestring, "price") == 0)
            {
                if (!cJSON_IsNumber(val))
                    continue;
                pos.price = val->valuedouble;
            }
            else if (strcmp(key->valuestring, "value") == 0)
            {
                if (!cJSON_IsNumber(val))
                    continue;
                pos.value = val->valuedouble;
            }
            else if (strcmp(key->valuestring, "plBase") == 0)
            {
                if (!cJSON_IsNumber(val))
                    continue;
                pos.pl_base = val->valuedouble;
            }
            else if (strcmp(key->valuestring, "todayPlBase") == 0)
            {
                if (!cJSON_IsNumber(val))
                    continue;
                pos.today_pl_base = val->valuedouble;
            }
            else if (strcmp(key->valuestring, "portfolioValueCorrection") == 0)
            {
                if (!cJSON_IsNumber(val))
                    continue;
                pos.portfolio_value_correction = val->valuedouble;
            }
            else if (strcmp(key->valuestring, "breakEvenPrice") == 0)
            {
                if (!cJSON_IsNumber(val))
                    continue;
                pos.break_even_price = val->valuedouble;
            }
            else if (strcmp(key->valuestring, "averageFxRate") == 0)
            {
                if (!cJSON_IsNumber(val))
                    continue;
                pos.average_fx_rate = val->valuedouble;
            }
            else if (strcmp(key->valuestring, "realizedProductPl") == 0)
            {
                if (!cJSON_IsNumber(val))
                    continue;
                pos.realized_product_pl = val->valuedouble;
            }
            else if (strcmp(key->valuestring, "realizedFxPl") == 0)
            {
                if (!cJSON_IsNumber(val))
                    continue;
                pos.realized_fx_pl = val->valuedouble;
            }
            else if (strcmp(key->valuestring, "todayRealizedProductPl") == 0)
            {
                if (!cJSON_IsNumber(val))
                    continue;
                pos.today_realized_product_pl = val->valuedouble;
            }
            else if (strcmp(key->valuestring, "todayRealizedFxPl") == 0)
            {
                if (!cJSON_IsNumber(val))
                    continue;
                pos.today_realized_fx_pl = val->valuedouble;
            }
            else
            {
                continue; // Any other fields are ignored for now
            }
        }
        
        nob_da_append(portfolio, pos);
    }

    nob_log(NOB_INFO, "Finished parsing portfolio (%zu positions)", n_positions);

    cJSON_Delete(json);
    return true;
}