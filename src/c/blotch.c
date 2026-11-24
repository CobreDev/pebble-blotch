#include <pebble.h>
#include "settings.h"
#include <pebble-fctx/fctx.h>
#include <pebble-fctx/fpath.h>
#include <pebble-fctx/ffont.h>

#define SCREENSHOT_MODE 0
#define SCREENSHOT_PRESET 0

#define TIME_FONT_SIZE_BASE 42
#define DATE_FONT_SIZE_BASE 24
#define WEEK_FONT_SIZE_BASE 20
#define WEEKDAY_WIDTH_BASE 18
#define REFERENCE_HEIGHT 168
#define REFERENCE_WIDTH 144

#if SCREENSHOT_MODE
static const struct {
    GColor background;
    GColor time;
    GColor date;
    GColor week;
    GColor highlight;
} presets[] = {
    { .background = GColorOxfordBlue, .time = GColorWhite, .date = GColorLightGray, .week = GColorLightGray, .highlight = GColorWhite },
    { .background = GColorDarkGreen, .time = GColorElectricBlue, .date = GColorVividCerulean, .week = GColorVividCerulean, .highlight = GColorElectricBlue },
    { .background = GColorImperialPurple, .time = GColorMediumSpringGreen, .date = GColorJaegerGreen, .week = GColorJaegerGreen, .highlight = GColorSunsetOrange },
    { .background = GColorBlack, .time = GColorSunsetOrange, .date = GColorMelon, .week = GColorMelon, .highlight = GColorJaegerGreen },
    { .background = GColorWhite, .time = GColorBlack, .date = GColorBlack, .week = GColorBlack, .highlight = GColorBlack },
    { .background = GColorBlack, .time = GColorWhite, .date = GColorWhite, .week = GColorWhite, .highlight = GColorWhite },
};
#endif

static Window *s_main_window;
static Layer *s_canvas_layer;
static int s_current_day_index = 0;

static int s_time_font_size;
static int s_date_font_size;
static int s_week_font_size;
static int s_weekday_width;

static char s_month_buffer[12];
static char s_day_buffer[4];

static FFont* s_font_oswald;
static FFont* s_font_leco;

static ClaySettings settings;

static const char *weekdays[] = {"S", "M", "T", "W", "T", "F", "S"};

static void calculate_sizes(int height) {
    int scale = (height * 100) / REFERENCE_HEIGHT;
    s_time_font_size = (TIME_FONT_SIZE_BASE * scale) / 100;
    s_date_font_size = (DATE_FONT_SIZE_BASE * scale) / 100;
    s_week_font_size = (WEEK_FONT_SIZE_BASE * scale) / 100;
    s_weekday_width = (WEEKDAY_WIDTH_BASE * scale) / 100;
#ifdef PBL_ROUND
    s_weekday_width = (s_weekday_width * 94) / 100;
#endif
}

// Draws one line of text whose vertical center is y_top + size / 2.
static void prv_draw_text(FContext *fctx, FFont *font, int size, GColor color,
                          int x, int y_top, GTextAlignment align, const char *text) {
    fctx_begin_fill(fctx);
    fctx_set_fill_color(fctx, color);
    fctx_set_text_em_height(fctx, font, size);
    fctx_set_offset(fctx, FPointI(x, y_top + size / 2));
    fctx_draw_string(fctx, text, font, align, FTextAnchorMiddle);
    fctx_end_fill(fctx);
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
    GRect full_bounds = layer_get_bounds(layer);
    GRect unobstructed_bounds = layer_get_unobstructed_bounds(layer);
    calculate_sizes(full_bounds.size.h);

    int y_offset = unobstructed_bounds.origin.y - full_bounds.origin.y;
    int available_height = unobstructed_bounds.size.h;

    FContext fctx;
    fctx_init_context(&fctx, ctx);
    fctx_set_color_bias(&fctx, 0);

#ifdef PBL_COLOR
    fctx_enable_aa(true);
#endif

    static char time_buffer[6];
#if SCREENSHOT_MODE
    snprintf(time_buffer, sizeof(time_buffer), "10:10");
    snprintf(s_month_buffer, sizeof(s_month_buffer), "July");
    snprintf(s_day_buffer, sizeof(s_day_buffer), "17");
    s_current_day_index = 0;
#else
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);
    strftime(time_buffer, sizeof(time_buffer), clock_is_24h_style() ? "%H:%M" : "%l:%M", tick_time);
