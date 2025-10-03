#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"
#include <stdio.h>
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GLFW/glfw3.h>
#include "implot.h"
#include <math.h>
#include <iostream>
extern "C"
{
#include "degiro.h"
}
#include "secrets.h"

bool show_demo_window = true;
bool show_implot_demo_window = true;
bool show_another_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

degiro dg = {0};

void render_login_popup()
{
    if (!dg.logged_in && glfwGetTime() > 0.5)
    {
        ImGui::OpenPopup("Login");
    }

    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Login", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
#ifdef USERNAME
        static char username[64] = USERNAME;
#else
        static char username[64] = "";
#endif
        ImGui::InputText("usename", username, IM_ARRAYSIZE(username));

#ifdef PASSWORD
        static char password[64] = PASSWORD;
#else
        static char password[64] = "";
#endif
        ImGui::InputTextWithHint("password", "******", password, IM_ARRAYSIZE(password), ImGuiInputTextFlags_Password);

        static char totp[7] = "";
        ImGui::InputTextWithHint("one-time password", "123456", totp, IM_ARRAYSIZE(totp));

        if (ImGui::Button("Login"))
        {
            if (!dg_login(&dg, username, password, totp))
            {
                fprintf(stderr, "Failed to login\n");
            }
            else
            {
                memset(&username, '\0', sizeof(username));
                memset(&password, '\0', sizeof(password));
                memset(&totp, '\0', sizeof(totp));

                if (!dg_get_portfolio(&dg))
                {
                    fprintf(stderr, "Failed to get portfolio\n");
                }
            }
        }

        if (dg.logged_in)
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void render_account_info()
{
    // ImGui::Begin("Account info");

    ImGui::Text("Username:          %s", dg.account_data.username);
    ImGui::Text("Display name:      %s", dg.account_data.display_name);
    ImGui::Text("Email:             %s", dg.account_data.email);
    ImGui::Text("Language:          %s", dg.account_data.language);
    ImGui::Text("Locale:            %s", dg.account_data.locale);
    ImGui::Text("Member code:       %s", dg.account_data.member_code);

    ImGui::Text("ID:                %d", dg.account_data.id);
    ImGui::Text("Int account:       %d", dg.account_data.int_account);

    ImGui::Text("Cellphone number:  %s", dg.account_data.cellphone_number);
    ImGui::Text("Client role:       %s", dg.account_data.client_role);
    ImGui::Text("Contract type:     %s", dg.account_data.contract_type);
    ImGui::Text("Culture:           %s", dg.account_data.culture);
    ImGui::Text("Eff. client role:  %s", dg.account_data.effective_client_role);

    if (ImGui::TreeNode("First contact"))
    {
        ImGui::Text("Display name:   %s", dg.account_data.first_contact.display_name);
        ImGui::Text("First name:     %s", dg.account_data.first_contact.first_name);
        ImGui::Text("Last name:      %s", dg.account_data.first_contact.last_name);
        ImGui::Text("Date of birth:  %s", dg.account_data.first_contact.date_of_birth);
        ImGui::Text("Place of birth: %s (%s)", dg.account_data.first_contact.place_of_birth, dg.account_data.first_contact.country_of_birth);
        ImGui::Text("Nationality:    %s", dg.account_data.first_contact.nationality);
        ImGui::Text("Gender:         %s", dg.account_data.first_contact.gender);
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Address"))
    {
        ImGui::Text("Street:        %s", dg.account_data.address.street_address);
        ImGui::Text("Street number: %s", dg.account_data.address.street_address_number);
        ImGui::Text("Zip:           %s", dg.account_data.address.zip);
        ImGui::Text("City:          %s", dg.account_data.address.city);
        ImGui::Text("Country:       %s", dg.account_data.address.country);
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Bank account"))
    {
        ImGui::Text("BIC:    %s", dg.account_data.bank_account.bic);
        ImGui::Text("IBAN:   %s", dg.account_data.bank_account.iban);
        ImGui::Text("Status: %s", dg.account_data.bank_account.status);
        ImGui::Text("ID:     %d", dg.account_data.bank_account.bank_account_id);

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Flags"))
    {
        ImGui::Text("Can upgrade:             %s", dg.account_data.can_upgrade ? "true" : "false");
        ImGui::Text("Is allocation available: %s", dg.account_data.is_allocation_available ? "true" : "false");
        ImGui::Text("Is am client active:     %s", dg.account_data.is_am_client_active ? "true" : "false");
        ImGui::Text("Is collective portfolio: %s", dg.account_data.is_collective_portfolio ? "true" : "false");
        ImGui::Text("Is isk client:           %s", dg.account_data.is_isk_client ? "true" : "false");
        ImGui::Text("Is withdrawal available: %s", dg.account_data.is_withdrawal_available ? "true" : "false");
        ImGui::TreePop();
    }

    // ImGui::End();
}

void render_portfolio()
{
    // ImGui::Begin("Portfolio");

    static bool show_non_zero_only = true;
    ImGui::Checkbox("Non-zero only", &show_non_zero_only);
    ImGui::SameLine();

    static bool show_cash = true;
    ImGui::Checkbox("Cash", &show_cash);

    if (dg.portfolio.count == 0)
    {
        ImGui::Text("Portfolio has not been loaded yet...");
    }
    else
    {
        for (auto i = 0; i < dg.portfolio.count; i++)
        {
            dg_position position = dg.portfolio.items[i];

            const dg_product *product = NULL;
            if (strcmp(position.position_type, "PRODUCT") == 0)
            {
                // product = dg_get_product_by_id(&dg, atoi(position.id));
            }

            if (show_non_zero_only && position.size <= 0)
                continue;
            if (!show_cash && strcmp(position.position_type, "CASH") == 0)
                continue;

            if (ImGui::TreeNode(product ? product->name : position.id))
            {
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

void render_products()
{
    // ImGui::Begin("Products");

    if (dg.products.count == 0)
    {
        ImGui::Text("No product info loaded yet...");
    }

    for (auto i = 0; i < dg.products.count; i++)
    {
        auto product = dg.products.items[i];
        if (ImGui::TreeNode(product.name))
        {
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
            ImGui::Text("Exchange id:         %s", product.exchange_id);
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
            ImGui::TreePop();
        }
    }
    // ImGui::End();
}

void render_transactions()
{
    // ImGui::Begin("Transactions");

    dg_get_transactions_options options = {
        .from_date = "2025-01-01",
        .to_date = "2025-12-01",
        .group_transactions_by_order = true};

    static dg_transactions transactions = {0};

    if (ImGui::Button("Get transactions"))
    {
        dg_get_transactions(&dg, options, &transactions);
    }

    for (size_t i = 0; i < transactions.count; ++i)
    {
        dg_transaction t = transactions.transactions[i];
        if (ImGui::TreeNode("%s", t.date))
        {
            ImGui::Text("id:                                   %d", t.id);
            ImGui::Text("product_id:                           %d", t.product_id);
            ImGui::Text("date:                                 %s", t.date);
            ImGui::Text("buysell:                              %s", t.buysell);
            ImGui::Text("price:                                %.2f", t.price);
            ImGui::Text("quantity:                             %d", t.quantity);
            ImGui::Text("total:                                %.2f", t.total);
            ImGui::Text("order_type_id:                        %d", t.order_type_id);
            ImGui::Text("counter_party:                        %s", t.counter_party);
            ImGui::Text("transfered:                           %s", t.transfered ? "true" : "false");
            ImGui::Text("fx_rate:                              %d", t.fx_rate);
            ImGui::Text("nett_fx_rate:                         %d", t.nett_fx_rate);
            ImGui::Text("gross_fx_rate:                        %d", t.gross_fx_rate);
            ImGui::Text("auto_fx_fee_in_base_currency:         %d", t.auto_fx_fee_in_base_currency);
            ImGui::Text("total_in_base_currency:               %.2f", t.total_in_base_currency);
            ImGui::Text("fee_in_base_currency:                 %.2f", t.fee_in_base_currency);
            ImGui::Text("total_fees_in_base_currency:          %.2f", t.total_fees_in_base_currency);
            ImGui::Text("total_plus_fee_in_base_currency:      %.2f", t.total_plus_fee_in_base_currency);
            ImGui::Text("total_plus_all_fees_in_base_currency: %.2f", t.total_plus_all_fees_in_base_currency);
            ImGui::Text("transaction_type_id:                  %d", t.transaction_type_id);
            ImGui::Text("trading_venue:                        %s", t.trading_venue);
            ImGui::Text("executing_entity_id:                  %s", t.executing_entity_id);
            ImGui::TreePop();
        }
    }
    // ImGui::End();
}

void render_price()
{
    // ImGui::Begin("Price data");

    static ImGuiComboFlags plot_flags = 0;
    const char *periods[] = {"1D", "1W", "1M", "6M", "1Y"};
    static int selected_period_ix = 0;

    // Pass in the preview value visible before opening the combo (it could technically be different contents or not pulled from items[])
    const char *combo_preview_value = periods[selected_period_ix];
    if (ImGui::BeginCombo("Period", combo_preview_value, plot_flags))
    {
        for (int n = 0; n < IM_ARRAYSIZE(periods); n++)
        {
            const bool is_selected = (selected_period_ix == n);
            if (ImGui::Selectable(periods[n], is_selected))
                selected_period_ix = n;

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    dg_price_plot_options opts = {0};
    opts.period = periods[selected_period_ix];
    opts.product = &dg.products.items[0];

    static dg_price_history history = {0};

    if (ImGui::Button("Get"))
    {
        if (!dg_get_price(&history, opts))
        {
            fprintf(stderr, "Failed to get price data\n");
        }
    }

    char plot_title[64];
    if (history.currency)
    {
        snprintf(plot_title, sizeof(plot_title), "Price [%s]", history.currency);
    }
    else
    {
        snprintf(plot_title, sizeof(plot_title), "Price");
    }

    if (ImPlot::BeginPlot(plot_title))
    {
        if (history.chart.n_points > 0)
        {
            float min_val = *(history.chart.prices);
            float max_val = *(history.chart.prices);
            for (size_t i = 0; i < history.chart.n_points; ++i)
            {
                if (history.chart.prices[i] < min_val)
                    min_val = history.chart.prices[i];
                if (history.chart.prices[i] > max_val)
                    max_val = history.chart.prices[i];
            }
            float padding = (max_val - min_val) * 0.1;

            ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
            ImPlot::GetStyle().Use24HourClock = true;
            ImPlot::SetupAxesLimits(history.chart.timestamps[0], history.chart.timestamps[history.chart.n_points - 1], min_val - padding, max_val + padding, ImPlotCond_Always);

            // Plot lines
            ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.25f);
            ImPlot::PlotShaded(history.product->name, history.chart.timestamps, history.chart.prices, history.chart.n_points, -INFINITY);
            ImPlot::PlotLine(history.product->name, history.chart.timestamps, history.chart.prices, history.chart.n_points);

            // Plot tags
            ImVec4 color;

            color = ImVec4(0, 1, 0, 1);
            ImPlot::TagY(history.high_price, color);
            ImPlot::PushStyleColor(ImPlotCol_Line, color);
            ImPlot::PlotInfLines("", &history.high_price, 1, ImPlotInfLinesFlags_Horizontal);
            ImPlot::PopStyleColor();

            color = ImVec4(1, 0, 0, 1);
            ImPlot::TagY(history.low_price, color);
            ImPlot::PushStyleColor(ImPlotCol_Line, color);
            ImPlot::PlotInfLines("", &history.low_price, 1, ImPlotInfLinesFlags_Horizontal);
            ImPlot::PopStyleColor();
        }
        ImPlot::EndPlot();
    }
    // ImGui::End();
}

void render_app()
{
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    if (show_implot_demo_window)
        ImPlot::ShowDemoWindow(&show_implot_demo_window);

    render_login_popup();

    ImGuiIO io = ImGui::GetIO();
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);

    if (ImGui::Begin("Docked Window", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
    {
        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
        if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
        {
            if (ImGui::BeginTabItem("Account info"))
            {
                render_account_info();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Portfolio"))
            {
                render_portfolio();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Transactions"))
            {
                render_transactions();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Products"))
            {
                render_products();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Price"))
            {
                render_price();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main(int, char **)
{
    if (!dg_init())
    {
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
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
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
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
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