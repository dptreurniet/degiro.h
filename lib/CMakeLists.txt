cmake_minimum_required(VERSION 3.10)
project(degiro C)

find_package(CURL REQUIRED)
find_package(cJSON CONFIG REQUIRED)

set(SOURCES
    public/src/degiro.c
    public/src/utils.c

    private/src/degiro_account.c
    private/src/degiro_curl.c
    private/src/degiro_main.c
    private/src/degiro_portfolio.c
    private/src/degiro_price.c
    private/src/degiro_products.c
    private/src/degiro_transactions.c
    private/src/degiro_user.c
    private/src/degiro_utils.c
)

set(HEADERS
    public/include/degiro.h
    public/include/utils.h
)


add_library(degiro STATIC ${SOURCES} ${HEADERS})

target_include_directories(degiro PUBLIC public/include)
target_include_directories(degiro PRIVATE private/include)

target_link_libraries(degiro PRIVATE CURL::libcurl cjson)

target_compile_options(degiro PRIVATE -Wno-incompatible-pointer-types -Wall -Wextra)
