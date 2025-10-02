#pragma once

#include <stddef.h>

typedef struct dg_product
{
  int id;
  const char *name;
  const char *isin;
  const char *symbol;
  int contract_size;
  const char *product_type;
  int product_type_id;
  bool tradable;
  const char *category;
  const char *currency;
  bool active;
  const char *exchange_id;
  bool only_eod_prices;
  const char **order_time_types;
  const char **buy_order_types;
  const char **sell_order_types;
  double close_price;
  const char *close_price_date;
  bool is_shortable;
  const char *feed_quality;
  int order_book_depth;
  const char *vwd_identifier_type;
  const char *vwd_id;
  bool quality_switchable;
  bool quality_switch_free;
  int vwd_module_id;
} dg_product;

typedef struct dg_product_library {
  dg_product *items;
  size_t count;
  size_t capacity;
} dg_product_library;