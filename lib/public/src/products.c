#include "products.h"

#include <cjson/cJSON.h>
#include <curl/curl.h>

#include "degiro.h"
#include "dg_curl.h"
#include "dg_types.h"
#include "dg_utils.h"
#include "nob.h"

const char *dg_get_order_type_str(dg_order_type_flags flags) {
    Nob_String_Builder sb = {0};
    if ((flags & (1 << DG_ORDER_TYPE_LIMITED)) != 0) nob_sb_appendf(&sb, "Limit, ");
    if ((flags & (1 << DG_ORDER_TYPE_STOP_LIMITED)) != 0) nob_sb_appendf(&sb, "Stoplimit, ");
    if ((flags & (1 << DG_ORDER_TYPE_MARKET_ORDER)) != 0) nob_sb_appendf(&sb, "Market, ");
    if ((flags & (1 << DG_ORDER_TYPE_STOP_LOSS)) != 0) nob_sb_appendf(&sb, "Stoploss, ");
    if ((flags & (1 << DG_ORDER_TYPE_AMOUNT)) != 0) nob_sb_appendf(&sb, "Amount, ");
    if ((flags & (1 << DG_ORDER_TYPE_SIZE)) != 0) nob_sb_appendf(&sb, "Size, ");
    if (sb.count > 0) sb.count -= 2;  // remove trailing comma

    nob_sb_append_null(&sb);
    return sb.items;
}

bool dg__parse_order_type_flags(cJSON *root, const char *key, dg_order_type_flags *flag) {
    *flag = 0;

    cJSON *types = cJSON_GetObjectItem(root, key);
    if (types == NULL || !cJSON_IsArray(types)) {
        nob_log(NOB_ERROR, "No valid types found");
        return false;
    }

    int array_size = cJSON_GetArraySize(types);
    for (int i = 0; i < array_size; ++i) {
        cJSON *type = cJSON_GetArrayItem(types, i);
        if (type != NULL) {
            const char *type_str = cJSON_GetStringValue(type);
            if (strcmp(type_str, "LIMIT") == 0) *flag |= (1 << DG_ORDER_TYPE_LIMITED);
            if (strcmp(type_str, "MARKET") == 0) *flag |= (1 << DG_ORDER_TYPE_MARKET_ORDER);
            if (strcmp(type_str, "STOPLOSS") == 0) *flag |= (1 << DG_ORDER_TYPE_STOP_LOSS);
            if (strcmp(type_str, "STOPLIMIT") == 0) *flag |= (1 << DG_ORDER_TYPE_STOP_LIMITED);
            if (strcmp(type_str, "STANDARDAMOUNT") == 0) *flag |= (1 << DG_ORDER_TYPE_AMOUNT);
            if (strcmp(type_str, "STANDARDSIZE") == 0) *flag |= (1 << DG_ORDER_TYPE_SIZE);
        }
    }

    return true;
}

dg_product_type dg__product_type_from_int(int type_id) {
    switch (type_id) {
        case 1:
            return DG_PRODUCT_TYPE_STOCK;
        case 180:
            return DG_PRODUCT_TYPE_INDEX;
        case 2:
            return DG_PRODUCT_TYPE_BOND;
        case 7:
            return DG_PRODUCT_TYPE_FUTURE;
        case 8:
            return DG_PRODUCT_TYPE_OPTION;
        case 13:
            return DG_PRODUCT_TYPE_FUND;
        case 14:
            return DG_PRODUCT_TYPE_LEVERAGE_PRODUCT;
        case 131:
            return DG_PRODUCT_TYPE_ETF;
        case 535:
            return DG_PRODUCT_TYPE_CFD;
        case 536:
            return DG_PRODUCT_TYPE_WARRANT;
        case 311:
            return DG_PRODUCT_TYPE_CURRENCY;
        default:
            nob_log(NOB_ERROR, "Unknown type id encountered: %d", type_id);
            NOB_UNREACHABLE("Unknown type id encountered");
    }
}

