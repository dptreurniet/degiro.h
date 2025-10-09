#include "degiro.h"

#include <stdlib.h>

#include "degiro_main.h"
#include "nob.h"

bool dg_init() {
    return dg__init();
}

bool dg_login(degiro *dg, const char *username, const char *password, const char *totp) {
    return dg__login(dg, username, password, totp);
}

bool dg_get_portfolio(degiro *dg) {
    return dg__get_portfolio(dg);
}

bool dg_get_transactions(degiro *dg, dg_get_transactions_options options, dg_transactions *transactions) {
    dg_da_transactions t = {0};
    if (!dg__get_transactions(dg, options, &t)) {
        nob_log(NOB_ERROR, "Failed to get transactions");
        return false;
    }

    transactions->items = (dg_transaction *)malloc(sizeof(dg_transaction) * t.count);
    if (!transactions->items) {
        nob_log(NOB_ERROR, "Failed to allocate memory for transactions");
        return false;
    }

    for (size_t i = 0; i < t.count; ++i)
        transactions->items[i] = t.items[i];

    transactions->count = t.count;

    return true;
}

bool dg_get_account_overview(degiro *dg, dg_get_account_overview_options options, dg_account_overview *account_overview) {
    return dg__get_account_overview(dg, options, account_overview);
}

bool dg_get_product_chart(dg_product_chart *result, dg_product_chart_options opts) {
    return dg__get_product_chart(result, opts);
}

void dg_cleanup() {
    dg__cleanup();
}