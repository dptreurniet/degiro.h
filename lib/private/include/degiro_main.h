#pragma once

#include <stdbool.h>

// public headers
#include "transactions.h"

// private headers
#include "degiro_curl.h"
#include "degiro_account.h"
#include "degiro_portfolio.h"
#include "degiro_products.h"
#include "degiro_transactions.h"

typedef struct dg_backend {
  dg_curl curl;
  dg_account_config account_config;
} dg_backend;

extern dg_backend dgb;

bool dg__init();
bool dg__login(degiro *dg, const char *username, const char *password, const char *totp);

bool dg__get_portfolio(degiro *dg);
bool dg__get_transactions(degiro *dg, dg_get_transactions_options options, dg_da_transactions *transactions);
bool dg__get_account_config();
bool dg__get_account_data();
bool dg__get_products_info(degiro *dg, int *ids, size_t n_ids, dg_products *products);
bool dg__get_product_chart(dg_product_chart *history, dg_product_chart_options opts);


void dg__cleanup();