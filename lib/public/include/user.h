#pragma once

typedef struct dg_user_data {
    struct
    {
        char *city;
        char *country;
        char *street_address;
        char *street_address_number;
        char *zip;
    } address;
    struct
    {
        int bank_account_id;
        char *bic;
        char *iban;
        char *status;
    } bank_account;
    bool can_upgrade;
    char *cellphone_number;
    char *client_role;
    char *contract_type;
    char *culture;
    char *display_name;
    char *effective_client_role;
    char *email;
    struct
    {
        char *country_of_birth;
        char *date_of_birth;
        char *display_name;
        char *first_name;
        char *gender;
        char *last_name;
        char *nationality;
        char *place_of_birth;
    } first_contact;
    int id;
    int int_account;
    bool is_allocation_available;
    bool is_am_client_active;
    bool is_collective_portfolio;
    bool is_isk_client;
    bool is_withdrawal_available;
    char *language;
    char *locale;
    char *member_code;
    char *username;
} dg_user_data;