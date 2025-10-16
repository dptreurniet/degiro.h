#include "degiro.h"

#include "defines.h"
#include "dg_curl.h"
#include "dg_types.h"
#include "dg_utils.h"

#define NOB_IMPLEMENTATION
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

    return true;
}

void dg_cleanup(dg_context *ctx) {
    nob_log(NOB_INFO, "Cleaning up degiro lib");
    curl_easy_cleanup(ctx->curl.curl);
    curl_global_cleanup();
}
