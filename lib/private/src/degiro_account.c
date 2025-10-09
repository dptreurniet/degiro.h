#include "degiro_account.h"

#include <cjson/cJSON.h>

#include "degiro_utils.h"
#include "nob.h"

bool dg__account_overview_from_json_string(dg_account_overview *acc_overview, const char *json_string) {
    cJSON *json = cJSON_Parse(json_string);
    if (json == NULL) {
        nob_log(NOB_ERROR, "Failed to parse string into JSON object");
        return false;
    }

    cJSON *data = cJSON_GetObjectItem(json, "data");
    if (!cJSON_IsObject(data)) {
        nob_log(NOB_ERROR, "JSON item \"data\" is expected to be an object, but is not");
        return false;
    }

    cJSON *cash_movements = cJSON_GetObjectItem(data, "cashMovements");
    if (!cJSON_IsArray(cash_movements)) {
        nob_log(NOB_ERROR, "JSON item \"cashMovements\" is expected to be an array, but is not");
        return false;
    }

    int n_moves = cJSON_GetArraySize(cash_movements);
    nob_da_reserve(&acc_overview->cash_moves, (size_t)n_moves);

    for (int i = 0; i < n_moves; i++) {
        cJSON *item = cJSON_GetArrayItem(cash_movements, i);
        dg_cash_move move = {0};

        char *type_str;
        parse_string(item, "type", &type_str);
        if (strcmp(type_str, "TRANSACTION") == 0)
            move.type = DG_MOVE_TYPE_TRANSACTION;
        else if (strcmp(type_str, "CASH_TRANSACTION") == 0)
            move.type = DG_MOVE_TYPE_CASH_TRANSATION;
        else if (strcmp(type_str, "FLATEX_CASH_SWEEP") == 0)
            move.type = DG_MOVE_TYPE_FLATEX_CASH_SWEEP;
        else if (strcmp(type_str, "PAYMENT") == 0)
            move.type = DG_MOVE_TYPE_PAYMENT;
        else {
            nob_log(NOB_WARNING, "Unknown transaction encountered: \"%s\"", type_str);
            continue;
        }

        parse_string(item, "date", &move.date);
        parse_string(item, "valueDate", &move.value_date);
        parse_long_long_int(item, "id", &move.id);
        parse_string(item, "description", &move.description);
        parse_double(item, "change", &move.change);
        parse_string(item, "orderId", &move.order_id);
        parse_int(item, "productId", &move.product_id);
        parse_string(item, "currency", &move.currency);

        cJSON *balance_item = cJSON_GetObjectItem(item, "balance");
        if (!cJSON_IsObject(data)) {
            nob_log(NOB_ERROR, "JSON item \"balance\" is expected to be an object, but is not");
            return false;
        }

        dg_balance balance = {0};
        parse_double(balance_item, "unsettledCash", &balance.unsettled_cash);
        parse_double(balance_item, "total", &balance.total);
        parse_double(balance_item, "flatexCash", &balance.flatex_cash);
        move.balance = balance;

        nob_da_append(&acc_overview->cash_moves, move);
    }

    return true;
}
