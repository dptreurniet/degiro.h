#include "dg_types.h"

#include "nob.h"

dg_currency dg__currency_from_string(const char* str) {
    if (strcmp(str, "CHF") == 0) return DG_CURRENCY_CHF;
    if (strcmp(str, "EUR") == 0) return DG_CURRENCY_EUR;
    if (strcmp(str, "PLN") == 0) return DG_CURRENCY_PLN;
    if (strcmp(str, "GBP") == 0) return DG_CURRENCY_GBP;
    if (strcmp(str, "DKK") == 0) return DG_CURRENCY_DKK;
    if (strcmp(str, "CZK") == 0) return DG_CURRENCY_CZK;
    if (strcmp(str, "USD") == 0) return DG_CURRENCY_USD;
    if (strcmp(str, "SEK") == 0) return DG_CURRENCY_SEK;
    if (strcmp(str, "HUF") == 0) return DG_CURRENCY_HUF;
    if (strcmp(str, "NOK") == 0) return DG_CURRENCY_NOK;
    NOB_UNREACHABLE("Undefined currency");
}