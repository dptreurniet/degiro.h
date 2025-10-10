#include "dg_curl.h"

#include <cjson/cJSON.h>

#include "dg_utils.h"
#include "nob.h"

size_t dg__curl_callback(void *buffer, size_t size, size_t nmemb, void *userp) {
    size_t new_size = size * nmemb;

    dg_curl_response *response = (dg_curl_response *)userp;
    char *ptr = realloc(response->data, response->size + new_size + 1);
    if (!ptr) {
        nob_log(NOB_ERROR, "Failed to allocate memory for response data");
        return 0;
    }

    response->data = ptr;
    memcpy(response->data + response->size, buffer, new_size);  // Append new data
    response->size += new_size;
    response->data[response->size] = '\0';  // null-terminate

    dg__dump_to_file(response->data, "reponse.json");

    return new_size;
};

CURLcode dg__make_request(dg_context *ctx) {
    nob_log(NOB_INFO, "Sending HTTP request to %s", ctx->curl.url);
    ctx->curl.response.data = NULL;
    ctx->curl.response.size = 0;
    CURLcode res = curl_easy_perform(ctx->curl.curl);
    nob_log(NOB_INFO, "Received %ld bytes of data", ctx->curl.response.size);
    // nob_log(NOB_WARNING, "%s", ctx->curl.response.data);
    return res;
}

bool dg__set_curl_url(dg_context *ctx, const char *url) {
    free(ctx->curl.url);
    ctx->curl.url = malloc(strlen(url) + 1);
    if (!ctx->curl.url) {
        nob_log(NOB_ERROR, "Failed to allocate memory for the curl URL");
        return false;
    }
    strcpy(ctx->curl.url, url);
    curl_easy_setopt(ctx->curl.curl, CURLOPT_URL, url);
    return true;
}

void dg__set_curl_payload(dg_context *ctx, const char *payload) {
    curl_easy_setopt(ctx->curl.curl, CURLOPT_POSTFIELDS, payload);
}

void dg__set_curl_GET(dg_context *ctx) {
    curl_easy_setopt(ctx->curl.curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(ctx->curl.curl, CURLOPT_POST, 0L);
}

void dg__set_curl_POST(dg_context *ctx) {
    curl_easy_setopt(ctx->curl.curl, CURLOPT_HTTPGET, 0L);
    curl_easy_setopt(ctx->curl.curl, CURLOPT_POST, 1L);
}

void dg__set_default_curl_headers(dg_context *ctx) {
    ctx->curl.headers = NULL;
    ctx->curl.headers = curl_slist_append(ctx->curl.headers, "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) Gecko/20100101 Firefox/92.0");
    ctx->curl.headers = curl_slist_append(ctx->curl.headers, "Content-Type: application/json");
    ctx->curl.headers = curl_slist_append(ctx->curl.headers, "Accept-Encoding: identity");

    // If there is a session id defined, add it to the headers
    if (ctx->user_config.session_id)
        ctx->curl.headers = curl_slist_append(ctx->curl.headers, dg__format_string("Cookie: JSESSIONID=%s;", ctx->user_config.session_id));

    curl_easy_setopt(ctx->curl.curl, CURLOPT_HTTPHEADER, ctx->curl.headers);
}
