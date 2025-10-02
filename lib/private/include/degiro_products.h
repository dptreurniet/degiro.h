#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <cjson/cJSON.h>
#include "products.h"

typedef struct dg_products
{
  dg_product *items;
  size_t count;
  size_t capacity;
} dg_products;

bool dg__products_from_json_string(dg_products *products, const char *json_string);
bool dg__product_from_json_obj(dg_product *product, cJSON *obj);

bool dg__product_id_in_library(dg_product_library *registry, int product_id);
void dg__add_product_to_library(dg_product_library *library, dg_product *product);
dg_product* dg__get_product_from_library_by_id(dg_product_library *lib, int id);