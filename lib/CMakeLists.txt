cmake_minimum_required(VERSION 3.10)
project(degiro C)

find_package(CURL REQUIRED)
find_package(cJSON CONFIG REQUIRED)

set(SOURCES
    public/src/degiro.c
    public/src/user.c
    public/src/transactions.c
    public/src/products.c
    public/src/chart.c
    public/src/dictionary.c
    public/src/helpers.c
    public/src/portfolio.c
    public/src/types.c

    private/src/dg_curl.c
    private/src/dg_utils.c
    private/src/dg_types.c
)

set(HEADERS
    public/include/degiro.h
    public/include/transactions.h
    public/include/products.h
    public/include/chart.h
    public/include/dictionary.h
    public/include/helpers.h
    public/include/portfolio.h
    public/include/types.h
)


add_library(degiro STATIC ${SOURCES} ${HEADERS})

target_include_directories(degiro PUBLIC public/include)
target_include_directories(degiro PRIVATE private/include)

target_link_libraries(degiro PRIVATE CURL::libcurl cjson)

target_compile_options(degiro PRIVATE -Wall -Wextra)
