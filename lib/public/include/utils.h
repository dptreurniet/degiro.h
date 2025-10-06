#pragma once

#include "degiro.h"

const char *dg_chart_period_to_str(dg_chart_period p);

bool dg_get_product_by_id(degiro *dg, int id, dg_product *result);