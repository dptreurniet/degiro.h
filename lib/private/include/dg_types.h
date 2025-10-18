/*
 * Filename: dg_types.h
 * Description: Contains definitions for types used in the degiro.h library but should not be exposed to the end-user.
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "portfolio.h"
#include "products.h"
#include "transactions.h"

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

typedef struct dg_da_products {
    dg_product *items;
    size_t count;
    size_t capacity;
} dg_da_products;

typedef struct dg_da_transactions {
    dg_transaction *items;
    size_t count;
    size_t capacity;
} dg_da_transactions;

typedef struct dg_da_positions {
    dg_position *items;
    size_t count;
    size_t capacity;
} dg_da_positions;

dg_currency dg__currency_from_string(const char *str);