#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "types.h"

// Forward-declare context
struct dg_context;
typedef struct dg_context dg_context;

typedef struct dg_position {
    char *id;
    char *position_type;
    int size;
    double price;
    double value;
    // TODO: accrued_interest;
    double pl_base;
    double today_pl_base;
    double portfolio_value_correction;
    double break_even_price;
    double average_fx_rate;
    double realized_product_pl;
    double realized_fx_pl;
    double today_realized_product_pl;
    double today_realized_fx_pl;
} dg_position;

typedef struct dg_portfolio {
    dg_position *positions;
    size_t count;
} dg_portfolio;

bool dg_get_portfolio(dg_context *ctx);
