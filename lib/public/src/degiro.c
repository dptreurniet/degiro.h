#include "degiro.h"

#include "defines.h"
#include "dg_curl.h"
#include "dg_types.h"
#include "dg_utils.h"

#define NOB_IMPLEMENTATION
#include "nob.h"

bool dg__parse_user_config(dg_context *ctx) {
    cJSON *json = cJSON_Parse(ctx->curl.response.data);
    if (json == NULL) {
        nob_log(NOB_ERROR, "Error parsing JSON");
        return false;
    }

    cJSON *data = cJSON_GetObjectItem(json, "data");
    if (!cJSON_IsObject(data)) {
        nob_log(NOB_ERROR, "No \"data\" field in JSON");
        return false;
    }

    dg__parse_string(data, "betaLandingPath", &ctx->user_config.beta_landing_path);
    dg__parse_string(data, "cashSolutionsUrl", &ctx->user_config.cash_solutions_url);
    dg__parse_int(data, "clientId", &ctx->user_config.client_id);
    dg__parse_string(data, "companiesServiceUrl", &ctx->user_config.companies_service_url);
    dg__parse_string(data, "dictionaryUrl", &ctx->user_config.dictionary_url);
    dg__parse_string(data, "firstLoginWizardUrl", &ctx->user_config.first_login_wizard_url);
    dg__parse_string(data, "i18nUrl", &ctx->user_config.i18n_url);
    dg__parse_string(data, "landingPath", &ctx->user_config.landing_path);
    dg__parse_string(data, "loginUrl", &ctx->user_config.login_url);
    dg__parse_string(data, "mobileLandingPath", &ctx->user_config.mobile_landing_path);
    dg__parse_string(data, "paUrl", &ctx->user_config.pa_url);
    dg__parse_string(data, "paymentServiceUrl", &ctx->user_config.payment_service_url);
    dg__parse_string(data, "productSearchUrl", &ctx->user_config.product_search_url);
    dg__parse_string(data, "productTypesUrl", &ctx->user_config.product_types_url);
    dg__parse_string(data, "reportingUrl", &ctx->user_config.reporting_url);
    dg__parse_string(data, "sessionId", &ctx->user_config.session_id);
    dg__parse_string(data, "taskManagerUrl", &ctx->user_config.task_manager_url);
    dg__parse_string(data, "tradingUrl", &ctx->user_config.trading_url);
    dg__parse_string(data, "vwdGossipsUrl", &ctx->user_config.vwd_gossips_url);
    dg__parse_string(data, "vwdNewsUrl", &ctx->user_config.vwd_news_url);
    dg__parse_string(data, "vwdQuotecastServiceUrl", &ctx->user_config.vwd_quotecast_service_url);

    cJSON_Delete(json);
    return true;
}

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

bool dg_init(dg_context *ctx) {
    nob_log(NOB_INFO, "Initializing DeGiro");
    curl_global_init(CURL_GLOBAL_DEFAULT);

    ctx->curl.curl = curl_easy_init();
    if (!ctx->curl.curl) {
        nob_log(NOB_ERROR, "Failed to init Curl");
        return false;
    }

    curl_easy_setopt(ctx->curl.curl, CURLOPT_WRITEFUNCTION, dg__curl_callback);
    curl_easy_setopt(ctx->curl.curl, CURLOPT_WRITEDATA, &ctx->curl.response);
    if (!dg__set_default_curl_headers(ctx)) return false;

    return true;
}

bool dg_login(dg_context *ctx, dg_login_data login) {
    nob_log(NOB_INFO, "Logging in at DeGiro");
    CURLcode res;
    ctx->logged_in = false;

    dg__set_curl_POST(ctx);
    if (!dg__set_default_curl_headers(ctx)) return false;
    if (!dg__set_curl_url(ctx, dg__format_string("%s%s/totp", DEGIRO_BASE_URL, DEGIRO_LOGIN_URL))) return false;
    dg__set_curl_payload(ctx, dg__format_string("{\"username\":\"%s\",\"password\":\"%s\",\"oneTimePassword\":\"%s\"}",
                                                login.username, login.password, login.totp));

    res = dg__make_request(ctx);
    if (res != CURLE_OK) {
        nob_log(NOB_ERROR, "Curl request failed: %s", curl_easy_strerror(res));
        return false;
    }

    dg_login_response response = {0};
    if (!dg__parse_login_response(ctx, &response)) {
        nob_log(NOB_ERROR, "Failed to parse login response");
        return false;
    }

    if (response.status != 0) {
        nob_log(NOB_ERROR, "Login failed: %s", response.status_text);
        return false;
    } else {
        nob_log(NOB_INFO, "Credentials OK");
    }

    // Automatically get user_config after login, as this struct stores the session ID and contains URLs for subsequent API calls
    ctx->user_config.session_id = response.session_id;
    if (!dg__set_default_curl_headers(ctx)) return false;
    if (!dg__set_curl_url(ctx, dg__format_string("%s%s", DEGIRO_BASE_URL, DEGIRO_GET_USER_CONFIG_URL))) return false;
    dg__set_curl_payload(ctx, "");
    dg__set_curl_GET(ctx);

    res = dg__make_request(ctx);
    if (res != CURLE_OK) {
        nob_log(NOB_ERROR, "Curl request failed: %s", curl_easy_strerror(res));
        return false;
    }

    if (!dg__parse_user_config(ctx)) {
        nob_log(NOB_ERROR, "Failed to parse user config");
        return false;
    }

    ctx->logged_in = true;
    nob_log(NOB_INFO, "Successful login");

    // Get account_info as well, because some calls require the int_account number that is stored in there.
    if (!dg_get_user_data(ctx)) {
        nob_log(NOB_ERROR, "Failed to get user data");
        return false;
    }

    // Get dicionary that contains a lot of info needed for later calls
    if (!dg_get_dictionary(ctx)) {
        nob_log(NOB_ERROR, "Failed to get dictionary");
        return false;
    }

    return true;
}

void dg_cleanup(dg_context *ctx) {
    nob_log(NOB_INFO, "Cleaning up degiro lib");
    curl_easy_cleanup(ctx->curl.curl);
    curl_global_cleanup();
}
