#include "dictionary.h"

#include <cjson/cJSON.h>

#include "dg_curl.h"
#include "dg_dictionary.h"
#include "dg_utils.h"
#include "nob.h"

bool dg__parse_exchanges(cJSON *root, dg_da_exchanges *result) {
    if (!cJSON_IsArray(root)) {
        nob_log(NOB_ERROR, "Expected an array, but got something else");
        return false;
    }

    result->count = 0;
    cJSON *exchange = root->child;
    while (exchange != NULL) {
        dg_exchange ex = {0};

        dg__parse_int(exchange, "id", &ex.id);
        dg__parse_string(exchange, "name", &ex.name);
        dg__parse_string(exchange, "code", &ex.code);
        dg__parse_string(exchange, "hiqAbbr", &ex.hiq_abbr);
        dg__parse_string(exchange, "country", &ex.country);
        dg__parse_string(exchange, "city", &ex.city);
        dg__parse_string(exchange, "micCode", &ex.mic_code);

        nob_da_append(result, ex);
        exchange = exchange->next;
    }

    return true;
}

bool dg_get_dictionary(dg_context *ctx) {
    nob_log(NOB_INFO, "Getting dictionary...");

    if (!dg__set_default_curl_headers(ctx)) return false;
    dg__set_curl_url(ctx, dg__format_string("%s", ctx->user_config.dictionary_url));
    dg__set_curl_payload(ctx, "");
    dg__set_curl_GET(ctx);

    CURLcode res = dg__make_request(ctx);
    if (res != CURLE_OK) {
        nob_log(NOB_ERROR, "Curl request failed: %s", curl_easy_strerror(res));
        return false;
    }

    cJSON *json = cJSON_Parse(ctx->curl.response.data);
    if (json == NULL) {
        nob_log(NOB_ERROR, "Error parsing JSON");
        return false;
    }

    dg_da_exchanges exchanges = {0};
    cJSON *json_exchanges = cJSON_GetObjectItem(json, "exchanges");
    if (!dg__parse_exchanges(json_exchanges, &exchanges)) {
        nob_log(NOB_ERROR, "Failed to parse exchanges");
        return false;
    }

    if (ctx->dictionary.exchanges.items != NULL) free(ctx->dictionary.exchanges.items);
    ctx->dictionary.exchanges.items = (dg_exchange *)malloc(sizeof(dg_exchange) * exchanges.count);
    if (ctx->dictionary.exchanges.items == NULL) {
        nob_log(NOB_ERROR, "Failed to allocate memory for exchanges");
        return false;
    }
    memcpy(ctx->dictionary.exchanges.items, exchanges.items, sizeof(dg_exchange) * exchanges.count);
    ctx->dictionary.exchanges.count = exchanges.count;

    return true;
}