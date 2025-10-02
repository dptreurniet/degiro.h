#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "portfolio.h"

bool dg__portfolio_from_json_string(dg_portfolio *portfolio, const char *json_string);