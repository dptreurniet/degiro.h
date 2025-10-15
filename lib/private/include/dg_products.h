#pragma once

#include <stddef.h>

#include "products.h"

typedef struct dg_da_products {
    dg_product *items;
    size_t count;
    size_t capacity;
} dg_da_products;
