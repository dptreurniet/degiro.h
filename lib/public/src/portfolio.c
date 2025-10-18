#include "portfolio.h"

#include <cjson/cJSON.h>
#include <curl/curl.h>

#include "degiro.h"
#include "dg_curl.h"
#include "dg_types.h"
#include "dg_utils.h"
#include "nob.h"

bool dg__parse_portfolio(dg_context *ctx) {
    cJSON *json = cJSON_Parse(ctx->curl.response.data);
    if (json == NULL) {
        nob_log(NOB_INFO, "Error parsing JSON");
        return false;
    }

    cJSON *pf = cJSON_GetObjectItem(json, "portfolio");
    if (!cJSON_IsObject(pf)) {
        nob_log(NOB_INFO, "No \"portfolio\" field in JSON");
        return false;
    }

    cJSON *json_positions = cJSON_GetObjectItemCaseSensitive(pf, "value");
    size_t n_positions = (size_t)cJSON_GetArraySize(json_positions);

    dg_da_positions positions = {0};
    nob_da_reserve(&positions, n_positions);

    cJSON *position = NULL;
    cJSON_ArrayForEach(position, json_positions) {
        cJSON *values = cJSON_GetObjectItemCaseSensitive(position, "value");
        cJSON *value = NULL;

        dg_position pos = {0};
        char *key;

        cJSON_ArrayForEach(value, values) {
            if (!dg__parse_string(value, "name", &key)) {
                nob_log(NOB_ERROR, "Failed to parse key");
                return false;
            }

            if (strcmp(key, "id") == 0)
                dg__parse_string(value, "value", &pos.id);
            else if (strcmp(key, "positionType") == 0) {
                dg__parse_string(value, "value", &pos.position_type);
            } else if (strcmp(key, "size") == 0) {
                dg__parse_int(value, "value", &pos.size);
            } else if (strcmp(key, "price") == 0) {
                dg__parse_double(value, "value", &pos.price);
            } else if (strcmp(key, "value") == 0) {
                dg__parse_double(value, "value", &pos.value);
            } else if (strcmp(key, "plBase") == 0) {
                dg__parse_double(cJSON_GetObjectItem(value, "value"), "EUR", &pos.pl_base);
            } else if (strcmp(key, "todayPlBase") == 0) {
                dg__parse_double(cJSON_GetObjectItem(value, "value"), "EUR", &pos.today_pl_base);
            } else if (strcmp(key, "portfolioValueCorrection") == 0) {
                dg__parse_double(value, "value", &pos.portfolio_value_correction);
            } else if (strcmp(key, "breakEvenPrice") == 0) {
                dg__parse_double(value, "value", &pos.break_even_price);
            } else if (strcmp(key, "averageFxRate") == 0) {
                dg__parse_double(value, "value", &pos.average_fx_rate);
            } else if (strcmp(key, "realizedProductPl") == 0) {
                dg__parse_double(value, "value", &pos.realized_product_pl);
            } else if (strcmp(key, "realizedFxPl") == 0) {
                dg__parse_double(value, "value", &pos.realized_fx_pl);
            } else if (strcmp(key, "todayRealizedProductPl") == 0) {
                dg__parse_double(value, "value", &pos.today_realized_product_pl);
            } else if (strcmp(key, "todayRealizedFxPl") == 0) {
                dg__parse_double(value, "value", &pos.today_realized_fx_pl);
            } else {
                continue;  // Any other fields are ignored for now
            }
        }

        nob_da_append(&positions, pos);
    }

    if (ctx->portfolio.positions != NULL) free(ctx->portfolio.positions);
    ctx->portfolio.positions = (dg_position *)malloc(sizeof(dg_position) * positions.count);
    memcpy(ctx->portfolio.positions, positions.items, sizeof(dg_position) * positions.count);
    ctx->portfolio.count = positions.count;

    nob_log(NOB_INFO, "Finished parsing portfolio (%zu positions)", n_positions);

    cJSON_Delete(json);
    return true;
}

bool dg_get_portfolio(dg_context *ctx) {
    nob_log(NOB_INFO, "Getting portfolio");
    if (!ctx->user_config.session_id) {
        nob_log(NOB_ERROR, "No session id defined");
        return false;
    }

    dg__set_default_curl_headers(ctx);
    dg__set_curl_url(ctx, dg__format_string("%sv5/update/%d;jsessionid=%s?&portfolio=0",
                                            ctx->user_config.trading_url,
                                            ctx->user_data.int_account,
                                            ctx->user_config.session_id));
    dg__set_curl_payload(ctx, "");
    dg__set_curl_GET(ctx);

    CURLcode res = dg__make_request(ctx);
    if (res != CURLE_OK) {
        nob_log(NOB_ERROR, "CURL request failed: %s", curl_easy_strerror(res));
        return false;
    }

    dg__parse_portfolio(ctx);

    // Generate array of number-only ids (to ignore cash positions)
    /*
    size_t n_ids = 0;
    for (size_t i = 0; i < dg->portfolio.count; i++) {
        if (is_only_numbers(dg->portfolio.items[i].id)) {
            n_ids++;
        }
    }

    int *ids = (int *)malloc(sizeof(int) * dg->portfolio.count);
    for (size_t i = 0; i < dg->portfolio.count; i++) {
        if (is_only_numbers(dg->portfolio.items[i].id)) {
            ids[i] = atoi(dg->portfolio.items[i].id);
        }
    }

    dg_products products = {0};
    dg__get_products_info(dg, ids, n_ids, &products);

    free(ids);
    */
    return true;
}
