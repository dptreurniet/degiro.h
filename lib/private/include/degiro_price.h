#pragma once

#include <stdbool.h>
#include <cjson/cJSON.h>
#include "chart.h"

bool dg__parse_chart_response(cJSON *data, dg_product_chart* result);