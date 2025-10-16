#pragma once

#include <stdbool.h>

#include "curl.h"
#include "products.h"
#include "user.h"

typedef struct dg_user_config {
    char *beta_landing_path;
    char *cash_solutions_url;
    int client_id;
    char *companies_service_url;
    char *dictionary_url;
    char *first_login_wizard_url;
    char *i18n_url;
    char *landing_path;
    char *login_url;
    char *mobile_landing_path;
    char *pa_url;
    char *payment_service_url;
    char *product_search_url;
    char *product_types_url;
    char *reporting_url;
    char *session_id;
    char *task_manager_url;
    char *trading_url;
    char *vwd_gossips_url;
    char *vwd_news_url;
    char *vwd_quotecast_service_url;
} dg_user_config;

typedef struct dg_context {
    bool logged_in;
    dg_curl curl;
    dg_user_config user_config;
    dg_user_data user_data;
    dg_products products;
} dg_context;

typedef struct dg_login_data {
    const char *username;
    const char *password;
    const char *totp;
} dg_login_data;

bool dg_init(dg_context *ctx);
bool dg_login(dg_context *ctx, dg_login_data login);
void dg_cleanup(dg_context *ctx);

// Include other headers to allow users to only include "degiro.h"
#include "chart.h"
#include "transactions.h"