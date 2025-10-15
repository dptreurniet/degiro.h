#pragma once

#include <stdbool.h>
#include <stddef.h>

// Forward-declare context
struct dg_context;
typedef struct dg_context dg_context;

typedef struct dg_product {
    int id;
    char *name;
    char *isin;
    char *symbol;
    int contract_size;
    char *product_type;
    int product_type_id;
    bool tradable;
    char *category;
    char *currency;
    bool active;
    char *exchange_id;
    bool only_eod_prices;
    char **order_time_types;
    char **buy_order_types;
    char **sell_order_types;
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

bool dg_get_product(dg_context *ctx, int id);
bool dg_get_products(dg_context *ctx, int *ids, size_t n_ids);
