#pragma once
#include "products.h"
#include <time.h>

typedef enum {
    PERIOD_1D,
    PERIOD_1W,
    PERIOD_1M,
    PERIOD_6M,
    PERIOD_1Y,
} dg_chart_period;

const char* chart_period_to_str(dg_chart_period p);

typedef struct dg_product_chart_options {
    dg_product product;
    dg_chart_period period;
} dg_product_chart_options;

typedef struct dg_chart {
    double *prices;
    double *timestamps;
    size_t n_points;
} dg_chart;

typedef struct dg_product_chart {
    dg_product product;
    dg_chart chart;
    const char* currency;
    double low_price;
    double high_price;
} dg_product_chart;
