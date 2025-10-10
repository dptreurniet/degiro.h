#pragma once

#include "dg_curl.h"
#include "dg_user.h"

typedef struct dg_backend {
    dg_curl curl;
    dg_user_config user_config;
} dg_backend;

typedef struct dg_login_response {
    bool captcha_required;
    bool is_pass_code_enabled;
    char *locale;
    char *redirect_url;
    char *session_id;
    int status;
    char *status_text;
    // ignoring userTokens: []
} dg_login_response;

bool dg__parse_login_response(dg_context *ctx, dg_login_response *response);

// bool dg__get_portfolio(degiro *dg);
// bool dg__get_transactions(degiro *dg, dg_get_transactions_options options, dg_da_transactions *transactions);
// bool dg__get_user_config();
// bool dg__get_user_data();
// bool dg__get_account_overview(degiro *dg, dg_get_account_overview_options options, dg_account_overview *account_overview);
// bool dg__get_products_info(degiro *dg, int *ids, size_t n_ids, dg_products *products);
// bool dg__get_product_chart(dg_product_chart *history, dg_product_chart_options opts);
