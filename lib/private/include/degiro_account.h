#pragma once

#include <stdbool.h>

#include "account.h"

typedef struct dg_acount_config
{
  char *beta_landing_path;
  char *cash_solutions_url;
  int client_id;
  char *companies_service_url;
  char *dictionary_url;
  char *first_login_wizard_url;
  char *i18n_url;
  char *landing_path;
  char *login_url;
  char *mobile_landing_path;
  char *pa_url;
  char *payment_service_url;
  char *product_search_url;
  char *product_types_url;
  char *reporting_url;
  char *session_id;
  char *task_manager_url;
  char *trading_url;
  char *vwd_gossips_url;
  char *vwd_news_url;
  char *vwd_quotecast_service_url;
} dg_account_config;

bool dg__account_config_from_json_string(dg_account_config *config, const char *json_string);
bool dg__account_data_from_json_string(dg_account_data *info, const char *json_string);