#endif

    int time_layer_y = y_offset + (available_height - s_time_font_size) / 2;
    int time_margin = (PBL_IF_ROUND_ELSE(15, 10) * full_bounds.size.w) / REFERENCE_WIDTH;
    prv_draw_text(&fctx, s_font_leco, s_time_font_size, settings.color_time,
                  full_bounds.size.w - time_margin, time_layer_y, GTextAlignmentRight, time_buffer);

    int date_layer_y = time_layer_y - s_date_font_size - 5;
    int date_offset = (PBL_IF_ROUND_ELSE(25, 10) * full_bounds.size.w) / REFERENCE_WIDTH;

    int day_chars = strlen(s_day_buffer);
    int day_width = (s_date_font_size * day_chars) / 2;
    int space_width = s_date_font_size / 10;
    prv_draw_text(&fctx, s_font_oswald, s_date_font_size, settings.color_date,
                  full_bounds.size.w - date_offset - day_width - space_width, date_layer_y,
                  GTextAlignmentRight, s_month_buffer);
    prv_draw_text(&fctx, s_font_oswald, s_date_font_size, settings.color_highlight,
                  full_bounds.size.w - date_offset, date_layer_y,
                  GTextAlignmentRight, s_day_buffer);

    int week_layer_y = time_layer_y + s_time_font_size + (available_height - (time_layer_y - y_offset + s_time_font_size) - s_week_font_size) / 8;
    int total_weekdays_width = 7 * s_weekday_width;
    int start_x = (full_bounds.size.w - total_weekdays_width) / 2;

    for (int i = 0; i < 7; i++) {
        prv_draw_text(&fctx, s_font_oswald, s_week_font_size,
                      i == s_current_day_index ? settings.color_highlight : settings.color_week,
                      start_x + i * s_weekday_width + s_weekday_width / 2, week_layer_y,
                      GTextAlignmentCenter, weekdays[i]);
    }

    graphics_context_set_stroke_width(ctx, 3);
    graphics_context_set_stroke_color(ctx, settings.color_highlight);
    int underline_width = (PBL_IF_ROUND_ELSE(12, 18) * s_weekday_width) / WEEKDAY_WIDTH_BASE;
    int x_position = start_x + s_current_day_index * s_weekday_width + (s_weekday_width / 2) - (underline_width / 2);
    GRect underline_rect = GRect(x_position, week_layer_y + s_week_font_size + 2, underline_width, 2);
    graphics_draw_rect(ctx, underline_rect);

    fctx_deinit_context(&fctx);
}

static void prv_default_settings() {
#if SCREENSHOT_MODE
    settings.color_background = presets[SCREENSHOT_PRESET].background;
    settings.color_time = presets[SCREENSHOT_PRESET].time;
    settings.color_date = presets[SCREENSHOT_PRESET].date;
    settings.color_week = presets[SCREENSHOT_PRESET].week;
    settings.color_highlight = presets[SCREENSHOT_PRESET].highlight;
#else
    // Keep in sync with the defaultValue entries in src/pkjs/config.js
    settings.color_background = GColorOxfordBlue;
    settings.color_time = GColorWhite;
    settings.color_date = GColorLightGray;
    settings.color_week = GColorLightGray;
    settings.color_highlight = GColorWhite;
#endif
}

static void prv_load_settings() {
    prv_default_settings();
    if (persist_exists(SETTINGS_KEY)) {
        persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
    }
}

static void prv_update_display() {
    layer_mark_dirty(s_canvas_layer);
    window_set_background_color(s_main_window, settings.color_background);
}

static void prv_save_settings() {
    persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
    prv_update_display();
}

static void update_time() {
#if !SCREENSHOT_MODE
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    static int last_day = -1;
    if (last_day != tick_time->tm_mday) {
        strftime(s_month_buffer, sizeof(s_month_buffer), "%B", tick_time);
        snprintf(s_day_buffer, sizeof(s_day_buffer), "%d", tick_time->tm_mday);
        last_day = tick_time->tm_mday;
    }

    s_current_day_index = tick_time->tm_wday;
#endif
    layer_mark_dirty(s_canvas_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time();
}

// Copies the color sent under `key` into `dst`. Returns true if it changed.
static bool prv_apply_color(DictionaryIterator *iter, uint32_t key, GColor *dst) {
    Tuple *tuple = dict_find(iter, key);
    if (!tuple) {
        return false;
    }
    GColor new_color = GColorFromHEX(tuple->value->int32);
    if (gcolor_equal(*dst, new_color)) {
        return false;
    }
    *dst = new_color;
    return true;
}

static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
    bool settings_changed = false;
    settings_changed |= prv_apply_color(iter, MESSAGE_KEY_backgroundColor, &settings.color_background);
    settings_changed |= prv_apply_color(iter, MESSAGE_KEY_timeColor, &settings.color_time);
    settings_changed |= prv_apply_color(iter, MESSAGE_KEY_dateColor, &settings.color_date);
    settings_changed |= prv_apply_color(iter, MESSAGE_KEY_weekColor, &settings.color_week);
    settings_changed |= prv_apply_color(iter, MESSAGE_KEY_highlightColor, &settings.color_highlight);
    if (settings_changed) {
        prv_save_settings();
    }
}

static void unobstructed_did_change(void *context) {
    layer_mark_dirty(s_canvas_layer);
}

static void main_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    window_set_background_color(window, settings.color_background);

    s_font_oswald = ffont_create_from_resource(RESOURCE_ID_FONT_OSWALD_MEDIUM);
    s_font_leco = ffont_create_from_resource(RESOURCE_ID_FONT_LECO_REGULAR);

    s_canvas_layer = layer_create(bounds);
    layer_set_update_proc(s_canvas_layer, canvas_update_proc);
    layer_add_child(window_layer, s_canvas_layer);

    update_time();
}

static void main_window_unload(Window *window) {
    layer_destroy(s_canvas_layer);
    ffont_destroy(s_font_oswald);
    ffont_destroy(s_font_leco);
}

static void init() {
    prv_load_settings();

    app_message_register_inbox_received(prv_inbox_received_handler);
    app_message_open(128, 128);

    s_main_window = window_create();
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload
    });

    window_stack_push(s_main_window, true);
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

    UnobstructedAreaHandlers handlers = {
        .did_change = unobstructed_did_change
    };
    unobstructed_area_service_subscribe(handlers, NULL);
}

static void deinit() {
    unobstructed_area_service_unsubscribe();
    window_destroy(s_main_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
