#include "types.h"

#include "nob.h"

const char* dg_currency_symbol(dg_currency currency) {
    switch (currency) {
        case DG_CURRENCY_CHF:
            return "CHF";
        case DG_CURRENCY_EUR:
            return "€";
        case DG_CURRENCY_PLN:
            return "zł";
        case DG_CURRENCY_GBP:
            return "£";
        case DG_CURRENCY_DKK:
            return "kr.";
        case DG_CURRENCY_CZK:
            return "Kč";
        case DG_CURRENCY_USD:
            return "$";
        case DG_CURRENCY_SEK:
            return "kr";
        case DG_CURRENCY_HUF:
            return "Ft";
        case DG_CURRENCY_NOK:
            return "kr";
        default:
            NOB_UNREACHABLE("Undefined currency");
    }
}

const char* dg_currency_str(dg_currency currency) {
    switch (currency) {
        case DG_CURRENCY_CHF:
            return "CHF";
        case DG_CURRENCY_EUR:
            return "EUR";
        case DG_CURRENCY_PLN:
            return "PLN";
        case DG_CURRENCY_GBP:
            return "GBP";
        case DG_CURRENCY_DKK:
            return "DKK";
        case DG_CURRENCY_CZK:
            return "CZK";
        case DG_CURRENCY_USD:
            return "USD";
        case DG_CURRENCY_SEK:
            return "SEK";
        case DG_CURRENCY_HUF:
            return "HUF";
        case DG_CURRENCY_NOK:
            return "NOK";
        default:
            NOB_UNREACHABLE("Undefined currency");
    }
}
