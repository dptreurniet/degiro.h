#pragma once

#include <stddef.h>

#include "dictionary.h"

typedef struct dg_da_exchanges {
    dg_exchange *items;
    size_t count;
    size_t capacity;
} dg_da_exchanges;
