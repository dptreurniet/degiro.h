#include "helpers.h"

dg_exchange *dg_lookup_exchange_by_id(dg_context *ctx, int id) {
    for (size_t i = 0; i < ctx->dictionary.exchanges.count; ++i) {
        if (ctx->dictionary.exchanges.items[i].id == id) {
            return &ctx->dictionary.exchanges.items[i];
        }
    }
    return NULL;
}

dg_product *dg_lookup_product_by_id(dg_context *ctx, int id) {
    for (size_t i = 0; i < ctx->products.count; ++i) {
        if (ctx->products.items[i].id == id) {
            return &ctx->products.items[i];
        }
    }
    return NULL;
}
