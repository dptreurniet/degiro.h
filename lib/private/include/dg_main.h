#pragma once

#include <stdbool.h>

typedef struct dg_login_response {
    bool captcha_required;
    bool is_pass_code_enabled;
    char *locale;
    char *redirect_url;
    char *session_id;
    int status;
    char *status_text;
    // ignoring userTokens: []
} dg_login_response;
