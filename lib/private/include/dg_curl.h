#pragma once

#include <curl/curl.h>
#include <stdarg.h>
#include <stdbool.h>

#include "degiro.h"

size_t dg__curl_callback(void *buffer, size_t size, size_t nmemb, void *userp);
CURLcode dg__make_request(dg_context *ctx);

bool dg__set_curl_url(dg_context *ctx, const char *url);
void dg__set_curl_payload(dg_context *ctx, const char *payload);
void dg__set_curl_GET(dg_context *ctx);
void dg__set_curl_POST(dg_context *ctx);
bool dg__set_default_curl_headers(dg_context *ctx);