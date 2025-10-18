#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "types.h"

// Forward-declare context
struct dg_context;
typedef struct dg_context dg_context;

typedef enum {
    DG_PRODUCT_TYPE_STOCK,
    DG_PRODUCT_TYPE_INDEX,
    DG_PRODUCT_TYPE_BOND,
    DG_PRODUCT_TYPE_FUTURE,
    DG_PRODUCT_TYPE_OPTION,
    DG_PRODUCT_TYPE_FUND,
    DG_PRODUCT_TYPE_LEVERAGE_PRODUCT,
    DG_PRODUCT_TYPE_ETF,
    DG_PRODUCT_TYPE_CFD,
    DG_PRODUCT_TYPE_WARRANT,
    DG_PRODUCT_TYPE_CURRENCY,
} dg_product_type;

const char *dg_product_type_str(dg_product_type type);

typedef enum {
    DG_ORDER_TYPE_LIMITED,
    DG_ORDER_TYPE_STOP_LIMITED,
    DG_ORDER_TYPE_MARKET_ORDER,
    DG_ORDER_TYPE_STOP_LOSS,
    DG_ORDER_TYPE_AMOUNT,
    DG_ORDER_TYPE_SIZE
} dg_order_type;

typedef uint8_t dg_order_type_flags;

const char *dg_get_order_type_str(dg_order_type_flags flags);

typedef struct dg_product {
    int id;
    char *name;
    char *isin;
    char *symbol;
    int contract_size;
    dg_product_type product_type;
    bool tradable;
    char *category;
    dg_currency currency;
    bool active;
    int exchange_id;
    bool only_eod_prices;
    char **order_time_types;
    dg_order_type_flags buy_order_types;
    dg_order_type_flags sell_order_types;
    double close_price;
    char *close_price_date;
    bool is_shortable;
    char *feed_quality;
    int order_book_depth;
    char *vwd_identifier_type;
    char *vwd_id;
    bool quality_switchable;
    bool quality_switch_free;
    int vwd_module_id;
} dg_product;

typedef struct dg_products {
    dg_product *items;
    size_t count;
} dg_products;

typedef struct dg_search_products_options {
    char *search_string;
    int limit;
    bool include_crypto;
} dg_search_products_options;

bool dg_get_product_info(dg_context *ctx, int id);
bool dg_get_product_infos_info(dg_context *ctx, int *ids, size_t n_ids);
bool dg_search_products(dg_context *ctx, dg_search_products_options options, dg_products *result);
