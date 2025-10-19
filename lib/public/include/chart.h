#pragma once

#include <time.h>

#include "products.h"

// Forward-declare context
struct dg_context;
typedef struct dg_context dg_context;

typedef enum {
    PERIOD_1D,
    PERIOD_1W,
    PERIOD_1M,
    PERIOD_6M,
    PERIOD_1Y,
    PERIOD_3Y,
    PERIOD_5Y,
    PERIOD_YTD,
    PERIOD_MAX,
} dg_chart_period;

typedef struct dg_product_chart_options {
    dg_product product;
    dg_chart_period period;
} dg_product_chart_options;

typedef struct dg_chart_data {
    double *prices;
    double *timestamps;
    size_t n_points;
} dg_chart_data;

typedef struct dg_product_chart {
    dg_product product;
    dg_chart_data chart_data;
    char *currency;
    double low_price;
    double high_price;
} dg_product_chart;

bool dg_get_product_info_chart(dg_context *ctx, dg_product_chart_options opts, dg_product_chart *chart);
