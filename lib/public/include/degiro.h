#pragma once

#include <stdbool.h>

#include "transactions.h"
#include "products.h"
#include "account.h"
#include "portfolio.h"
#include "chart.h"

// State struct exposed to end-user
typedef struct degiro {
  bool logged_in;
  dg_account_data account_data;
  dg_product_library products;
  dg_portfolio portfolio;
} degiro;

bool dg_init();
bool dg_login(degiro *dg, const char *username, const char *password, const char *totp);

bool dg_get_portfolio(degiro *dg);
bool dg_get_transactions(degiro *dg, dg_get_transactions_options options, dg_transactions *transactions);
bool dg_get_product_chart(dg_product_chart *result, dg_product_chart_options opts);

void dg_cleanup();

#include "utils.h"
