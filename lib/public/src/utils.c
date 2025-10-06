#include "degiro.h"
#include "nob.h"

const char *dg_chart_period_to_str(dg_chart_period p) {
    switch (p) {
        case PERIOD_1D:
            return "1D";
        case PERIOD_1W:
            return "1W";
        case PERIOD_1M:
            return "1M";
        case PERIOD_1Y:
            return "1Y";
        default:
            NOB_UNREACHABLE("Undefined chart period");
    }
    return "";
}

bool dg_get_product_by_id(degiro *dg, int id, dg_product *result) {
    for (size_t i = 0; i < dg->products.count; ++i) {
        if (dg->products.items[i].id == id) {
            *result = dg->products.items[i];
            return true;
        }
    }
    return false;
}