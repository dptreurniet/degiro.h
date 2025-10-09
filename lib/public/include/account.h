#pragma once

#include <stdlib.h>

typedef struct dg_get_account_overview_options {
    const char *from_date;
    const char *to_date;
} dg_get_account_overview_options;

typedef struct dg_balance {
    double unsettled_cash;
    double flatex_cash;
    double total;
} dg_balance;

typedef enum {
    DG_MOVE_TYPE_TRANSACTION,        // Buy or sell a product
    DG_MOVE_TYPE_CASH_TRANSATION,    // Deposit or withdraw cash
    DG_MOVE_TYPE_FLATEX_CASH_SWEEP,  // Back-end transations made by DeGiro
    DG_MOVE_TYPE_PAYMENT             // Used when deposit failed, maybe there are other use-cases as well
} dg_cash_move_type;

typedef struct dg_cash_move {
    long long int id;
    const char *order_id;
    int product_id;
    const char *date;
    const char *value_date;
    const char *description;
    const char *currency;
    dg_cash_move_type type;
    double change;
    dg_balance balance;
} dg_cash_move;

typedef struct dg_cash_moves {
    dg_cash_move *items;
    size_t count;
    size_t capacity;
} dg_cash_moves;

typedef struct dg_account_overview {
    dg_cash_moves cash_moves;
} dg_account_overview;
