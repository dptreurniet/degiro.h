#pragma once

#include <curl/curl.h>
#include <stdbool.h>

typedef struct dg_curl_response
{
  char *data;
  size_t size;
} dg_curl_response;

typedef struct dg_curl
{
  CURL *curl;
  dg_curl_response response;
  struct curl_slist *headers;
  const char *url;
} dg_curl;

typedef struct dg_login_response
{
  bool captcha_required;
  bool is_pass_code_enabled;
  char *locale;
  char *redirect_url;
  char *session_id;
  int status;
  char *status_text;
  // ignoring userTokens: []
} dg_login_response;

size_t dg__curl_callback(void *buffer, size_t size, size_t nmemb, void *userp);
CURLcode dg__make_request(dg_curl *curl);

void dg__set_curl_url(dg_curl *curl, const char *url);
void dg__set_curl_payload(dg_curl *curl, const char *payload);
void dg__set_curl_GET(dg_curl *curl);
void dg__set_curl_POST(dg_curl *curl);

void dg__set_default_curl_headers(dg_curl *curl, const char* session_id);
bool dg__login_response_from_json_string(dg_login_response *response, const char *json_string);
