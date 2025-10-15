#include "dg_products.h"

#include "degiro.h"
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
