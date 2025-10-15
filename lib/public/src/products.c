#include "products.h"

#include <curl/curl.h>

#include "degiro.h"
#include "dg_curl.h"
#include "dg_products.h"
#include "dg_utils.h"
#include "nob.h"

bool dg__parse_products(dg_context *ctx, dg_da_products *products) {
    cJSON *json = cJSON_Parse(ctx->curl.response.data);
    if (json == NULL) {
        nob_log(NOB_ERROR, "Error parsing JSON");
        return false;
    }

    cJSON *data = cJSON_GetObjectItem(json, "data");
    if (!cJSON_IsObject(data)) {
        nob_log(NOB_ERROR, "Failed to load \"data\" in JSON");
        return false;
    }

    products->count = 0;
    cJSON *product = data->child;
    while (product != NULL) {
        dg_product p = {0};

        dg__parse_string_to_int(product, "id", &p.id);
        dg__parse_string(product, "name", &p.name);
        dg__parse_string(product, "isin", &p.isin);
        dg__parse_string(product, "symbol", &p.symbol);
        dg__parse_int(product, "contractSize", &p.contract_size);
        dg__parse_string(product, "productType", &p.product_type);
        dg__parse_int(product, "productTypeId", &p.product_type_id);
        dg__parse_bool(product, "tradable", &p.tradable);
        dg__parse_string(product, "category", &p.category);
        dg__parse_string(product, "currency", &p.currency);
        dg__parse_bool(product, "active", &p.active);
        dg__parse_string(product, "exchangeId", &p.exchange_id);
        dg__parse_bool(product, "onlyEodPrices", &p.only_eod_prices);
        // TODO: parse list items
        // dg__parse_string(product, "*order_time_types",   &p.order_time_types);
        // dg__parse_string(product, "*buy_order_types",    &p.buy_order_types);
        // dg__parse_string(product, "*sell_order_types",   &p.sell_order_types);
        dg__parse_double(product, "closePrice", &p.close_price);
        dg__parse_string(product, "closePriceDate", &p.close_price_date);
        dg__parse_bool(product, "isShortable", &p.is_shortable);
        dg__parse_string(product, "feedQuality", &p.feed_quality);
        dg__parse_int(product, "orderBookDepth", &p.order_book_depth);
        dg__parse_string(product, "vwdIdentifierType", &p.vwd_identifier_type);
        dg__parse_string(product, "vwdId", &p.vwd_id);
        dg__parse_bool(product, "qualitySwitchable", &p.quality_switchable);
        dg__parse_bool(product, "qualitySwitchFree", &p.quality_switch_free);
        dg__parse_int(product, "vwdModuleId", &p.vwd_module_id);

        nob_da_append(products, p);
        product = product->next;
    }

    return true;
}

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
