#include "dg_transactions.h"

#include "degiro.h"
#include "dg_utils.h"
#include "nob.h"

bool dg__parse_transactions(dg_context *ctx, dg_da_transactions *transactions) {
    cJSON *json = cJSON_Parse(ctx->curl.response.data);
    if (json == NULL) {
        nob_log(NOB_ERROR, "Error parsing JSON");
        return false;
    }

    cJSON *items = cJSON_GetObjectItemCaseSensitive(json, "data");
    size_t n_transactions = (size_t)cJSON_GetArraySize(items);

    transactions->count = 0;
    nob_da_reserve(transactions, n_transactions);

    cJSON *item = NULL;
    cJSON_ArrayForEach(item, items) {
        dg_transaction t = {0};

        dg__parse_int(item, "id", &t.id);
        dg__parse_int(item, "productId", &t.product_id);
        dg__parse_string(item, "date", &t.date);
        dg__parse_string(item, "buysell", &t.buysell);
        dg__parse_double(item, "price", &t.price);
        dg__parse_int(item, "quantity", &t.quantity);
        dg__parse_double(item, "total", &t.total);
        dg__parse_int(item, "orderTypeId", &t.order_type_id);
        dg__parse_string(item, "counterParty", &t.counter_party);
        dg__parse_bool(item, "transfered", &t.transfered);
        dg__parse_int(item, "fxRate", &t.fx_rate);
        dg__parse_int(item, "nettFxRate", &t.nett_fx_rate);
        dg__parse_int(item, "grossFxRate", &t.gross_fx_rate);
        dg__parse_int(item, "autoFxFeeInBaseCurrency", &t.auto_fx_fee_in_base_currency);
        dg__parse_double(item, "totalInBaseCurrency", &t.total_in_base_currency);
        dg__parse_double(item, "feeInBaseCurrency", &t.fee_in_base_currency);
        dg__parse_double(item, "totalFeesInBaseCurrency", &t.total_fees_in_base_currency);
        dg__parse_double(item, "totalPlusFeeInBaseCurrency", &t.total_plus_fee_in_base_currency);
        dg__parse_double(item, "totalPlusAllFeesInBaseCurrency", &t.total_plus_all_fees_in_base_currency);
        dg__parse_int(item, "transactionTypeId", &t.transaction_type_id);
        dg__parse_string(item, "tradingVenue", &t.trading_venue);
        dg__parse_string(item, "executingEntityId", &t.executing_entity_id);

        nob_da_append(transactions, t);
    }

    nob_log(NOB_INFO, "%zu transactions received", n_transactions);

    return true;
}
