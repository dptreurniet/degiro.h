#include "degiro_products.h"
#include "nob.h"
#include "utils.h"

bool dg__products_from_json_string(dg_products *products, const char *json_string)
{
    cJSON *json = cJSON_Parse(json_string);
    if (json == NULL)
    {
        nob_log(NOB_ERROR, "Error parsing JSON");
        return false;
    }

    cJSON *data = cJSON_GetObjectItem(json, "data");
    if (!cJSON_IsObject(data)) {
        nob_log(NOB_ERROR, "Failed to load \"data\" in JSON");
        return false;
    }

    cJSON *product = data->child;
    while (product != NULL)
    {
        dg_product p = {0};
        dg__product_from_json_obj(&p, product);
        nob_da_append(products, p);

        product = product->next;
    }

    return true;
}

bool dg__product_from_json_obj(dg_product *product, cJSON *obj)
{
    parse_string_to_int(   obj, "id",                  &product->id);
    parse_string(obj, "name",                &product->name);
    parse_string(obj, "isin",                &product->isin);
    parse_string(obj, "symbol",              &product->symbol);
    parse_int(   obj, "contractSize",       &product->contract_size);
    parse_string(obj, "productType",        &product->product_type);
    parse_int(   obj, "productTypeId",     &product->product_type_id);
    parse_bool(  obj, "tradable",            &product->tradable);
    parse_string(obj, "category",            &product->category);
    parse_string(obj, "currency",            &product->currency);
    parse_bool(  obj, "active",              &product->active);
    parse_string(obj, "exchangeId",         &product->exchange_id);
    parse_bool(  obj, "onlyEodPrices",     &product->only_eod_prices);
    // TODO: parse list items
    // parse_string(obj, "*order_time_types",   &product->order_time_types);
    //parse_string(obj, "*buy_order_types",    &product->buy_order_types);
    //parse_string(obj, "*sell_order_types",   &product->sell_order_types);
    parse_double(obj, "closePrice",         &product->close_price);
    parse_string(obj, "closePriceDate",    &product->close_price_date);
    parse_bool(  obj, "isShortable",        &product->is_shortable);
    parse_string(obj, "feedQuality",        &product->feed_quality);
    parse_int(   obj, "orderBookDepth",    &product->order_book_depth);
    parse_string(obj, "vwdIdentifierType", &product->vwd_identifier_type);
    parse_string(obj, "vwdId",              &product->vwd_id);
    parse_bool(  obj, "qualitySwitchable",  &product->quality_switchable);
    parse_bool(  obj, "qualitySwitchFree", &product->quality_switch_free);
    parse_int(   obj, "vwdModuleId",       &product->vwd_module_id);

    return true;
}

bool dg__product_id_in_library(dg_product_library *registry, int product_id)
{
    for (size_t i = 0; i < registry->count; i ++) {
        if (registry->items[i].id == product_id)
            return true;
    }
    return false;
}

void dg__add_product_to_library(dg_product_library *registry, dg_product *product)
{
    if (dg__product_id_in_library(registry, product->id)) {
        nob_log(NOB_INFO, "Product with id \"%d\" already in library", product->id);
        return;
    }
    nob_da_append(registry, *product);
    nob_log(NOB_INFO, "Added \"%s\" to library", product->name);
}

const dg_product* dg__get_product_from_library(dg_product_library *registry, int product_id)
{
    for (size_t i = 0; i < registry->count; i ++) {
        if (registry->items[i].id == product_id)
        {
            return &registry->items[i];
        }
    }
    return NULL;
}

dg_product* dg__get_product_from_library_by_id(dg_product_library *lib, int id)
{
    for (size_t i = 0; i < lib->count; i++) {
        if (lib->items[i].id == id) {
            return &lib->items[i];
        }
    }
    return NULL;
}