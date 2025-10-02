#pragma once
#include "products.h"
#include <time.h>

typedef struct dg_price_plot_options {
    dg_product* product;
    const char* period;
} dg_price_plot_options;

typedef struct dg_price_history_chart {
    double *prices;
    double *timestamps;
    size_t n_points;
} dg_price_history_chart;

typedef struct dg_price_history {
    dg_product* product;
    dg_price_history_chart chart;
    const char* currency;
    double low_price;
    double high_price;
} dg_price_history;
