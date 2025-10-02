#include "degiro_transactions.h"
#include "utils.h"
#include "nob.h"

bool dg__transaction_from_json_obj(dg_transaction *transaction, cJSON *obj)
{
    parse_int(   obj, "id",                             &transaction->id);
    parse_int(   obj, "productId",                      &transaction->product_id);
    parse_string(obj, "date",                           &transaction->date);
    parse_string(obj, "buysell",                        &transaction->buysell);
    parse_double(obj, "price",                          &transaction->price);
    parse_int(   obj, "quantity",                       &transaction->quantity);
    parse_double(obj, "total",                          &transaction->total);
    parse_int(   obj, "orderTypeId",                    &transaction->order_type_id);
    parse_string(obj, "counterParty",                   &transaction->counter_party);
    parse_bool(  obj, "transfered",                     &transaction->transfered);
    parse_int(   obj, "fxRate",                         &transaction->fx_rate);
    parse_int(   obj, "nettFxRate",                     &transaction->nett_fx_rate);
    parse_int(   obj, "grossFxRate",                    &transaction->gross_fx_rate);
    parse_int(   obj, "autoFxFeeInBaseCurrency",        &transaction->auto_fx_fee_in_base_currency);
    parse_double(obj, "totalInBaseCurrency",            &transaction->total_in_base_currency);
    parse_double(obj, "feeInBaseCurrency",              &transaction->fee_in_base_currency);
    parse_double(obj, "totalFeesInBaseCurrency",        &transaction->total_fees_in_base_currency);
    parse_double(obj, "totalPlusFeeInBaseCurrency",     &transaction->total_plus_fee_in_base_currency);
    parse_double(obj, "totalPlusAllFeesInBaseCurrency", &transaction->total_plus_all_fees_in_base_currency);
    parse_int(   obj, "transactionTypeId",              &transaction->transaction_type_id);
    parse_string(obj, "tradingVenue",                   &transaction->trading_venue);
    parse_string(obj, "executingEntityId",              &transaction->executing_entity_id);

    return true;
}

bool dg__transactions_from_json_str(dg_da_transactions *transactions, const char *json_string)
{
    cJSON *json = cJSON_Parse(json_string);
    if (json == NULL)
    {
        nob_log(NOB_ERROR, "Error parsing JSON");
        return false;
    }

    cJSON *items = cJSON_GetObjectItemCaseSensitive(json, "data");
    size_t n_transactions = (size_t)cJSON_GetArraySize(items);
    
    // Reset dynamic array and reserve space for the new transactions
    transactions->count = 0;
    nob_da_reserve(transactions, n_transactions);
    
    cJSON *item = NULL;
    cJSON_ArrayForEach(item, items)
    {
        dg_transaction t = {0};
        dg__transaction_from_json_obj(&t, item);
        nob_da_append(transactions, t);
    }
    
    nob_log(NOB_INFO, "%zu transactions received", n_transactions);

    return true;
}

