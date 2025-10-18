#pragma once

#include <time.h>

typedef struct tm dg_datetime;

typedef enum {
    DG_CURRENCY_CHF,
    DG_CURRENCY_EUR,
    DG_CURRENCY_PLN,
    DG_CURRENCY_GBP,
    DG_CURRENCY_DKK,
    DG_CURRENCY_CZK,
    DG_CURRENCY_USD,
    DG_CURRENCY_SEK,
    DG_CURRENCY_HUF,
    DG_CURRENCY_NOK
} dg_currency;

const char* dg_currency_symbol(dg_currency currency);
const char* dg_currency_str(dg_currency currency);
