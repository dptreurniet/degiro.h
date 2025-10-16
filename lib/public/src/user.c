#include "user.h"

#include "degiro.h"
#include "dg_curl.h"
#include "dg_types.h"
#include "dg_utils.h"
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

bool dg__parse_user_data(dg_context *ctx) {
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

    dg__parse_bool(data, "canUpgrade", &ctx->user_data.can_upgrade);
    dg__parse_string(data, "cellphoneNumber", &ctx->user_data.cellphone_number);
    dg__parse_string(data, "clientRole", &ctx->user_data.client_role);
    dg__parse_string(data, "contractType", &ctx->user_data.contract_type);
    dg__parse_string(data, "culture", &ctx->user_data.culture);
    dg__parse_string(data, "displayName", &ctx->user_data.display_name);
    dg__parse_string(data, "effectiveClientRole", &ctx->user_data.effective_client_role);
    dg__parse_string(data, "email", &ctx->user_data.email);
    dg__parse_int(data, "id", &ctx->user_data.id);
    dg__parse_int(data, "intAccount", &ctx->user_data.int_account);
    dg__parse_bool(data, "isAllocationAvailable", &ctx->user_data.is_allocation_available);
    dg__parse_bool(data, "isAmClientActive", &ctx->user_data.is_am_client_active);
    dg__parse_bool(data, "isCollectivePortfolio", &ctx->user_data.is_collective_portfolio);
    dg__parse_bool(data, "isIskClient", &ctx->user_data.is_isk_client);
    dg__parse_bool(data, "isWithdrawalAvailable", &ctx->user_data.is_withdrawal_available);
    dg__parse_string(data, "language", &ctx->user_data.language);
    dg__parse_string(data, "locale", &ctx->user_data.locale);
    dg__parse_string(data, "memberCode", &ctx->user_data.member_code);
    dg__parse_string(data, "username", &ctx->user_data.username);

    {
        // Address struct
        cJSON *address = cJSON_GetObjectItem(data, "address");
        if (!cJSON_IsObject(data)) {
            nob_log(NOB_ERROR, "No \"address\" field in JSON");
            return false;
        }

        dg__parse_string(address, "city", &ctx->user_data.address.city);
        dg__parse_string(address, "country", &ctx->user_data.address.country);
        dg__parse_string(address, "streetAddress", &ctx->user_data.address.street_address);
        dg__parse_string(address, "streetAddressNumber", &ctx->user_data.address.street_address_number);
        dg__parse_string(address, "zip", &ctx->user_data.address.zip);
    }

    {
        // Address struct
        cJSON *bank_account = cJSON_GetObjectItem(data, "bankAccount");
        if (!cJSON_IsObject(bank_account)) {
            nob_log(NOB_ERROR, "No \"bankAccount\" field in JSON");
            return false;
        }

        dg__parse_int(bank_account, "bankAccountId", &ctx->user_data.bank_account.bank_account_id);
        dg__parse_string(bank_account, "bic", &ctx->user_data.bank_account.bic);
        dg__parse_string(bank_account, "iban", &ctx->user_data.bank_account.iban);
        dg__parse_string(bank_account, "status", &ctx->user_data.bank_account.status);
    }

    {
        // First contact struct
        cJSON *first_contact = cJSON_GetObjectItem(data, "firstContact");
        if (!cJSON_IsObject(first_contact)) {
            nob_log(NOB_ERROR, "No \"firstContact\" field in JSON");
            return false;
        }

        dg__parse_string(first_contact, "countryOfBirth", &ctx->user_data.first_contact.country_of_birth);
        dg__parse_string(first_contact, "dateOfBirth", &ctx->user_data.first_contact.date_of_birth);
        dg__parse_string(first_contact, "displayName", &ctx->user_data.first_contact.display_name);
        dg__parse_string(first_contact, "firstName", &ctx->user_data.first_contact.first_name);
        dg__parse_string(first_contact, "gender", &ctx->user_data.first_contact.gender);
        dg__parse_string(first_contact, "lastName", &ctx->user_data.first_contact.last_name);
        dg__parse_string(first_contact, "nationality", &ctx->user_data.first_contact.nationality);
        dg__parse_string(first_contact, "placeOfBirth", &ctx->user_data.first_contact.place_of_birth);
    }

    cJSON_Delete(json);
    return true;
}

bool dg_get_user_data(dg_context *ctx) {
    nob_log(NOB_INFO, "Getting user info");
    if (!ctx->user_config.session_id) {
        nob_log(NOB_ERROR, "No session ID defined");
        return false;
    }

    if (!dg__set_default_curl_headers(ctx)) return false;
    dg__set_curl_payload(ctx, "");
    dg__set_curl_GET(ctx);
    dg__set_curl_url(ctx, dg__format_string("%sclient?sessionId=%s", ctx->user_config.pa_url, ctx->user_config.session_id));

    CURLcode res = dg__make_request(ctx);
    if (res != CURLE_OK) {
        nob_log(NOB_ERROR, "curl_easy_perform() failed: %s", curl_easy_strerror(res));
        return false;
    }

    if (!dg__parse_user_data(ctx)) {
        nob_log(NOB_ERROR, "Failed to prase user data");
        return false;
    }

    return true;
}
