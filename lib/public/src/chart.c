#include "chart.h"
#include "nob.h"

const char* chart_period_to_str(dg_chart_period p)
{
    switch(p)
    {
    case PERIOD_1D: return "1D";
    case PERIOD_1W: return "1W";
    case PERIOD_1M: return "1M";
    case PERIOD_1Y: return "1Y";
    default:
        NOB_UNREACHABLE("Undefined chart period");
    }
    return "";
}