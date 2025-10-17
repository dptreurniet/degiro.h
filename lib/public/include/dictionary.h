#pragma once

#include <stdbool.h>
#include <stddef.h>

// Forward-declare context
struct dg_context;
typedef struct dg_context dg_context;

typedef struct dg_exchange {
    int id;
    char *name;
    char *code;
    char *hiq_abbr;
    char *country;
    char *city;
    char *mic_code;
} dg_exchange;

typedef struct dg_exchanges {
    dg_exchange *items;
    size_t count;
} dg_exchanges;

typedef struct dg_dictionary {
    dg_exchanges exchanges;
} dg_dictionary;

bool dg_get_dictionary(dg_context *ctx);
