#include "transactions.h"

#include <curl/curl.h>

#include "defines.h"
#include "degiro.h"
#include "dg_curl.h"
#include "dg_transactions.h"
#include "dg_utils.h"
#include "nob.h"

bool dg_get_transactions(dg_context *ctx, dg_get_transactions_options options, dg_transactions *transactions) {
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
    transactions->items = (dg_transaction *)malloc(sizeof(dg_transaction) * t.count);
    if (!transactions->items) {
        nob_log(NOB_ERROR, "Failed to allocate memory for transactions");
        return false;
    }

    transactions->count = t.count;
    for (size_t i = 0; i < t.count; ++i)
        transactions->items[i] = t.items[i];

    return true;
}
