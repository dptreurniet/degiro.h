#include "transactions.h"

#include <curl/curl.h>

#include "defines.h"
#include "degiro.h"
#include "dg_curl.h"
#include "dg_types.h"
#include "dg_utils.h"
#include "nob.h"

bool dg__parse_buysell(cJSON* root, const char* key, dg_buysell* destination) {
    char* str;
    dg__parse_string(root, key, &str);
    if (strcmp(str, "B") == 0) {
        *destination = BUY;
        return true;
    }
    if (strcmp(str, "S") == 0) {
        *destination = SELL;
        return true;
    }

    nob_log(NOB_ERROR, "Expected \"S\" or \"B\", but found \"%s\"", str);
    return false;
}

bool dg__parse_counterparty(cJSON* root, const char* key, dg_buysell* destination) {
    char* str;
    dg__parse_string(root, key, &str);
    if (strcmp(str, "MK") == 0) {
        *destination = COUNTERPARTY_MK;
        return true;
    }

    nob_log(NOB_ERROR, "Expected \"MK\", but found \"%s\"", str);
    NOB_TODO("Implement missing counterpary");
    return false;
}

bool dg__parse_transactions(dg_context* ctx, dg_da_transactions* transactions) {
    cJSON* json = cJSON_Parse(ctx->curl.response.data);
    if (json == NULL) {
        nob_log(NOB_ERROR, "Error parsing JSON");
        return false;
    }

    cJSON* items = cJSON_GetObjectItemCaseSensitive(json, "data");
    size_t n_transactions = (size_t)cJSON_GetArraySize(items);

    transactions->count = 0;
    nob_da_reserve(transactions, n_transactions);

    cJSON* item = NULL;
    cJSON_ArrayForEach(item, items) {
        dg_transaction t = {0};

        dg__parse_int(item, "id", &t.id);
        dg__parse_int(item, "productId", &t.product_id);
        dg__parse_string(item, "date", &t.date);
        dg__parse_buysell(item, "buysell", &t.buysell);
        dg__parse_double(item, "price", &t.price);
        dg__parse_int(item, "quantity", &t.quantity);
        dg__parse_double(item, "total", &t.total);
        dg__parse_int(item, "orderTypeId", &t.order_type_id);
        dg__parse_counterparty(item, "counterParty", &t.counter_party);
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

bool dg_get_transactions(dg_context* ctx, dg_get_transactions_options options) {
    nob_log(NOB_INFO, "Getting transactions...");

    char to_date_buffer[11];  // "YYYY-MM-DD" + null terminator
    strftime(to_date_buffer, sizeof(to_date_buffer), "%Y-%m-%d", &options.to_date);
    char from_date_buffer[11];
    strftime(from_date_buffer, sizeof(to_date_buffer), "%Y-%m-%d", &options.from_date);

    if (!dg__set_default_curl_headers(ctx)) return false;
    dg__set_curl_url(ctx, dg__format_string("%s%s?fromDate=%s&toDate=%s&groupTransactionsByOrder=%s&intAccount=%d&sessionId=%s",
                                            ctx->user_config.reporting_url,
                                            DEGIRO_GET_TRANSACTIONS_URL,
                                            from_date_buffer,
                                            to_date_buffer,
                                            options.group_by_order ? "true" : "false",
                                            ctx->user_data.int_account,
                                            ctx->user_config.session_id));
    dg__set_curl_payload(ctx, "");
    dg__set_curl_GET(ctx);

    CURLcode res = dg__make_request(ctx);
    if (res != CURLE_OK) {
        nob_log(NOB_ERROR, "Curl request failed: %s", curl_easy_strerror(res));
        return false;
    }

    dg_da_transactions t = {0};
    dg__parse_transactions(ctx, &t);

    // Get product info of the products in transactions
    // int *ids = (int *)malloc(sizeof(int) * transactions->count);
    // for (size_t i = 0; i < transactions->count; i++) {
    //     ids[i] = transactions->items[i].product_id;
    // }
    // dg_products products = {0};
    // dg__get_products_info(dg, ids, transactions->count, &products);

    // Copy items from dynamic array to user-managed struct
    ctx->transactions.items = (dg_transaction*)malloc(sizeof(dg_transaction) * t.count);
    if (!ctx->transactions.items) {
        nob_log(NOB_ERROR, "Failed to allocate memory for transactions");
        return false;
    }

    ctx->transactions.count = t.count;
    for (size_t i = 0; i < t.count; ++i)
        ctx->transactions.items[i] = t.items[i];

    return true;
}
