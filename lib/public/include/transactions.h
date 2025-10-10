#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "types.h"

// Forward-declare context
struct dg_context;
typedef struct dg_context dg_context;

typedef struct dg_get_transactions_options {
    dg_datetime from_date;
    dg_datetime to_date;
    bool group_by_order;
} dg_get_transactions_options;

typedef struct dg_transaction {
    int id;
    int product_id;
    char *date;
    char *buysell;
    double price;
    int quantity;
    double total;
    int order_type_id;
    char *counter_party;
    bool transfered;
    int fx_rate;
    int nett_fx_rate;
    int gross_fx_rate;
    int auto_fx_fee_in_base_currency;
    double total_in_base_currency;
    double fee_in_base_currency;
    double total_fees_in_base_currency;
    double total_plus_fee_in_base_currency;
    double total_plus_all_fees_in_base_currency;
    int transaction_type_id;
    char *trading_venue;
    char *executing_entity_id;
} dg_transaction;

typedef struct dg_transactions {
    dg_transaction *items;
    size_t count;
} dg_transactions;

bool dg_get_transactions(dg_context *ctx, dg_get_transactions_options options, dg_transactions *transactions);