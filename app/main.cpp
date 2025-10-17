#include <stdio.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GLFW/glfw3.h>
#include <math.h>

#include <iostream>
#include <sstream>

#include "implot.h"
extern "C" {
#include "degiro.h"
}
#include "secrets.h"

bool show_demo_window = true;
bool show_implot_demo_window = true;
bool show_another_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

dg_context dg = {0};

void render_login_popup() {
    if (!dg.logged_in && glfwGetTime() > 0.5) {
        ImGui::OpenPopup("Login");
    }

    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Login", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
#ifdef USERNAME
        static char username[64] = USERNAME;
#else
        static char username[64] = "";
#endif
        ImGui::InputText("username", username, IM_ARRAYSIZE(username));

#ifdef PASSWORD
        static char password[64] = PASSWORD;
#else
        static char password[64] = "";
#endif
        ImGui::InputTextWithHint("password", "******", password, IM_ARRAYSIZE(password), ImGuiInputTextFlags_Password);

        if (glfwGetTime() < 0.6) {
            ImGui::SetKeyboardFocusHere();
        }
        static char totp[7] = "";
        ImGui::InputTextWithHint("one-time password", "123456", totp, IM_ARRAYSIZE(totp));

        dg_login_data login = {
            .username = username,
            .password = password,
            .totp = totp};

        if (ImGui::Button("Login") || ImGui::IsKeyPressed(ImGuiKey_Enter)) {
            if (!dg_login(&dg, login)) {
                fprintf(stderr, "Failed to login\n");
            } else {
                memset(&username, '\0', sizeof(username));
                memset(&password, '\0', sizeof(password));
                memset(&totp, '\0', sizeof(totp));
            }
        }

        if (dg.logged_in) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void render_user_info() {
    if (!dg.user_data.id) {
        if (ImGui::Button("Get account info")) {
            if (!dg_get_user_data(&dg)) {
                fprintf(stderr, "Failed to get user data");
            }
        }
    } else {
        ImGui::Text("Username:          %s", dg.user_data.username);
        ImGui::Text("Display name:      %s", dg.user_data.display_name);
        ImGui::Text("Email:             %s", dg.user_data.email);
        ImGui::Text("Language:          %s", dg.user_data.language);
        ImGui::Text("Locale:            %s", dg.user_data.locale);
        ImGui::Text("Member code:       %s", dg.user_data.member_code);

        ImGui::Text("ID:                %d", dg.user_data.id);
        ImGui::Text("Int account:       %d", dg.user_data.int_account);

        ImGui::Text("Cellphone number:  %s", dg.user_data.cellphone_number);
        ImGui::Text("Client role:       %s", dg.user_data.client_role);
        ImGui::Text("Contract type:     %s", dg.user_data.contract_type);
        ImGui::Text("Culture:           %s", dg.user_data.culture);
        ImGui::Text("Eff. client role:  %s", dg.user_data.effective_client_role);

        if (ImGui::TreeNode("First contact")) {
            ImGui::Text("Display name:   %s", dg.user_data.first_contact.display_name);
            ImGui::Text("First name:     %s", dg.user_data.first_contact.first_name);
            ImGui::Text("Last name:      %s", dg.user_data.first_contact.last_name);
            ImGui::Text("Date of birth:  %s", dg.user_data.first_contact.date_of_birth);
            ImGui::Text("Place of birth: %s (%s)", dg.user_data.first_contact.place_of_birth, dg.user_data.first_contact.country_of_birth);
            ImGui::Text("Nationality:    %s", dg.user_data.first_contact.nationality);
            ImGui::Text("Gender:         %s", dg.user_data.first_contact.gender);
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Address")) {
            ImGui::Text("Street:        %s", dg.user_data.address.street_address);
            ImGui::Text("Street number: %s", dg.user_data.address.street_address_number);
            ImGui::Text("Zip:           %s", dg.user_data.address.zip);
            ImGui::Text("City:          %s", dg.user_data.address.city);
            ImGui::Text("Country:       %s", dg.user_data.address.country);
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Bank account")) {
            ImGui::Text("BIC:    %s", dg.user_data.bank_account.bic);
            ImGui::Text("IBAN:   %s", dg.user_data.bank_account.iban);
            ImGui::Text("Status: %s", dg.user_data.bank_account.status);
            ImGui::Text("ID:     %d", dg.user_data.bank_account.bank_account_id);

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Flags")) {
            ImGui::Text("Can upgrade:             %s", dg.user_data.can_upgrade ? "true" : "false");
            ImGui::Text("Is allocation available: %s", dg.user_data.is_allocation_available ? "true" : "false");
            ImGui::Text("Is am client active:     %s", dg.user_data.is_am_client_active ? "true" : "false");
            ImGui::Text("Is collective portfolio: %s", dg.user_data.is_collective_portfolio ? "true" : "false");
            ImGui::Text("Is isk client:           %s", dg.user_data.is_isk_client ? "true" : "false");
            ImGui::Text("Is withdrawal available: %s", dg.user_data.is_withdrawal_available ? "true" : "false");
            ImGui::TreePop();
        }
    }
}

void render_products() {
    static int selected_ix = -1;

    {  // -------- Products --------
        ImGui::BeginChild("Products", ImVec2(ImGui::GetContentRegionAvail().x * 0.6f, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_None, ImGuiWindowFlags_None);

        static dg_products search_results = {0};
        static char search_input[128];
        dg_search_products_options search_options = {
            .limit = 10,
            .include_crypto = true,
        };

        if (ImGui::InputText("Product search", search_input, sizeof(search_input), ImGuiInputTextFlags_EnterReturnsTrue)) {
            if (strcmp(search_input, "") != 0) {
                search_options.search_string = search_input;
                dg_search_products(&dg, search_options, &search_results);
                ImGui::OpenPopup("search_result_popup");
            }
        }

        ImVec2 rect_min = ImGui::GetItemRectMin();
        ImGui::SetNextWindowPos({rect_min.x, rect_min.y + ImGui::GetItemRectSize().y}, ImGuiCond_Appearing);
        if (ImGui::BeginPopup("search_result_popup")) {
            if (search_results.count == 0) ImGui::Text("No results...");

            static size_t selected_ix = -1;
            for (size_t i = 0; i < search_results.count; ++i) {
                dg_product p = search_results.items[i];
                dg_exchange *exchange = dg_lookup_exchange_by_id(&dg, atoi(p.exchange_id));
                std::ostringstream line_str;
                line_str << ((p.symbol == NULL) ? "????" : p.symbol) << " | " << p.isin << " | " << exchange->hiq_abbr << " | " << p.name;
                if (ImGui::Selectable(line_str.str().c_str(), i == selected_ix)) {
                    selected_ix = i;
                    if (!dg_get_product_info(&dg, p.id)) {
                        fprintf(stderr, "Failed to load product with id %d", p.id);
                    }
                }
            }
            ImGui::EndPopup();
        }

        ImGuiTableFlags table_flags = 0;
        table_flags |= ImGuiTableFlags_SizingFixedFit;
        table_flags |= ImGuiTableFlags_NoHostExtendX;
        table_flags |= ImGuiTableFlags_RowBg;
        table_flags |= ImGuiTableFlags_BordersOuter;
        table_flags |= ImGuiTableFlags_BordersV;
        table_flags |= ImGuiTableFlags_NoBordersInBody;
        table_flags |= ImGuiTableFlags_ScrollY;

        if (ImGui::BeginTable("products", 6, table_flags)) {
            ImGui::TableSetupColumn("Symbol");
            ImGui::TableSetupColumn("ISIN");
            ImGui::TableSetupColumn("Exchange");
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Close price");
            ImGui::TableSetupScrollFreeze(0, 1);  // Keep header always visible
            ImGui::TableHeadersRow();

            for (size_t i = 0; i < dg.products.count; ++i) {
                dg_product product = dg.products.items[i];
                ImGui::PushID(i);

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                if (ImGui::Selectable(product.symbol, i == selected_ix, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {
                    selected_ix = i;
                }

                ImGui::TableNextColumn();
                ImGui::Text("%s  ", product.isin);

                ImGui::TableNextColumn();
                ImGui::Text("%s  ", dg_lookup_exchange_by_id(&dg, atoi(product.exchange_id))->hiq_abbr);

                ImGui::TableNextColumn();
                ImGui::Text("%s  ", product.name);

                ImGui::TableNextColumn();
                ImGui::Text("%.2f", product.close_price);

                ImGui::PopID();
            }
            ImGui::EndTable();
        }

        ImGui::EndChild();
    }

    ImGui::SameLine();

    {  // -------- Properties --------
        ImGui::BeginChild("Properties", ImVec2(0, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_None, ImGuiWindowFlags_None);
        ImGui::SeparatorText("Properties");

        if (selected_ix == -1) {
            ImGui::Text("Select a product");
        } else {
            dg_product product = dg.products.items[selected_ix];

            ImGui::Text("ID:                  %d", product.id);
            ImGui::Text("Name:                %s", product.name);
            ImGui::Text("ISIN:                %s", product.isin);
            ImGui::Text("Symbol:              %s", product.symbol);
            ImGui::Text("Contract size:       %d", product.contract_size);
            ImGui::Text("Product type:        %s", product.product_type);
            ImGui::Text("Product type id:     %d", product.product_type_id);
            ImGui::Text("Tradable:            %s", product.tradable ? "true" : "false");
            ImGui::Text("Category:            %s", product.category);
            ImGui::Text("Currency:            %s", product.currency);
            ImGui::Text("Active:              %s", product.active ? "true" : "false");
            ImGui::Text("Exchange:            %s", dg_lookup_exchange_by_id(&dg, atoi(product.exchange_id))->name);
            ImGui::Text("Only EOD prices:     %s", product.only_eod_prices ? "true" : "false");
            ImGui::Text("Order time types:    %s", product.order_time_types);
            ImGui::Text("Buy order types:     %s", product.buy_order_types);
            ImGui::Text("Sell order types:    %s", product.sell_order_types);
            ImGui::Text("Close price:         %.2f", product.close_price);
            ImGui::Text("Close price date:    %s", product.close_price_date);
            ImGui::Text("Is shortable:        %s", product.is_shortable ? "true" : "false");
            ImGui::Text("Feed quality:        %s", product.feed_quality);
            ImGui::Text("Order book depth:    %d", product.order_book_depth);
            ImGui::Text("Vwd identifier type: %s", product.vwd_identifier_type);
            ImGui::Text("Vwd id:              %s", product.vwd_id);
            ImGui::Text("Quality switchable:  %s", product.quality_switchable ? "true" : "false");
            ImGui::Text("Quality switch free: %s", product.quality_switch_free ? "true" : "false");
            ImGui::Text("Vwd module id:       %d", product.vwd_module_id);

            // -------- Chart --------
            {
                ImGui::SeparatorText("Price");

                static dg_product_chart chart = {0};

                static dg_product_chart_options prev_options = {0};
                static dg_product_chart_options options = {.period = PERIOD_1W};
                options.product = product;

                ImGui::Text("Chart period:");
                ImGui::SameLine();
                if (ImGui::RadioButton("1D", options.period == PERIOD_1D)) {
                    options.period = PERIOD_1D;
                }
                ImGui::SameLine();
                if (ImGui::RadioButton("1W", options.period == PERIOD_1W)) {
                    options.period = PERIOD_1W;
                }
                ImGui::SameLine();
                if (ImGui::RadioButton("1M", options.period == PERIOD_1M)) {
                    options.period = PERIOD_1M;
                }
                ImGui::SameLine();
                if (ImGui::RadioButton("6M", options.period == PERIOD_6M)) {
                    options.period = PERIOD_6M;
                }
                ImGui::SameLine();
                if (ImGui::RadioButton("1Y", options.period == PERIOD_1Y)) {
                    options.period = PERIOD_1Y;
                }

                if (prev_options.period != options.period ||
                    prev_options.product.id != options.product.id) {
                    dg_get_product_info_chart(&dg, options, &chart);
                }
                prev_options = options;

                char plot_title[64];
                if (chart.currency) {
                    snprintf(plot_title, sizeof(plot_title), "Price [%s]", chart.currency);
                } else {
                    snprintf(plot_title, sizeof(plot_title), "Price");
                }

                if (ImPlot::BeginPlot(plot_title, ImGui::GetContentRegionAvail(), ImPlotFlags_NoLegend)) {
                    ImPlot::BustColorCache(plot_title);
                    if (chart.chart_data.n_points > 0) {
                        ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
                        ImPlot::GetStyle().Use24HourClock = true;
                        ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoHighlight, ImPlotAxisFlags_NoHighlight);

                        float padding = (chart.high_price - chart.low_price) * 0.1;
                        ImPlot::SetupAxesLimits(chart.chart_data.timestamps[0], chart.chart_data.timestamps[chart.chart_data.n_points - 1], chart.low_price - padding, chart.high_price + padding, ImPlotCond_Always);

                        // Plot lines
                        ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.25f);
                        ImPlot::PlotShaded(chart.product.name, chart.chart_data.timestamps, chart.chart_data.prices, chart.chart_data.n_points, -INFINITY);
                        ImPlot::PlotLine(chart.product.name, chart.chart_data.timestamps, chart.chart_data.prices, chart.chart_data.n_points);

                        // Plot tags
                        ImVec4 color;

                        color = ImVec4(0, 1, 0, 1);
                        ImPlot::TagY(chart.high_price, color);
                        ImPlot::PushStyleColor(ImPlotCol_Line, color);
                        ImPlot::PlotInfLines("", &chart.high_price, 1, ImPlotInfLinesFlags_Horizontal);
                        ImPlot::PopStyleColor();

                        color = ImVec4(1, 0, 0, 1);
                        ImPlot::TagY(chart.low_price, color);
                        ImPlot::PushStyleColor(ImPlotCol_Line, color);
                        ImPlot::PlotInfLines("", &chart.low_price, 1, ImPlotInfLinesFlags_Horizontal);
                        ImPlot::PopStyleColor();
                    }
                    ImPlot::EndPlot();
                }
            }
        }

        ImGui::EndChild();
    }
}

/*

void render_portfolio() {
    static bool show_non_zero_only = true;
    ImGui::Checkbox("Non-zero only", &show_non_zero_only);
    ImGui::SameLine();

    static bool show_cash = true;
    ImGui::Checkbox("Cash", &show_cash);

    if (dg.portfolio.count == 0) {
        ImGui::Text("Portfolio has not been loaded yet...");
    } else {
        for (auto i = 0; i < dg.portfolio.count; i++) {
            dg_position position = dg.portfolio.items[i];

            const dg_product *product = NULL;
            if (strcmp(position.position_type, "PRODUCT") == 0) {
                // product = dg_get_product_info_by_id(&dg, atoi(position.id));
            }

            if (show_non_zero_only && position.size <= 0)
                continue;
            if (!show_cash && strcmp(position.position_type, "CASH") == 0)
                continue;

            if (ImGui::TreeNode(product ? product->name : position.id)) {
                ImGui::Text("ID:                         %s", position.id);
                ImGui::Text("Position type:              %s", position.position_type);
                ImGui::Text("Size:                       %d", position.size);
                ImGui::Text("Price:                      %.2f", position.price);
                ImGui::Text("Value:                      %.2f", position.value);
                ImGui::Text("Pl base:                    %.2f", position.pl_base);
                ImGui::Text("Today pl base:              %.2f", position.today_pl_base);
                ImGui::Text("Portfolio value correction: %.2f", position.portfolio_value_correction);
                ImGui::Text("Break even price:           %.2f", position.break_even_price);
                ImGui::Text("Average fx rate:            %.2f", position.average_fx_rate);
                ImGui::Text("Realized product pl:        %.2f", position.realized_product_pl);
                ImGui::Text("Realized fx pl:             %.2f", position.realized_fx_pl);
                ImGui::Text("Today realized product pl:  %.2f", position.today_realized_product_pl);
                ImGui::Text("Today realized fx pl:       %.2f", position.today_realized_fx_pl);
                ImGui::TreePop();
            }
        }
    }
    // ImGui::End();
}

void render_product_chart(dg_product_chart chart) {
    char plot_title[64];
    if (chart.currency) {
        snprintf(plot_title, sizeof(plot_title), "Price [%s]", chart.currency);
    } else {
        snprintf(plot_title, sizeof(plot_title), "Price");
    }

    if (ImPlot::BeginPlot(plot_title, ImGui::GetContentRegionAvail(), ImPlotFlags_NoLegend)) {
        if (chart.chart.n_points > 0) {
            ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
            ImPlot::GetStyle().Use24HourClock = true;

            float padding = (chart.high_price - chart.low_price) * 0.1;
            ImPlot::SetupAxesLimits(chart.chart.timestamps[0], chart.chart.timestamps[chart.chart.n_points - 1], chart.low_price - padding, chart.high_price + padding, ImPlotCond_Always);

            // Plot lines
            ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.25f);
            ImPlot::PlotShaded(chart.product.name, chart.chart.timestamps, chart.chart.prices, chart.chart.n_points, -INFINITY);
            ImPlot::PlotLine(chart.product.name, chart.chart.timestamps, chart.chart.prices, chart.chart.n_points);

            // Plot tags
            ImVec4 color;

            color = ImVec4(0, 1, 0, 1);
            ImPlot::TagY(chart.high_price, color);
            ImPlot::PushStyleColor(ImPlotCol_Line, color);
            ImPlot::PlotInfLines("", &chart.high_price, 1, ImPlotInfLinesFlags_Horizontal);
            ImPlot::PopStyleColor();

            color = ImVec4(1, 0, 0, 1);
            ImPlot::TagY(chart.low_price, color);
            ImPlot::PushStyleColor(ImPlotCol_Line, color);
            ImPlot::PlotInfLines("", &chart.low_price, 1, ImPlotInfLinesFlags_Horizontal);
            ImPlot::PopStyleColor();
        }
        ImPlot::EndPlot();
    }
}

bool operator==(const dg_product_chart_options &lhs, const dg_product_chart_options &rhs) {
    if (lhs.product.id != rhs.product.id)
        return false;
    if (lhs.period != rhs.period)
        return false;
    return true;
}
bool operator!=(const dg_product_chart_options &lhs, const dg_product_chart_options &rhs) {
    if (lhs == rhs)
        return false;
    return true;
}

*/
void render_transactions() {
    static dg_transactions transactions = {0};
    static size_t selected_ix = -1;

    {  // -------- Transactions list --------
        ImGui::BeginChild("Transactions", ImVec2(ImGui::GetContentRegionAvail().x * 0.6f, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_None, ImGuiWindowFlags_None);

        if (ImGui::Button("Get all")) {
            dg_get_transactions_options opts = {
                .from_date = {.tm_mday = 1},
                .to_date = {.tm_mday = 1, .tm_year = 200},
                .group_by_order = true};

            if (!dg_get_transactions(&dg, opts, &transactions)) {
                fprintf(stderr, "Failed to get transactions\n");
            }

            int ids[transactions.count];
            for (size_t i = 0; i < transactions.count; ++i) {
                ids[i] = transactions.items[i].product_id;
            }
            dg_get_product_infos_info(&dg, ids, transactions.count);
        }

        if (transactions.items == 0) {
            ImGui::Text("No transactions loaded yet");
        } else {
            ImGuiTableFlags table_flags = 0;
            table_flags |= ImGuiTableFlags_SizingFixedFit;
            table_flags |= ImGuiTableFlags_NoHostExtendX;
            table_flags |= ImGuiTableFlags_RowBg;
            table_flags |= ImGuiTableFlags_BordersOuter;
            table_flags |= ImGuiTableFlags_BordersV;
            table_flags |= ImGuiTableFlags_NoBordersInBody;
            table_flags |= ImGuiTableFlags_ScrollY;

            if (ImGui::BeginTable("transactions", 7, table_flags)) {
                ImGui::TableSetupColumn("Date");
                ImGui::TableSetupColumn("Action");
                ImGui::TableSetupColumn("#");
                ImGui::TableSetupColumn("Product");
                ImGui::TableSetupColumn("Price");
                ImGui::TableSetupColumn("Total w/ fees");
                ImGui::TableSetupScrollFreeze(0, 1);  // Keep header always visible
                ImGui::TableHeadersRow();

                for (size_t i = 0; i < transactions.count; ++i) {
                    dg_transaction t = transactions.items[i];
                    ImGui::PushID(i);

                    dg_product product = {0};
                    for (size_t j = 0; j < dg.products.count; ++j) {
                        if (dg.products.items[j].id == t.product_id) {
                            product = dg.products.items[j];
                            break;
                        }
                    }
                    if (product.id == 0) {
                        fprintf(stderr, "Product with id %d not found", t.product_id);
                        return;
                    }

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    char *trimmed_date = (char *)malloc(sizeof(char) * 11);
                    strncpy(trimmed_date, t.date, 10);
                    trimmed_date[10] = '\0';
                    if (ImGui::Selectable(trimmed_date, i == selected_ix, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {
                        selected_ix = i;
                    }
                    free(trimmed_date);

                    ImGui::TableNextColumn();
                    const char *buysell = strcmp(t.buysell, "S") == 0 ? " Sold " : "Bought";
                    if (strcmp(t.buysell, "S") == 0) {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(.7, 0, 0, 1));
                        ImGui::Button("Sell", ImVec2(50, 0));
                    } else {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, .7, 0, 1));
                        ImGui::Button("Buy", ImVec2(50, 0));
                    }
                    ImGui::PopStyleColor();

                    ImGui::TableNextColumn();
                    ImGui::Text("%d", abs(t.quantity));

                    ImGui::TableNextColumn();
                    ImGui::Text("%s  ", product.name);

                    ImGui::TableNextColumn();
                    ImGui::Text("%.2f  ", t.price);

                    ImGui::TableNextColumn();
                    ImGui::Text("%.2f  ", t.total_plus_all_fees_in_base_currency);

                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
        }
        ImGui::EndChild();
    }

    ImGui::SameLine();

    {  // -------- Properties --------
        ImGui::BeginChild("Properties", ImVec2(0, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_None, ImGuiWindowFlags_None);
        ImGui::SeparatorText("Properties");

        if (selected_ix == -1) {
            ImGui::Text("Select a transaction to view its properties");
        } else {
            dg_transaction t = transactions.items[selected_ix];
            ImGui::Text("ID:                                   %d", t.id);                                      // int id;
            ImGui::Text("Product ID:                           %d", t.product_id);                              // int product_id;
            ImGui::Text("Date:                                 %s", t.date);                                    // char *date;
            ImGui::Text("Buysell:                              %s", t.buysell);                                 // char *buysell;
            ImGui::Text("Price:                                %.2f", t.price);                                 // double price;
            ImGui::Text("Quantity:                             %d", t.quantity);                                // int quantity;
            ImGui::Text("Total:                                %.2f", t.total);                                 // double total;
            ImGui::Text("Order type ID:                        %d", t.order_type_id);                           // int order_type_id;
            ImGui::Text("Counter party:                        %s", t.counter_party);                           // char *counter_party;
            ImGui::Text("Transfered:                           %s", t.transfered ? "yes" : "no");               // bool transfered;
            ImGui::Text("Fx rate:                              %d", t.fx_rate);                                 // int fx_rate;
            ImGui::Text("Nett fx rate:                         %d", t.nett_fx_rate);                            // int nett_fx_rate;
            ImGui::Text("Gross fx rate:                        %d", t.gross_fx_rate);                           // int gross_fx_rate;
            ImGui::Text("Auto fx fee in base currency:         %d", t.auto_fx_fee_in_base_currency);            // int auto_fx_fee_in_base_currency;
            ImGui::Text("Total in base currency:               %.2f", t.total_in_base_currency);                // double total_in_base_currency;
            ImGui::Text("Fee in base currency:                 %.2f", t.fee_in_base_currency);                  // double fee_in_base_currency;
            ImGui::Text("Total fees in base currency:          %.2f", t.total_fees_in_base_currency);           // double total_fees_in_base_currency;
            ImGui::Text("Total plus fee in base currency:      %.2f", t.total_plus_fee_in_base_currency);       // double total_plus_fee_in_base_currency;
            ImGui::Text("Total plus all fees in base currency: %.2f", t.total_plus_all_fees_in_base_currency);  // double total_plus_all_fees_in_base_currency;
            ImGui::Text("Transaction type ID:                  %d", t.transaction_type_id);                     // int transaction_type_id;
            ImGui::Text("Trading venue:                        %s", t.trading_venue);                           // char *trading_venue;
            ImGui::Text("Executing entity ID:                  %s", t.executing_entity_id);                     // char *executing_entity_id;
        }
        ImGui::EndChild();
    }
}

// dg_get_transactions_options options = {
//     .from_date = "1900-01-01",
//     .to_date = "2100-01-01",
//     .group_transactions_by_order = true};

// static dg_transactions transactions = {0};

// if (ImGui::Button("Get transactions"))
// {
//     dg_get_transactions(&dg, options, &transactions);
// }

// for (size_t i = 0; i < transactions.count; ++i)
// {
//     dg_transaction t = transactions.items[i];
//     if (ImGui::TreeNode("##%s", t.date))
//     {
//         ImGui::Text("id:                                   %d", t.id);
//         ImGui::Text("product_id:                           %d", t.product_id);
//         ImGui::Text("date:                                 %s", t.date);
//         ImGui::Text("buysell:                              %s", t.buysell);
//         ImGui::Text("price:                                %.2f", t.price);
//         ImGui::Text("quantity:                             %d", t.quantity);
//         ImGui::Text("total:                                %.2f", t.total);
//         ImGui::Text("order_type_id:                        %d", t.order_type_id);
//         ImGui::Text("counter_party:                        %s", t.counter_party);
//         ImGui::Text("transfered:                           %s", t.transfered ? "true" : "false");
//         ImGui::Text("fx_rate:                              %d", t.fx_rate);
//         ImGui::Text("nett_fx_rate:                         %d", t.nett_fx_rate);
//         ImGui::Text("gross_fx_rate:                        %d", t.gross_fx_rate);
//         ImGui::Text("auto_fx_fee_in_base_currency:         %d", t.auto_fx_fee_in_base_currency);
//         ImGui::Text("total_in_base_currency:               %.2f", t.total_in_base_currency);
//         ImGui::Text("fee_in_base_currency:                 %.2f", t.fee_in_base_currency);
//         ImGui::Text("total_fees_in_base_currency:          %.2f", t.total_fees_in_base_currency);
//         ImGui::Text("total_plus_fee_in_base_currency:      %.2f", t.total_plus_fee_in_base_currency);
//         ImGui::Text("total_plus_all_fees_in_base_currency: %.2f", t.total_plus_all_fees_in_base_currency);
//         ImGui::Text("transaction_type_id:                  %d", t.transaction_type_id);
//         ImGui::Text("trading_venue:                        %s", t.trading_venue);
//         ImGui::Text("executing_entity_id:                  %s", t.executing_entity_id);
//         ImGui::TreePop();
//     }
// }

void render_app() {
    render_login_popup();

    ImGuiIO io = ImGui::GetIO();
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);

    if (ImGui::Begin("Docked Window", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
        if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags)) {
            if (ImGui::BeginTabItem("User info")) {
                render_user_info();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Portfolio")) {
                // render_portfolio();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Transactions")) {
                render_transactions();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Products")) {
                render_products();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();

    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    if (show_implot_demo_window)
        ImPlot::ShowDemoWindow(&show_implot_demo_window);
}

static void glfw_error_callback(int error, const char *description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main(int, char **) {
    if (!dg_init(&dg)) {
        fprintf(stderr, "Failed to initialize DeGiro\n");
        return 1;
    }

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Create window with graphics context
    GLFWwindow *window = glfwCreateWindow(1440, 900, "DeGiro.h test application", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImPlot::CreateContext();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details. If you like the default font but want it to scale better, consider using the 'ProggyVector' from the same author!
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // style.FontSizeBase = 20.0f;
    // io.Fonts->AddFontDefault();
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
    // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
    // IM_ASSERT(font != nullptr);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        render_app();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        // If you are using this code with non-legacy OpenGL header/contexts (which you should not, prefer using imgui_impl_opengl3.cpp!!),
        // you may need to backup/reset/restore other state, e.g. for current shader using the commented lines below.
        // GLint last_program;
        // glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
        // glUseProgram(0);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        // glUseProgram(last_program);

        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}