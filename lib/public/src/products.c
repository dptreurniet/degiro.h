#include "products.h"

#include <curl/curl.h>

#include "degiro.h"
#include "dg_curl.h"
#include "dg_products.h"
#include "dg_utils.h"
#include "nob.h"

bool dg_get_product(dg_context *ctx, int id) {
    return dg_get_products(ctx, &id, 1);
}

bool dg_get_products(dg_context *ctx, int *ids, size_t n_ids) {
    nob_log(NOB_INFO, "Getting product info for %zu products...", n_ids);

    // Make list of unique ids (required to make request valid)
    struct {
        int *items;
        size_t count;
        size_t capacity;
    } unique_ids = {0};

    nob_da_reserve(&unique_ids, n_ids);

    for (size_t i = 0; i < n_ids; i++) {
        for (size_t j = 0; j < unique_ids.count; j++) {
            if (ids[i] == unique_ids.items[j]) {
                goto next_loop;
            }
        }
        nob_da_append(&unique_ids, ids[i]);
    next_loop:
    }

    nob_log(NOB_INFO, "Reduced to %zu unique ids", unique_ids.count);
    if (unique_ids.count == 0) {
        nob_log(NOB_INFO, "No product info to load");
        return true;
    }

    if (!ctx->user_config.session_id) {
        nob_log(NOB_ERROR, "No session ID defined, unable to make request to DeGiro");
        return false;
    }

    // Generate payload as JSON list of IDs
    cJSON *array = cJSON_CreateArray();
    for (size_t i = 0; i < unique_ids.count; i++) {
        cJSON_AddItemToArray(array, cJSON_CreateString(dg__format_string("%d", unique_ids.items[i])));
    }
    dg__set_curl_payload(ctx, cJSON_PrintUnformatted(array));

    if (!dg__set_default_curl_headers(ctx)) return false;
    dg__set_curl_url(ctx, dg__format_string("%sv5/products/info?intAccount=%d&sessionId=%s",
                                            ctx->user_config.product_search_url,
                                            ctx->user_data.int_account,
                                            ctx->user_config.session_id));
    dg__set_curl_POST(ctx);

    CURLcode res = dg__make_request(ctx);
    if (res != CURLE_OK) {
        nob_log(NOB_ERROR, "Curl request failed: %s", curl_easy_strerror(res));
        return false;
    }

    dg_da_products products = {0};
    if (!dg__parse_products(ctx, &products)) {
        nob_log(NOB_ERROR, "Failed to parse products from response");
        return false;
    }

    size_t old_size = ctx->products.count;
    ctx->products.items = (dg_product *)realloc(ctx->products.items, sizeof(dg_product) * (ctx->products.count + products.count));
    for (size_t i = 0; i < products.count; ++i) {
        ctx->products.items[old_size + i] = products.items[i];
    }
    ctx->products.count = old_size + products.count;

    return true;
}
