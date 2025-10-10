#pragma once

#include <curl/curl.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct dg_curl_response {
    char *data;
    size_t size;
} dg_curl_response;

typedef struct dg_curl {
    CURL *curl;
    dg_curl_response response;
    struct curl_slist *headers;
    char *url;
} dg_curl;
