/**********************************************************************************************
*
*   raylib-nuklear-example - Example of using Nuklear with Raylib.
*
*   LICENSE: zlib/libpng
*
*   nuklear_raylib is licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software:
*
*   Copyright (c) 2020 Rob Loach (@RobLoach)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

import raylib;
import nuklear;
import raylib_nuklear;

int nk_tab(nk_context* ctx, const char* title, int active) {
    auto f = cast(nk_user_font*) ctx.style.font;
    float text_width = f.width(f.userdata, f.height, title, nk_strlen(title));
    float widget_width = text_width + 3 * ctx.style.button.padding.x;
    nk_layout_row_push(ctx, widget_width);
    auto c = ctx.style.button.normal;
    if (active) {
        ctx.style.button.normal = ctx.style.button.active;
    }
    int r = nk_button_label(ctx, title);
    ctx.style.button.normal = c;
    return r;
}

int main() {
    // Initialization
    //--------------------------------------------------------------------------------------

    enum SCREEN_WIDTH = 700;
    enum SCREEN_HEIGHT = 394;
    enum FONT_SIZE = 16;
    enum PAD = 8;

    SetConfigFlags(raylib.FLAG_WINDOW_HIGHDPI);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "[dray-nuklear] demo");

    auto dpi_scale = cast(int) raylib.GetWindowScaleDPI().x;

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    /* GUI */
    auto bg = ColorToNuklearF(Colors.SKYBLUE);
    auto ui_font = raylib.LoadFontEx("./res/SourceSansPro-Regular.ttf", FONT_SIZE, null, 0);
    raylib.SetTextureFilter(ui_font.texture, raylib.TextureFilter.TEXTURE_FILTER_POINT);
    auto ctx = InitNuklearEx(ui_font, FONT_SIZE);

    // nk_color[nk_style_colors.NK_COLOR_COUNT] table;
    // table[nk_style_colors.NK_COLOR_TEXT] = nk_rgba(190, 190, 190, 255);
    // table[nk_style_colors.NK_COLOR_WINDOW] = nk_rgba(30, 33, 40, 215);
    // table[nk_style_colors.NK_COLOR_HEADER] = nk_rgba(181, 45, 69, 220);
    // table[nk_style_colors.NK_COLOR_BORDER] = nk_rgba(51, 55, 67, 255);
    // table[nk_style_colors.NK_COLOR_BUTTON] = nk_rgba(181, 45, 69, 255);
    // table[nk_style_colors.NK_COLOR_BUTTON_HOVER] = nk_rgba(190, 50, 70, 255);
    // table[nk_style_colors.NK_COLOR_BUTTON_ACTIVE] = nk_rgba(195, 55, 75, 255);
    // table[nk_style_colors.NK_COLOR_TOGGLE] = nk_rgba(51, 55, 67, 255);
    // table[nk_style_colors.NK_COLOR_TOGGLE_HOVER] = nk_rgba(45, 60, 60, 255);
    // table[nk_style_colors.NK_COLOR_TOGGLE_CURSOR] = nk_rgba(181, 45, 69, 255);
    // table[nk_style_colors.NK_COLOR_SELECT] = nk_rgba(51, 55, 67, 255);
    // table[nk_style_colors.NK_COLOR_SELECT_ACTIVE] = nk_rgba(181, 45, 69, 255);
    // table[nk_style_colors.NK_COLOR_SLIDER] = nk_rgba(51, 55, 67, 255);
    // table[nk_style_colors.NK_COLOR_SLIDER_CURSOR] = nk_rgba(181, 45, 69, 255);
    // table[nk_style_colors.NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(186, 50, 74, 255);
    // table[nk_style_colors.NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(191, 55, 79, 255);
    // table[nk_style_colors.NK_COLOR_PROPERTY] = nk_rgba(51, 55, 67, 255);
    // table[nk_style_colors.NK_COLOR_EDIT] = nk_rgba(51, 55, 67, 225);
    // table[nk_style_colors.NK_COLOR_EDIT_CURSOR] = nk_rgba(190, 190, 190, 255);
    // table[nk_style_colors.NK_COLOR_COMBO] = nk_rgba(51, 55, 67, 255);
    // table[nk_style_colors.NK_COLOR_CHART] = nk_rgba(51, 55, 67, 255);
    // table[nk_style_colors.NK_COLOR_CHART_COLOR] = nk_rgba(170, 40, 60, 255);
    // table[nk_style_colors.NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 0, 0, 255);
    // table[nk_style_colors.NK_COLOR_SCROLLBAR] = nk_rgba(30, 33, 40, 255);
    // table[nk_style_colors.NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(64, 84, 95, 255);
    // table[nk_style_colors.NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(70, 90, 100, 255);
    // table[nk_style_colors.NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(75, 95, 105, 255);
    // table[nk_style_colors.NK_COLOR_TAB_HEADER] = nk_rgba(181, 45, 69, 220);
    // nk_style_from_table(ctx, cast(nk_color*) table);
    ctx.style.button.padding.x = PAD;

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Update
        UpdateNuklear(ctx);

        // GUI
        // auto window_bounds = nk_rect(0, 0, GetRenderWidth(), GetRenderHeight());
        auto window_bounds = Rectangle(0, 0,
            GetRenderWidth() / GetWindowScaleDPI()
                .x,
            GetRenderHeight() / GetWindowScaleDPI().x
        );
        if (nk_begin(ctx, "Demo", RectangleToNuklear(ctx, window_bounds),
                nk_panel_flags.NK_WINDOW_BORDER | nk_panel_flags.NK_WINDOW_TITLE)) {
            enum EASY = 0;
            enum HARD = 1;

            nk_style_push_vec2(ctx, &ctx.style.window.spacing, nk_vec2(0, 0));
            nk_style_push_float(ctx, &ctx.style.button.rounding, 0);
            nk_layout_row_begin(ctx, nk_layout_format.NK_STATIC, 30, 2);
            enum TAB1 = 0;
            enum TAB2 = 1;
            static int tab_state = TAB1;
            if (nk_tab(ctx, "TAB1", tab_state == TAB1)) {
                tab_state = TAB1;
            }
            if (nk_tab(ctx, "TAB2", tab_state == TAB2)) {
                tab_state = TAB2;
            }

            if (tab_state == TAB1) {
                static int op = EASY;

                nk_layout_row_dynamic(ctx, PAD, 1);

                nk_layout_row_static(ctx, 30, 80, 1);
                if (nk_button_label(ctx, "button"))
                    TraceLog(TraceLogLevel.LOG_INFO, "button pressed");

                nk_layout_row_dynamic(ctx, 30, 2);
                if (nk_option_label(ctx, "easy", op == EASY))
                    op = EASY;
                if (nk_option_label(ctx, "hard", op == HARD))
                    op = HARD;
            } else if (tab_state == TAB2) {
                static int property = 20;

                nk_layout_row_dynamic(ctx, PAD, 1);

                nk_layout_row_dynamic(ctx, 25, 1);
                nk_property_int(ctx, "Compression:", 0, &property, 100, 10, 1);

                nk_layout_row_dynamic(ctx, 20, 1);
                nk_label(ctx, "background:", nk_text_alignment.NK_TEXT_LEFT);
                nk_layout_row_dynamic(ctx, 25, 1);
                if (nk_combo_begin_color(ctx, nk_rgb_cf(bg), nk_vec2(nk_widget_width(ctx), 400))) {
                    nk_layout_row_dynamic(ctx, 120, 1);
                    bg = nk_color_picker(ctx, bg, nk_color_format.NK_RGBA);
                    nk_layout_row_dynamic(ctx, 25, 1);
                    bg.r = nk_propertyf(ctx, "#R:", 0, bg.r, 1.0f, 0.01f, 0.005f);
                    bg.g = nk_propertyf(ctx, "#G:", 0, bg.g, 1.0f, 0.01f, 0.005f);
                    bg.b = nk_propertyf(ctx, "#B:", 0, bg.b, 1.0f, 0.01f, 0.005f);
                    bg.a = nk_propertyf(ctx, "#A:", 0, bg.a, 1.0f, 0.01f, 0.005f);
                    nk_combo_end(ctx);
                }
            }

            nk_style_pop_float(ctx);
            nk_style_pop_vec2(ctx);
        }

        nk_end(ctx);

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(ColorFromNuklearF(bg));

        DrawNuklear(ctx);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadNuklear(ctx); // Unload the Nuklear GUI
    CloseWindow();
    //--------------------------------------------------------------------------------------

    return 0;
}