bool dg__parse_products(cJSON *root, dg_da_products *products) {
    if (!(cJSON_IsObject(root) || cJSON_IsArray(root))) {
        nob_log(NOB_ERROR, "Root object is not an object");
        return false;
    }

    products->count = 0;
    cJSON *product = root->child;
    while (product != NULL) {
        dg_product p = {0};

        dg__parse_string_to_int(product, "id", &p.id);
        dg__parse_string(product, "name", &p.name);
        dg__parse_string(product, "isin", &p.isin);
        dg__parse_string(product, "symbol", &p.symbol);
        dg__parse_int(product, "contractSize", &p.contract_size);
        dg__parse_bool(product, "tradable", &p.tradable);
        dg__parse_string(product, "category", &p.category);
        dg__parse_bool(product, "active", &p.active);
        dg__parse_string_to_int(product, "exchangeId", &p.exchange_id);
        dg__parse_bool(product, "onlyEodPrices", &p.only_eod_prices);
        // TODO: parse list items
        // dg__parse_string(product, "*order_time_types",   &p.order_time_types);
        dg__parse_order_type_flags(product, "buyOrderTypes", &p.buy_order_types);
        dg__parse_order_type_flags(product, "sellOrderTypes", &p.sell_order_types);
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

        int type_id;
        dg__parse_int(product, "productTypeId", &type_id);
        p.product_type = dg__product_type_from_int(type_id);

        char *currency_str;
        dg__parse_string(product, "currency", &currency_str);
        p.currency = dg__currency_from_string(currency_str);
        free(currency_str);

        nob_da_append(products, p);
        product = product->next;
    }

    return true;
}

bool dg_get_product_info(dg_context *ctx, int id) {
    return dg_get_product_infos_info(ctx, &id, 1);
}

bool dg_get_product_infos_info(dg_context *ctx, int *ids, size_t n_ids) {
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

    dg_da_products products = {0};
    if (!dg__parse_products(data, &products)) {
        nob_log(NOB_ERROR, "Failed to parse products from response");
        return false;
    }

    // If a product is loaded that was already in the context, remove the old one from the context
    size_t i = 0;
    while (i < ctx->products.count) {
        bool product_removed = false;
        for (size_t j = 0; j < products.count; ++j) {
            if (products.items[j].id == ctx->products.items[i].id) {
                nob_log(NOB_INFO, "Replacing product %s", products.items[j].name);
                nob_da_remove_unordered(&ctx->products, i);
                product_removed = true;
                break;
            }
        }
        if (!product_removed) ++i;
    }

    size_t old_size = ctx->products.count;
    ctx->products.items = (dg_product *)realloc(ctx->products.items, sizeof(dg_product) * (ctx->products.count + products.count));
    for (size_t i = 0; i < products.count; ++i) {
        ctx->products.items[old_size + i] = products.items[i];
    }
    ctx->products.count = old_size + products.count;

    return true;
}

bool dg_search_products(dg_context *ctx, dg_search_products_options options, dg_products *result) {
    nob_log(NOB_INFO, "Searching products...");

    if (!dg__set_default_curl_headers(ctx)) return false;
    dg__set_curl_url(ctx, dg__format_string("%sv5/products/lookup?intAccount=%d&sessionId=%s&crypto=%s&limit=%d&searchText=%s",
                                            ctx->user_config.product_search_url,
                                            ctx->user_data.int_account,
                                            ctx->user_config.session_id,
                                            options.include_crypto ? "1" : "0",
                                            options.limit,
                                            options.search_string));
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

    cJSON *products_json = cJSON_GetObjectItem(json, "products");
    if (!cJSON_IsArray(products_json)) {
        nob_log(NOB_ERROR, "Failed to load \"products\" in JSON");
        return false;
    }

    dg_da_products products = {0};
    dg__parse_products(products_json, &products);

    if (result->items != NULL) free(result->items);
    result->items = (dg_product *)malloc(sizeof(dg_product) * products.count);
    if (result->items == NULL) {
        nob_log(NOB_ERROR, "Failed to allocate memory for search results");
        return false;
    }
    memcpy(result->items, products.items, products.count * sizeof(dg_product));
    result->count = products.count;

    return true;
}

const char *dg_product_type_str(dg_product_type type_id) {
    switch (type_id) {
        case DG_PRODUCT_TYPE_STOCK:
            return "STOCK";
        case DG_PRODUCT_TYPE_INDEX:
            return "INDEX";
        case DG_PRODUCT_TYPE_BOND:
            return "BOND";
        case DG_PRODUCT_TYPE_FUTURE:
            return "FUTURE";
        case DG_PRODUCT_TYPE_OPTION:
            return "OPTION";
        case DG_PRODUCT_TYPE_FUND:
            return "FUND";
        case DG_PRODUCT_TYPE_LEVERAGE_PRODUCT:
            return "PRODUCT";
        case DG_PRODUCT_TYPE_ETF:
            return "ETF";
        case DG_PRODUCT_TYPE_CFD:
            return "CFD";
        case DG_PRODUCT_TYPE_WARRANT:
            return "WARRANT";
        case DG_PRODUCT_TYPE_CURRENCY:
            return "CURRENCY";
        default:
            NOB_UNREACHABLE("Missing type case");
    }
    return "";
}
