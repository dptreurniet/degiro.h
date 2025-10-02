#pragma once

#include <stdbool.h>
#include <cjson/cJSON.h>
#include "prices.h"

bool dg__parse_price_response(cJSON *data, dg_price_history* result);