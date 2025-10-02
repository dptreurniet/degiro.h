#pragma once

#include <stdbool.h>
#include <cjson/cJSON.h>
#include "transactions.h"

typedef struct dg_da_transactions
{
  dg_transaction *items;
  size_t count;
  size_t capacity;
} dg_da_transactions;

bool dg__transactions_from_json_str(dg_da_transactions *transactions, const char *json_string);
bool dg__transaction_from_json_obj(dg_transaction *transaction, cJSON *obj);
