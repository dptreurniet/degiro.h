#pragma once

#include "degiro.h"

bool dg__account_overview_from_json_string(dg_account_overview *acc_overview, const char *json_string);