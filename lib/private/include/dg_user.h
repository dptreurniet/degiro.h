#pragma once

#include <stdbool.h>

#include "degiro.h"
#include "user.h"

// Forward-declare context
struct dg_context;
typedef struct dg_context dg_context;

bool dg__parse_user_config(dg_context *ctx);
bool dg__parse_user_data(dg_context *ctx);
