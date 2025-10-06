#include "degiro_curl.h"
#include <cjson/cJSON.h>
#include "degiro_utils.h"
#include <stdlib.h>
#include <string.h>
#include "nob.h"


void dg__dump_to_file(const char *str, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    fprintf(file, "%s", str);
    fclose(file);
}

size_t dg__curl_callback(void *buffer, size_t size, size_t nmemb, void *userp)
{
    size_t new_size = size * nmemb;
    nob_log(NOB_INFO, "Received %zu bytes of data", new_size);

    dg_curl_response *response = (dg_curl_response *)userp;
    char *ptr = realloc(response->data, response->size + new_size + 1);
    if (!ptr)
    {
        nob_log(NOB_ERROR, "Failed to allocate response data");
        return 0;
    }

    response->data = ptr;
    memcpy(response->data + response->size, buffer, new_size); // Append new data
    response->size += new_size;
    response->data[response->size] = '\0'; // null-terminate

    dg__dump_to_file(response->data, "reponse.json");

    return new_size;
};

CURLcode dg__make_request(dg_curl *curl)
{
    nob_log(NOB_INFO, "Sending HTTP request to %s", curl->url);
    curl->response.data = NULL;
    curl->response.size = 0;
    CURLcode res = curl_easy_perform(curl->curl);
    // nob_log(NOB_INFO, "Received data: %s", curl->response.data);
    return res;
}

void dg__set_curl_url(dg_curl *curl, const char *url)
{
    curl->url = url;
    curl_easy_setopt(curl->curl, CURLOPT_URL, url);
}

void dg__set_curl_payload(dg_curl *curl, const char *payload)
{
    curl_easy_setopt(curl->curl, CURLOPT_POSTFIELDS, payload);
}

void dg__set_curl_GET(dg_curl *curl)
{
    curl_easy_setopt(curl->curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl->curl, CURLOPT_POST, 0L);
}

void dg__set_curl_POST(dg_curl *curl)
{
    curl_easy_setopt(curl->curl, CURLOPT_HTTPGET, 0L);
    curl_easy_setopt(curl->curl, CURLOPT_POST, 1L);
}

void dg__set_default_curl_headers(dg_curl *curl, const char *session_id)
{
    curl->headers = NULL;
    curl->headers = curl_slist_append(curl->headers, "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) Gecko/20100101 Firefox/92.0");
    curl->headers = curl_slist_append(curl->headers, "Content-Type: application/json");
    curl->headers = curl_slist_append(curl->headers, "Accept-Encoding: identity");

    // If there is a session id defined, add it
    if (session_id)
    {
        curl->headers = curl_slist_append(curl->headers, format_string("Cookie: JSESSIONID=%s;", session_id));
    }

    curl_easy_setopt(curl->curl, CURLOPT_HTTPHEADER, curl->headers);
}

bool dg__login_response_from_json_string(dg_login_response *response, const char *json_string)
{
    cJSON *json = cJSON_Parse(json_string);
    if (json == NULL)
    {
        nob_log(NOB_ERROR, "Error parsing JSON");
        return false;
    }

    parse_bool(  json, "captchaRequired",   &(response->captcha_required));
    parse_bool(  json, "isPassCodeEnabled", &(response->is_pass_code_enabled));
    parse_string(json, "locale",            &(response->locale));
    parse_string(json, "redirectUrl",       &(response->redirect_url));
    parse_string(json, "sessionId",         &(response->session_id));
    parse_int(   json, "status",            &(response->status));
    parse_string(json, "statusText",        &(response->status_text));

    cJSON_Delete(json);
    return true;
}