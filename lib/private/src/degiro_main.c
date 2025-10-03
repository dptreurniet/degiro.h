#include "degiro.h"
#include "degiro_main.h"
#include "defines.h"
#include "utils.h"
#include <ctype.h>

#ifndef NOB_IMPLEMENTATION
#define NOB_IMPLEMENTATION
#endif
#include "nob.h"

#include "secrets.h" //temp
#include "degiro_price.h"

dg_backend dgb = {0};

bool dg__init()
{
    nob_log(NOB_INFO, "Initializing DeGiro...");
    curl_global_init(CURL_GLOBAL_DEFAULT);

    dgb.curl.curl = curl_easy_init();
    if (!dgb.curl.curl)
    {
        nob_log(NOB_ERROR, "Failed to init Curl");
        return false;
    }

    curl_easy_setopt(dgb.curl.curl, CURLOPT_WRITEFUNCTION, dg__curl_callback);
    curl_easy_setopt(dgb.curl.curl, CURLOPT_WRITEDATA, &dgb.curl.response);
    dg__set_default_curl_headers(&dgb.curl, dgb.account_config.session_id);
    // curl_easy_setopt(dgb.curl.curl, CURLOPT_VERBOSE, 1L);

    nob_log(NOB_INFO, "DeGiro initialized");
    return true;
}

bool dg__login(degiro *dg, const char *username, const char *password, const char *totp)
{
    nob_log(NOB_INFO, "Logging in on DeGiro...");

    dg->logged_in = false;
    CURLcode res;
    dg_login_response response;

    // -------- Part 1: username / password --------

    dg__set_curl_POST(&dgb.curl);
    dg__set_default_curl_headers(&dgb.curl, dgb.account_config.session_id);
    // dg__set_curl_url(&dgb.curl, format_string("%s%s", DEGIRO_BASE_URL, DEGIRO_LOGIN_URL));

    // const char *payload = format_string("{\"username\":\"%s\",\"password\":\"%s\"}", username, password);
    // dg__set_curl_payload(&dgb.curl, payload);

    // res = dg__make_request(&dgb.curl);
    // if (res != CURLE_OK)
    // {
    //     nob_log(NOB_ERROR, "curl_easy_perform() failed: %s", curl_easy_strerror(res));
    //     return false;
    // }

    // dg__login_response_from_json_string(&response, dgb.curl.response.data);
    // if (response.status != 6)
    // {
    //     nob_log(NOB_ERROR, "Unexpected reponse status (%d), aborting login", response.status);
    //     return false;
    // }

    // -------- Part 2: time-based one-time password --------
    dg__set_curl_url(&dgb.curl, format_string("%s%s/totp", DEGIRO_BASE_URL, DEGIRO_LOGIN_URL));

    const char* payload = format_string("{\"username\":\"%s\",\"password\":\"%s\",\"oneTimePassword\":\"%s\"}", username, password, totp);
    curl_easy_setopt(dgb.curl.curl, CURLOPT_POSTFIELDS, payload);

    res = dg__make_request(&dgb.curl);
    if (res != CURLE_OK)
    {
        nob_log(NOB_ERROR, "curl_easy_perform() failed: %s", curl_easy_strerror(res));
        return false;
    }

    dg__login_response_from_json_string(&response, dgb.curl.response.data);
    if (response.status != 0)
    {
        nob_log(NOB_ERROR, "Login failed: %s", response.status_text);
        return false;
    }

    dgb.account_config.session_id = response.session_id; // Store session id to use in subsequent HTTP requests
    nob_log(NOB_INFO, "Got session id: \"%s\"", dgb.account_config.session_id);

    if (!dg__get_account_config(&dgb))
    {
        nob_log(NOB_ERROR, "Failed to get account config");
        return false;
    }

    if (!dg__get_account_data(dg))
    {
        nob_log(NOB_ERROR, "Failed to get account data");
        return false;
    }

    dg->logged_in = true;
    nob_log(NOB_INFO, "Login successful");
    return true;
}

bool dg__get_account_config()
{
    nob_log(NOB_INFO, "Getting account config...");

    CURLcode res;

    if (!dgb.account_config.session_id)
    {
        nob_log(NOB_ERROR, "No session id defined");
        return false;
    }

    dg__set_default_curl_headers(&dgb.curl, dgb.account_config.session_id);
    dg__set_curl_url(&dgb.curl, format_string("%s%s", DEGIRO_BASE_URL, DEGIRO_GET_ACCOUNT_CONFIG_URL));
    dg__set_curl_payload(&dgb.curl, "");
    dg__set_curl_GET(&dgb.curl);

    res = dg__make_request(&dgb.curl);
    if (res != CURLE_OK)
    {
        nob_log(NOB_ERROR, "curl_easy_perform() failed: %s", curl_easy_strerror(res));
        return false;
    }

    if (!dg__account_config_from_json_string(&dgb.account_config, dgb.curl.response.data))
    {
        nob_log(NOB_ERROR, "Failed to get account config");
        return false;
    }

    return true;
}

