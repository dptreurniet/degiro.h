#include <cjson/cJSON.h>

#include "degiro_user.h"
#include "degiro_utils.h"
#include "nob.h"

bool dg__user_config_from_json_string(dg_user_config *config, const char *json_string) {
    nob_log(NOB_INFO, "Parsing user_config from JSON string...");

    cJSON *json = cJSON_Parse(json_string);
    if (json == NULL) {
        nob_log(NOB_ERROR, "Error parsing JSON");
        return false;
    }

    cJSON *data = cJSON_GetObjectItem(json, "data");
    if (!cJSON_IsObject(data)) {
        nob_log(NOB_ERROR, "No \"data\" field in JSON");
        return false;
    }

    parse_string(data, "betaLandingPath", &(config->beta_landing_path));
    parse_string(data, "cashSolutionsUrl", &(config->cash_solutions_url));
    parse_int(data, "clientId", &(config->client_id));
    parse_string(data, "companiesServiceUrl", &(config->companies_service_url));
    parse_string(data, "dictionaryUrl", &(config->dictionary_url));
    parse_string(data, "firstLoginWizardUrl", &(config->first_login_wizard_url));
    parse_string(data, "i18nUrl", &(config->i18n_url));
    parse_string(data, "landingPath", &(config->landing_path));
    parse_string(data, "loginUrl", &(config->login_url));
    parse_string(data, "mobileLandingPath", &(config->mobile_landing_path));
    parse_string(data, "paUrl", &(config->pa_url));
    parse_string(data, "paymentServiceUrl", &(config->payment_service_url));
    parse_string(data, "productSearchUrl", &(config->product_search_url));
    parse_string(data, "productTypesUrl", &(config->product_types_url));
    parse_string(data, "reportingUrl", &(config->reporting_url));
    parse_string(data, "sessionId", &(config->session_id));
    parse_string(data, "taskManagerUrl", &(config->task_manager_url));
    parse_string(data, "tradingUrl", &(config->trading_url));
    parse_string(data, "vwdGossipsUrl", &(config->vwd_gossips_url));
    parse_string(data, "vwdNewsUrl", &(config->vwd_news_url));
    parse_string(data, "vwdQuotecastServiceUrl", &(config->vwd_quotecast_service_url));

    cJSON_Delete(json);
    return true;
}

bool dg__user_data_from_json_string(dg_user_data *info, const char *json_string) {
    nob_log(NOB_INFO, "Parsing user_data from JSON string...");

    cJSON *json = cJSON_Parse(json_string);
    if (json == NULL) {
        nob_log(NOB_ERROR, "Error parsing JSON");
        return false;
    }

    cJSON *data = cJSON_GetObjectItem(json, "data");
    if (!cJSON_IsObject(data)) {
        nob_log(NOB_ERROR, "No \"data\" field in JSON");
        return false;
    }

    parse_bool(data, "canUpgrade", &(info->can_upgrade));
    parse_string(data, "cellphoneNumber", &(info->cellphone_number));
    parse_string(data, "clientRole", &(info->client_role));
    parse_string(data, "contractType", &(info->contract_type));
    parse_string(data, "culture", &(info->culture));
    parse_string(data, "displayName", &(info->display_name));
    parse_string(data, "effectiveClientRole", &(info->effective_client_role));
    parse_string(data, "email", &(info->email));
    parse_int(data, "id", &(info->id));
    parse_int(data, "intAccount", &(info->int_account));
    parse_bool(data, "isAllocationAvailable", &(info->is_allocation_available));
    parse_bool(data, "isAmClientActive", &(info->is_am_client_active));
    parse_bool(data, "isCollectivePortfolio", &(info->is_collective_portfolio));
    parse_bool(data, "isIskClient", &(info->is_isk_client));
    parse_bool(data, "isWithdrawalAvailable", &(info->is_withdrawal_available));
    parse_string(data, "language", &(info->language));
    parse_string(data, "locale", &(info->locale));
    parse_string(data, "memberCode", &(info->member_code));
    parse_string(data, "username", &(info->username));

    // Address struct
    cJSON *address = cJSON_GetObjectItem(data, "address");
    if (!cJSON_IsObject(data)) {
        nob_log(NOB_ERROR, "No \"address\" field in JSON");
        return false;
    }

    parse_string(address, "city", &(info->address.city));
    parse_string(address, "country", &(info->address.country));
    parse_string(address, "streetAddress", &(info->address.street_address));
    parse_string(address, "streetAddressNumber", &(info->address.street_address_number));
    parse_string(address, "zip", &(info->address.zip));

    // Address struct
    cJSON *bank_account = cJSON_GetObjectItem(data, "bankAccount");
    if (!cJSON_IsObject(data)) {
        nob_log(NOB_ERROR, "No \"bankAccount\" field in JSON");
        return false;
    }

    parse_int(bank_account, "bankAccountId", &(info->bank_account.bank_account_id));
    parse_string(bank_account, "bic", &(info->bank_account.bic));
    parse_string(bank_account, "iban", &(info->bank_account.iban));
    parse_string(bank_account, "status", &(info->bank_account.status));

    // First contact struct
    cJSON *first_contact = cJSON_GetObjectItem(data, "firstContact");
    if (!cJSON_IsObject(data)) {
        nob_log(NOB_ERROR, "No \"firstContact\" field in JSON");
        return false;
    }

    parse_string(first_contact, "countryOfBirth", &(info->first_contact.country_of_birth));
    parse_string(first_contact, "dateOfBirth", &(info->first_contact.date_of_birth));
    parse_string(first_contact, "displayName", &(info->first_contact.display_name));
    parse_string(first_contact, "firstName", &(info->first_contact.first_name));
    parse_string(first_contact, "gender", &(info->first_contact.gender));
    parse_string(first_contact, "lastName", &(info->first_contact.last_name));
    parse_string(first_contact, "nationality", &(info->first_contact.nationality));
    parse_string(first_contact, "placeOfBirth", &(info->first_contact.place_of_birth));

    cJSON_Delete(json);
    return true;
}
