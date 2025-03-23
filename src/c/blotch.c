#include <pebble.h>
#include "settings.h"

#define TIME_LAYER_HEIGHT 50
#define DATE_LAYER_HEIGHT 30
#define WEEK_LAYER_HEIGHT 30
#define WEEKDAY_WIDTH PBL_IF_ROUND_ELSE(17, 18)
#define DATE_SUFFIX_WIDTH 20

static Window *s_main_window;
static TextLayer *s_time_layer, *s_date_layer, *s_date_suffix_layer;
static TextLayer *s_week_layers[7];
static Layer *s_underline_layer;
static int s_current_day_index = -1;

ClaySettings settings;

static const char *weekdays[] = {"S", "M", "T", "W", "T", "F", "S"};
  
static void underline_layer_update_proc(Layer *layer, GContext *ctx) {
    graphics_context_set_stroke_width(ctx, 3);
    graphics_context_set_stroke_color(ctx, settings.color_highlight);

    GRect day_bounds = layer_get_frame(text_layer_get_layer(s_week_layers[s_current_day_index]));
    int underline_width = PBL_IF_ROUND_ELSE(12, 18);
    int x_position = day_bounds.origin.x + (day_bounds.size.w / 2) - (underline_width / 2);
    GRect underline_rect = GRect(x_position, day_bounds.origin.y + day_bounds.size.h, underline_width, 2);
    graphics_draw_rect(ctx, underline_rect);
}

static void prv_default_settings() {
    settings.color_background = GColorOxfordBlue;
    settings.color_time = GColorWhite;
    settings.color_date = GColorLightGray;
    settings.color_week = GColorLightGray;
    settings.color_highlight = GColorWhite;
    settings.timeFont = 1; // Default to LECO
}

static void prv_load_settings() {
    prv_default_settings();
    if (persist_exists(SETTINGS_KEY)) {
        persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
    }
}

static void prv_save_settings() {
    persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
    prv_update_display();
}

///////////////////////////
////// DEMO SETTINGS //////
///////////////////////////

static void update_time() {
    // Hardcoded time and date for demo purposes
    struct tm tick_time;
    tick_time.tm_hour = 10;
    tick_time.tm_min = 10;
    tick_time.tm_mday = 17;
    tick_time.tm_mon = 11; // July (0-based index)
    tick_time.tm_wday = 0; // Sunday (0-based index)

    static char time_buffer[6];
    static char date_buffer[12];
    static char date_suffix_buffer[3]; // Buffer for the last two characters of the date
    static char month_buffer[10]; // Buffer for the month name

    static int last_hour = -1;
    static int last_minute = -1;
    static int last_day = -1;

    if (last_hour != tick_time.tm_hour || last_minute != tick_time.tm_min) {
        strftime(time_buffer, sizeof(time_buffer), clock_is_24h_style() ? "%H:%M" : "%l:%M", &tick_time);
        text_layer_set_text(s_time_layer, time_buffer);
        last_hour = tick_time.tm_hour;
        last_minute = tick_time.tm_min;
    }

    if (last_day != tick_time.tm_mday) {
        strftime(date_buffer, sizeof(date_buffer), "%B %d", &tick_time);
        // Split the date string into month name and last two characters
        snprintf(month_buffer, sizeof(month_buffer), "%.*s ", (int)(strlen(date_buffer) - 3), date_buffer); // Add space after month name
        snprintf(date_suffix_buffer, sizeof(date_suffix_buffer), "%s", &date_buffer[strlen(date_buffer) - 2]);

        text_layer_set_text(s_date_layer, month_buffer);
        text_layer_set_text(s_date_suffix_layer, date_suffix_buffer);
        last_day = tick_time.tm_mday;
    }

    // Calculate current weekday index
    int new_day_index = tick_time.tm_wday; // Sunday = 0, Saturday = 6
    if (s_current_day_index != new_day_index) {
        s_current_day_index = new_day_index;
        // Trigger redraw of the underline and update display
        layer_mark_dirty(s_underline_layer);
        prv_update_display();
    }
}

// static void update_time() {
//     time_t temp = time(NULL);
//     struct tm *tick_time = localtime(&temp);

//     static char time_buffer[6], date_buffer[12], date_suffix_buffer[3], month_buffer[10];
//     static int last_hour = -1, last_minute = -1, last_day = -1;

//     if (last_hour != tick_time->tm_hour || last_minute != tick_time->tm_min) {
//         strftime(time_buffer, sizeof(time_buffer), clock_is_24h_style() ? "%H:%M" : "%l:%M", tick_time);
//         text_layer_set_text(s_time_layer, time_buffer);
//         last_hour = tick_time->tm_hour;
//         last_minute = tick_time->tm_min;
//     }

//     if (last_day != tick_time->tm_mday) {
//         strftime(date_buffer, sizeof(date_buffer), "%B %d", tick_time);
//         snprintf(month_buffer, sizeof(month_buffer), "%.*s ", (int)(strlen(date_buffer) - 3), date_buffer);
//         snprintf(date_suffix_buffer, sizeof(date_suffix_buffer), "%s", &date_buffer[strlen(date_buffer) - 2]);

