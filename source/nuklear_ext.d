module nuklear_ext;

import nuklear;

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