bool dg__get_account_data(degiro *dg)
{
    CURLcode res;

    nob_log(NOB_INFO, "Getting account info...");
    if (!dgb.account_config.session_id)
    {
        nob_log(NOB_ERROR, "No session id defined");
        return false;
    }

    dg__set_default_curl_headers(&dgb.curl, dgb.account_config.session_id);
    dg__set_curl_url(&dgb.curl, format_string("%sclient?sessionId=%s",
                                              dgb.account_config.pa_url,
                                              dgb.account_config.session_id));
    dg__set_curl_payload(&dgb.curl, "");
    dg__set_curl_GET(&dgb.curl);

    res = dg__make_request(&dgb.curl);
    if (res != CURLE_OK)
    {
        nob_log(NOB_ERROR, "curl_easy_perform() failed: %s", curl_easy_strerror(res));
        return false;
    }

    if (!dg__account_data_from_json_string(&dg->account_data, dgb.curl.response.data))
    {
        nob_log(NOB_ERROR, "Failed to get account info");
        return false;
    }

    return true;
}

bool dg__get_portfolio(degiro *dg)
{
    CURLcode res;

    nob_log(NOB_INFO, "Getting portfolio");
    if (!dgb.account_config.session_id)
    {
        nob_log(NOB_ERROR, "No session id defined");
        return false;
    }

    dg__set_default_curl_headers(&dgb.curl, dgb.account_config.session_id);
    const char *url = format_string("%sv5/update/%d;jsessionid=%s?&portfolio=0",
                                    dgb.account_config.trading_url,
                                    dg->account_data.int_account,
                                    dgb.account_config.session_id);
    dg__set_curl_url(&dgb.curl, url);
    dg__set_curl_payload(&dgb.curl, "");
    dg__set_curl_GET(&dgb.curl);

    res = dg__make_request(&dgb.curl);
    if (res != CURLE_OK)
    {
        nob_log(NOB_ERROR, "curl_easy_perform() failed: %s", curl_easy_strerror(res));
        return false;
    }

    dg__portfolio_from_json_string(&dg->portfolio, dgb.curl.response.data);

    // Generate array of number-only ids (to ignore cash positions)
    size_t n_ids = 0;
    for (size_t i = 0; i < dg->portfolio.count; i++)
    {
        if (is_only_numbers(dg->portfolio.items[i].id))
        {
            n_ids++;
        }
    }

    int *ids = (int *)malloc(sizeof(int) * dg->portfolio.count);
    for (size_t i = 0; i < dg->portfolio.count; i++)
    {
        if (is_only_numbers(dg->portfolio.items[i].id))
        {
            ids[i] = atoi(dg->portfolio.items[i].id);
        }
    }

    dg_products products = {0};
    dg__get_products_info(dg, ids, n_ids, &products);

    free(ids);
    return true;
}

bool dg__get_transactions(degiro *dg, dg_get_transactions_options options, dg_da_transactions *transactions)
{
    nob_log(NOB_INFO, "Getting transactions...");

    CURLcode res;

    const char *group_transactions_by_order_str;
    if (options.group_transactions_by_order)
    {
        group_transactions_by_order_str = "true";
    }
    else
    {
        group_transactions_by_order_str = "false";
    }

    dg__set_default_curl_headers(&dgb.curl, dgb.account_config.session_id);
    const char *url = format_string("%s%s?fromDate=%s&toDate=%s&groupTransactionsByOrder=%s&intAccount=%d&sessionId=%s",
                                    dgb.account_config.reporting_url,
                                    DEGIRO_GET_TRANSACTIONS_URL,
                                    options.from_date,
                                    options.to_date,
                                    group_transactions_by_order_str,
                                    dg->account_data.int_account,
                                    dgb.account_config.session_id);
    dg__set_curl_url(&dgb.curl, url);
    dg__set_curl_payload(&dgb.curl, "");
    dg__set_curl_GET(&dgb.curl);

    res = dg__make_request(&dgb.curl);
    if (res != CURLE_OK)
    {
        nob_log(NOB_ERROR, "curl_easy_perform() failed: %s", curl_easy_strerror(res));
        return false;
    }

    dg__transactions_from_json_str(transactions, dgb.curl.response.data);

    // Get product info of the products in transactions
    int *ids = (int *)malloc(sizeof(int) * transactions->count);
    for (size_t i = 0; i < transactions->count; i++)
    {
        ids[i] = transactions->items[i].product_id;
    }
    dg_products products = {0};
    dg__get_products_info(dg, ids, transactions->count, &products);

    return true;
}

