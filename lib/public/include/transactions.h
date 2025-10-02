#pragma once

#include <stddef.h>

typedef struct dg_get_transactions_options
{
  const char *from_date;
  const char *to_date;
  bool group_transactions_by_order;
} dg_get_transactions_options;

typedef struct dg_transaction
{
  int id;
  int product_id;
  const char *date;
  const char *buysell;
  double price;
  int quantity;
  double total;
  int order_type_id;
  const char *counter_party;
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
  const char *trading_venue;
  const char *executing_entity_id;
} dg_transaction;

typedef struct dg_transactions
{
    dg_transaction *transactions;
    size_t count;
} dg_transactions;