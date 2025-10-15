#pragma once

#include <stdbool.h>

#include "transactions.h"

typedef struct dg_da_transactions {
    dg_transaction *items;
    size_t count;
    size_t capacity;
} dg_da_transactions;