//         text_layer_set_text(s_date_layer, month_buffer);
//         text_layer_set_text(s_date_suffix_layer, date_suffix_buffer);
//         last_day = tick_time->tm_mday;
//     }

//     int new_day_index = tick_time->tm_wday;
//     if (s_current_day_index != new_day_index) {
//         s_current_day_index = new_day_index;
//         layer_mark_dirty(s_underline_layer);
//         prv_update_display();
//     }
// }

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time();
}

static void prv_update_display() {
    // Update text colors
    text_layer_set_text_color(s_time_layer, settings.color_time);
    text_layer_set_text_color(s_date_layer, settings.color_date);
    text_layer_set_text_color(s_date_suffix_layer, settings.color_highlight);

    // Update font for the time layer
    text_layer_set_font(s_time_layer, get_font_for_selection(settings.timeFont));

    // Update week layers
    for (int i = 0; i < 7; i++) {
        text_layer_set_text_color(s_week_layers[i], i == s_current_day_index ? settings.color_highlight : settings.color_week);
        layer_mark_dirty(text_layer_get_layer(s_week_layers[i]));
    }

    // Mark layers dirty to refresh
    layer_mark_dirty(text_layer_get_layer(s_time_layer));
    layer_mark_dirty(text_layer_get_layer(s_date_layer));
    layer_mark_dirty(text_layer_get_layer(s_date_suffix_layer));
    layer_mark_dirty(s_underline_layer);

    // Update background color
    window_set_background_color(s_main_window, settings.color_background);
}

static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
    bool settings_changed = false;

    Tuple *color_background_t = dict_find(iter, MESSAGE_KEY_backgroundColor);
    if (color_background_t) {
        GColor new_color = GColorFromHEX(color_background_t->value->int32);
        if (!gcolor_equal(settings.color_background, new_color)) {
            settings.color_background = new_color;
            settings_changed = true;
        }
    }
    Tuple *color_time_t = dict_find(iter, MESSAGE_KEY_timeColor);
    if (color_time_t) {
        GColor new_color = GColorFromHEX(color_time_t->value->int32);
        if (!gcolor_equal(settings.color_time, new_color)) {
            settings.color_time = new_color;
            settings_changed = true;
        }
    }
    Tuple *color_date_t = dict_find(iter, MESSAGE_KEY_dateColor);
    if (color_date_t) {
        GColor new_color = GColorFromHEX(color_date_t->value->int32);
        if (!gcolor_equal(settings.color_date, new_color)) {
            settings.color_date = new_color;
            settings_changed = true;
        }
    }
    Tuple *color_week_t = dict_find(iter, MESSAGE_KEY_weekColor);
    if (color_week_t) {
        GColor new_color = GColorFromHEX(color_week_t->value->int32);
        if (!gcolor_equal(settings.color_week, new_color)) {
            settings.color_week = new_color;
            settings_changed = true;
        }
    }
    Tuple *color_highlight_t = dict_find(iter, MESSAGE_KEY_highlightColor);
    if (color_highlight_t) {
        GColor new_color = GColorFromHEX(color_highlight_t->value->int32);
        if (!gcolor_equal(settings.color_highlight, new_color)) {
            settings.color_highlight = new_color;
            settings_changed = true;
        }
    }
    Tuple *time_font_t = dict_find(iter, MESSAGE_KEY_timeFont);
    if (time_font_t) {
        int new_font = time_font_t->value->int32;
        if (settings.timeFont != new_font) {
            settings.timeFont = new_font;
            settings_changed = true;
        }
    }
    if (settings_changed) {
        prv_save_settings();
    }
}

static void unobstructed_will_change(GRect final_unobstructed_screen_area, void *context) {}

static void unobstructed_did_change(void *context) {
    Layer *window_layer = window_get_root_layer(s_main_window);
    GRect bounds = layer_get_unobstructed_bounds(window_layer);

    static GRect last_bounds = { .origin = {0, 0}, .size = {0, 0} };
    if (grect_equal(&bounds, &last_bounds)) {
        return;
    }
    last_bounds = bounds;

    int time_layer_y = (bounds.size.h - TIME_LAYER_HEIGHT) / 2;
    layer_set_frame(text_layer_get_layer(s_time_layer), GRect(0, time_layer_y, bounds.size.w - 10, TIME_LAYER_HEIGHT));

    int date_layer_y = (time_layer_y - DATE_LAYER_HEIGHT) / 1;
    layer_set_frame(text_layer_get_layer(s_date_layer), GRect(0, date_layer_y, bounds.size.w - PBL_IF_ROUND_ELSE(25, 10) - DATE_SUFFIX_WIDTH, DATE_LAYER_HEIGHT));

    layer_set_frame(text_layer_get_layer(s_date_suffix_layer), GRect(bounds.size.w - PBL_IF_ROUND_ELSE(25, 10) - DATE_SUFFIX_WIDTH, date_layer_y, DATE_SUFFIX_WIDTH, DATE_LAYER_HEIGHT));

    int week_layer_y = time_layer_y + TIME_LAYER_HEIGHT + (bounds.size.h - (time_layer_y + TIME_LAYER_HEIGHT) - WEEK_LAYER_HEIGHT) / 8;
    int total_weekdays_width = 7 * WEEKDAY_WIDTH;
    int start_x = (bounds.size.w - total_weekdays_width) / 2;

    for (int i = 0; i < 7; i++) {
        layer_set_frame(text_layer_get_layer(s_week_layers[i]), GRect(start_x + i * WEEKDAY_WIDTH, week_layer_y, 20, WEEK_LAYER_HEIGHT));
    }

    layer_set_frame(s_underline_layer, bounds);
    layer_mark_dirty(s_underline_layer);
}

