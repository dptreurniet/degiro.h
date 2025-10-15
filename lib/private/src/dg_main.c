#include "dg_main.h"

#include <cjson/cJSON.h>

#include "degiro.h"
#include "dg_utils.h"
#include "nob.h"

bool dg__parse_login_response(dg_context *ctx, dg_login_response *response) {
    cJSON *json = cJSON_Parse(ctx->curl.response.data);
    if (json == NULL) {
        nob_log(NOB_ERROR, "Failed to parse JSON");
        return false;
    }

    dg__parse_bool(json, "captchaRequired", &response->captcha_required);
    dg__parse_bool(json, "isPassCodeEnabled", &response->is_pass_code_enabled);
    dg__parse_string(json, "locale", &response->locale);
    dg__parse_string(json, "redirectUrl", &response->redirect_url);
    dg__parse_string(json, "sessionId", &response->session_id);
    dg__parse_int(json, "status", &response->status);
    dg__parse_string(json, "statusText", &response->status_text);

    cJSON_Delete(json);
    return true;
}

/*
bool dg__get_portfolio(degiro *dg) {
    CURLcode res;

    nob_log(NOB_INFO, "Getting portfolio");
    if (!dgb.user_config.session_id) {
        nob_log(NOB_ERROR, "No session id defined");
        return false;
    }

    dg__set_default_curl_headers(&dgb.curl, dgb.user_config.session_id);
    const char *url = dg__format_string("%sv5/update/%d;jsessionid=%s?&portfolio=0",
                                    dgb.user_config.trading_url,
                                    dg->user_data.int_account,
                                    dgb.user_config.session_id);
    dg__set_curl_url(&dgb.curl, url);
    dg__set_curl_payload(&dgb.curl, "");
    dg__set_curl_GET(&dgb.curl);

    res = dg__make_request(&dgb.curl);
    if (res != CURLE_OK) {
        nob_log(NOB_ERROR, "curl_easy_perform() failed: %s", curl_easy_strerror(res));
        return false;
    }

    dg__portfolio_from_json_string(&dg->portfolio, dgb.curl.response.data);

    // Generate array of number-only ids (to ignore cash positions)
    size_t n_ids = 0;
    for (size_t i = 0; i < dg->portfolio.count; i++) {
        if (is_only_numbers(dg->portfolio.items[i].id)) {
            n_ids++;
        }
    }

    int *ids = (int *)malloc(sizeof(int) * dg->portfolio.count);
    for (size_t i = 0; i < dg->portfolio.count; i++) {
        if (is_only_numbers(dg->portfolio.items[i].id)) {
            ids[i] = atoi(dg->portfolio.items[i].id);
        }
    }

    dg_products products = {0};
    dg__get_products_info(dg, ids, n_ids, &products);

    free(ids);
    return true;
}

bool dg__get_product_chart(dg_product_chart *result, dg_product_chart_options opts) {
    // https://charting.vwdservices.com/hchart/v1/deGiro/data.js?requestid=1&resolution=P1D&culture=nl-NL&period=P50Y&series=issueid%3A485013849&series=price%3Aissueid%3A485013849&format=json&callback=vwd.hchart.seriesRequestManager.sync_response&userToken=4626342&tz=Europe%2FAmsterdam

    result->product = opts.product;

    const char *resolution = "1D";
    switch (opts.period) {
        case PERIOD_1D:
            resolution = "T1M";
            break;
        case PERIOD_1W:
            resolution = "T30M";
            break;
        case PERIOD_1M:
            resolution = "T2H";
            break;
        case PERIOD_1Y:
            resolution = "1D";
            break;
        default:
            NOB_UNREACHABLE("Undefined period");
    }

    nob_log(NOB_INFO, "Getting price info with the following options:");
    nob_log(NOB_INFO, " - Product: %s", opts.product.name);
    nob_log(NOB_INFO, " - Period: %s", dg_chart_period_to_str(opts.period));
    nob_log(NOB_INFO, " - Resolution: %s", resolution);

    Nob_String_Builder url = {0};
    nob_sb_appendf(&url, "%s?", DEGIRO_GET_CHART_URL);
    nob_sb_appendf(&url, "series=%s:%s&", opts.product.vwd_identifier_type, opts.product.vwd_id);
    nob_sb_appendf(&url, "series=price:%s:%s&", opts.product.vwd_identifier_type, opts.product.vwd_id);
    nob_sb_appendf(&url, "period=P%s&", dg_chart_period_to_str(opts.period));
    nob_sb_appendf(&url, "resolution=P%s&", resolution);
    nob_sb_appendf(&url, "userToken=%d&", dgb.user_config.client_id);
    nob_sb_appendf(&url, "culture=nl-NL&");
    nob_sb_appendf(&url, "format=json&");
    nob_sb_appendf(&url, "tz=Europe/Amsterdam");
    nob_sb_append_null(&url);

    dg__set_curl_url(&dgb.curl, url.items);
    dg__set_curl_GET(&dgb.curl);
    dg__make_request(&dgb.curl);

    cJSON *json = cJSON_Parse(dgb.curl.response.data);
    if (json == NULL) {
        nob_log(NOB_ERROR, "Error parsing JSON");
        return false;
    }

    return dg__parse_chart_response(json, result);
}

bool dg__get_account_overview(degiro *dg, dg_get_account_overview_options options, dg_account_overview *account_overview) {
    nob_log(NOB_INFO, "Getting account overview");

    options.from_date = "01/01/1900";
    options.to_date = "01/01/2100";

    Nob_String_Builder url = {0};
    nob_sb_appendf(&url, "%s", DEGIRO_BASE_URL);
    nob_sb_appendf(&url, "%s?", DEGIRO_GET_ACCOUNT_OVERVIEW_URL);
    nob_sb_appendf(&url, "fromDate=%s&", options.from_date);
    nob_sb_appendf(&url, "toDate=%s&", options.to_date);
    nob_sb_appendf(&url, "intAccount=%d&", dg->user_data.int_account);
    nob_sb_appendf(&url, "sessionId=%s&", dgb.user_config.session_id);
    nob_sb_append_null(&url);

    dg__set_curl_url(&dgb.curl, url.items);
    dg__set_curl_GET(&dgb.curl);
    dg__make_request(&dgb.curl);

    return dg__account_overview_from_json_string(account_overview, dgb.curl.response.data);
}


*/