bool dg__get_products_info(degiro *dg, int *ids, size_t n_ids, dg_products *products)
{
    nob_log(NOB_INFO, "Getting product info for %zu products...", n_ids);

    // Make list of unique ids (required to make request valid)
    struct
    {
        int *items;
        size_t count;
        size_t capacity;
    } unique_ids = {0};

    nob_da_reserve(&unique_ids, n_ids);

    for (size_t i = 0; i < n_ids; i++)
    {
        bool already_in = false;
        for (size_t j = 0; j < unique_ids.count; j++)
        {
            if (ids[i] == unique_ids.items[j])
            {
                already_in = true;
            }
        }
        if (already_in)
            continue;
        nob_da_append(&unique_ids, ids[i]);
    }

    nob_log(NOB_INFO, "Reduced to %zu unique ids", unique_ids.count);
    if (unique_ids.count == 0)
    {
        nob_log(NOB_INFO, "No product info to load");
        return true;
    }

    CURLcode res;

    if (!dgb.account_config.session_id)
    {
        nob_log(NOB_ERROR, "No session id defined, unable to make request to DeGiro");
        return false;
    }

    dg__set_default_curl_headers(&dgb.curl, dgb.account_config.session_id);
    const char *url = format_string("%sv5/products/info?intAccount=%d&sessionId=%s",
                                    dgb.account_config.product_search_url,
                                    dg->account_data.int_account,
                                    dgb.account_config.session_id);
    dg__set_curl_url(&dgb.curl, url);

    cJSON *array = cJSON_CreateArray();
    for (size_t i = 0; i < unique_ids.count; i++)
    {
        cJSON_AddItemToArray(array, cJSON_CreateString(format_string("%d", unique_ids.items[i])));
    }
    const char *payload = cJSON_PrintUnformatted(array);
    dg__set_curl_payload(&dgb.curl, payload);
    dg__set_curl_POST(&dgb.curl);

    res = dg__make_request(&dgb.curl);
    if (res != CURLE_OK)
    {
        nob_log(NOB_ERROR, "curl_easy_perform() failed: %s", curl_easy_strerror(res));
        return false;
    }

    if (!dg__products_from_json_string(products, dgb.curl.response.data))
    {
        nob_log(NOB_ERROR, "Failed to parse product from response");
        return false;
    }

    // Add products to registry for later reference
    for (size_t i = 0; i < products->count; i++)
    {
        dg__add_product_to_library(&dg->products, &products->items[i]);
    }

    return true;
}


bool dg__get_price(dg_price_history *result, dg_price_plot_options opts)
{
    // https://charting.vwdservices.com/hchart/v1/deGiro/data.js?requestid=1&resolution=P1D&culture=nl-NL&period=P50Y&series=issueid%3A485013849&series=price%3Aissueid%3A485013849&format=json&callback=vwd.hchart.seriesRequestManager.sync_response&userToken=4626342&tz=Europe%2FAmsterdam

    result->product = opts.product;

    const char *resolution = "1D";
    if (strcmp(opts.period, "1Y") == 0) {
        resolution = "1D";
    } else if(strcmp(opts.period, "1M") == 0) {
        resolution = "T2H";
    } else if(strcmp(opts.period, "1W") == 0) {
        resolution = "T30M";
    } else if(strcmp(opts.period, "1D") == 0) {
        resolution = "T1M";
    }
    
    nob_log(NOB_INFO, "Getting price info with the following options:");
    nob_log(NOB_INFO, " - Product: %s", opts.product->name);
    nob_log(NOB_INFO, " - Period: %s", opts.period);
    nob_log(NOB_INFO, " - Resolution: %s", resolution);

    Nob_String_Builder url = {0};
    nob_sb_appendf(&url, "%s?", DEGIRO_GET_CHART_URL);
    nob_sb_appendf(&url, "series=%s:%s&", opts.product->vwd_identifier_type, opts.product->vwd_id);
    nob_sb_appendf(&url, "series=price:%s:%s&", opts.product->vwd_identifier_type, opts.product->vwd_id);
    nob_sb_appendf(&url, "period=P%s&", opts.period);
    nob_sb_appendf(&url, "resolution=P%s&", resolution);
    nob_sb_appendf(&url, "userToken=%d&", dgb.account_config.client_id);
    nob_sb_appendf(&url, "culture=nl-NL&");
    nob_sb_appendf(&url, "format=json&");
    nob_sb_appendf(&url, "tz=Europe/Amsterdam");
    nob_sb_append_null(&url);

    dg__set_curl_url(&dgb.curl, url.items);

    dg__set_curl_GET(&dgb.curl);
    // curl_easy_setopt(dgb.curl.curl, CURLOPT_HTTPHEADER, NULL);

    dg__make_request(&dgb.curl);

    cJSON *json = cJSON_Parse(dgb.curl.response.data);
    if (json == NULL)
    {
        nob_log(NOB_ERROR, "Error parsing JSON");
        return false;
    }

    return dg__parse_price_response(json, result);
}

void dg__cleanup()
{
    nob_log(NOB_INFO, "Cleaning up degiro");
    curl_easy_cleanup(dgb.curl.curl);
    curl_global_cleanup();
}