static GFont get_font_for_selection(int font_selection) {
    switch (font_selection) {
        case 1:
            return fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS);
        case 2:
            return fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);
        case 3:
            return fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT);
        default:
            return fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS); // Default to LECO
    }
}

static void configure_text_layer(TextLayer *layer, GRect frame, GFont font, GTextAlignment alignment, GColor background_color, GColor text_color) {
    text_layer_set_font(layer, font);
    text_layer_set_text_alignment(layer, alignment);
    text_layer_set_background_color(layer, background_color);
    text_layer_set_text_color(layer, text_color);
    layer_set_frame(text_layer_get_layer(layer), frame);
}

static void main_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_unobstructed_bounds(window_layer);

    window_set_background_color(window, settings.color_background);

    int time_layer_y = (bounds.size.h - TIME_LAYER_HEIGHT) / 2;
    s_time_layer = text_layer_create(GRect(0, time_layer_y, bounds.size.w - 10, TIME_LAYER_HEIGHT));
    configure_text_layer(s_time_layer, GRect(0, time_layer_y, bounds.size.w - 10, TIME_LAYER_HEIGHT), 
                         get_font_for_selection(settings.timeFont), GTextAlignmentRight, GColorClear, settings.color_time);
    layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

    int date_layer_y = (time_layer_y - DATE_LAYER_HEIGHT) / 1;
    s_date_layer = text_layer_create(GRect(0, date_layer_y, bounds.size.w - PBL_IF_ROUND_ELSE(25, 10) - DATE_SUFFIX_WIDTH, DATE_LAYER_HEIGHT));
    configure_text_layer(s_date_layer, GRect(0, date_layer_y, bounds.size.w - PBL_IF_ROUND_ELSE(25, 10) - DATE_SUFFIX_WIDTH, DATE_LAYER_HEIGHT), 
                         fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GTextAlignmentRight, GColorClear, settings.color_date);
    layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

    s_date_suffix_layer = text_layer_create(GRect(bounds.size.w - PBL_IF_ROUND_ELSE(25, 10) - DATE_SUFFIX_WIDTH, date_layer_y, DATE_SUFFIX_WIDTH, DATE_LAYER_HEIGHT));
    configure_text_layer(s_date_suffix_layer, GRect(bounds.size.w - PBL_IF_ROUND_ELSE(25, 10) - DATE_SUFFIX_WIDTH, date_layer_y, DATE_SUFFIX_WIDTH, DATE_LAYER_HEIGHT), 
                         fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GTextAlignmentLeft, GColorClear, settings.color_highlight);
    layer_add_child(window_layer, text_layer_get_layer(s_date_suffix_layer));

    int week_layer_y = time_layer_y + TIME_LAYER_HEIGHT + (bounds.size.h - (time_layer_y + TIME_LAYER_HEIGHT) - WEEK_LAYER_HEIGHT) / 8;
    int total_weekdays_width = 7 * WEEKDAY_WIDTH;
    int start_x = (bounds.size.w - total_weekdays_width) / 2;

    for (int i = 0; i < 7; i++) {
        s_week_layers[i] = text_layer_create(GRect(start_x + i * WEEKDAY_WIDTH, week_layer_y, 20, WEEK_LAYER_HEIGHT));
        configure_text_layer(s_week_layers[i], GRect(start_x + i * WEEKDAY_WIDTH, week_layer_y, 20, WEEK_LAYER_HEIGHT), 
                             fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GTextAlignmentCenter, GColorClear, settings.color_date);
        text_layer_set_text(s_week_layers[i], weekdays[i]);
        layer_add_child(window_layer, text_layer_get_layer(s_week_layers[i]));
    }

    s_underline_layer = layer_create(bounds);
    layer_set_update_proc(s_underline_layer, underline_layer_update_proc);
    layer_add_child(window_layer, s_underline_layer);

    update_time();
}

static void main_window_unload(Window *window) {
    text_layer_destroy(s_time_layer);
    text_layer_destroy(s_date_layer);
    text_layer_destroy(s_date_suffix_layer);
    for (int i = 0; i < 7; i++) {
        text_layer_destroy(s_week_layers[i]);
    }
    layer_destroy(s_underline_layer);
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
        .will_change = unobstructed_will_change,
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