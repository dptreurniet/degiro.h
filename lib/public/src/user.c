#include "user.h"

#include "defines.h"
#include "dg_main.h"
#include "dg_user.h"
#include "dg_utils.h"
#include "nob.h"

bool dg_get_user_data(dg_context *ctx) {
    nob_log(NOB_INFO, "Getting user info");
    if (!ctx->user_config.session_id) {
        nob_log(NOB_ERROR, "No session ID defined");
        return false;
    }

    dg__set_default_curl_headers(ctx);
    dg__set_curl_payload(ctx, "");
    dg__set_curl_GET(ctx);
    dg__set_curl_url(ctx, format_string("%sclient?sessionId=%s", ctx->user_config.pa_url, ctx->user_config.session_id));

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
