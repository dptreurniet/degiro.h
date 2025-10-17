#pragma once

#include "degiro.h"

// Dictionary lookups
dg_exchange *dg_lookup_exchange_by_id(dg_context *ctx, int id);
