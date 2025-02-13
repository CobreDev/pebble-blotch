#include <pebble.h>
#include "settings.h"

static Window *s_main_window;
static TextLayer *s_time_layer, *s_date_layer;
static TextLayer *s_week_layers[7]; // One text layer per weekday
static Layer *s_background_layer;
static Layer *s_underline_layer;
static int s_current_day_index = -1; // Initialize to an invalid value

// A struct for our specific settings (see main.h)
ClaySettings settings;

static const char *weekdays[] = {"S", "M", "T", "W", "T", "F", "S"};
static const int underline_positions[] = {10, 28, 46, 64, 82, 100, 118}; // x-coordinates for each day

static void underline_layer_update_proc(Layer *layer, GContext *ctx) {
    graphics_context_set_stroke_width(ctx, 3);
    graphics_context_set_stroke_color(ctx, settings.color_secondary); // Change underline color to white
    
    // Calculate the x position based on the current day index
    GRect day_bounds = layer_get_frame(text_layer_get_layer(s_week_layers[s_current_day_index]));
    int x_position = day_bounds.origin.x + (day_bounds.size.w / 2) - 9; // Center the underline within the text layer
    GRect underline_rect = GRect(x_position, day_bounds.origin.y + day_bounds.size.h, 18, 2); // Vertical position, Width, and Height of underline
    graphics_draw_rect(ctx, underline_rect);
}

// Initialize the default settings
static void prv_default_settings() {
    settings.color_background = GColorBlack;
    settings.color_primary = GColorWhite;
    settings.color_secondary = GColorLightGray;
}

// Read settings from persistent storage
static void prv_load_settings() {
    // Load the default settings
    prv_default_settings();
    // Read settings from persistent storage, if they exist
    persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// Save the settings to persistent storage
static void prv_save_settings() {
    persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
    // Update the display based on new settings
    prv_update_display();
}

static void update_time() {
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    static char time_buffer[6];
    static char date_buffer[12];

    static int last_hour = -1;
    static int last_minute = -1;
    static int last_day = -1;

    if (last_hour != tick_time->tm_hour || last_minute != tick_time->tm_min) {
        strftime(time_buffer, sizeof(time_buffer), clock_is_24h_style() ? "%H:%M" : "%l:%M", tick_time);
        text_layer_set_text(s_time_layer, time_buffer);
        last_hour = tick_time->tm_hour;
        last_minute = tick_time->tm_min;
    }

    if (last_day != tick_time->tm_mday) {
        strftime(date_buffer, sizeof(date_buffer), "%B %d", tick_time);
        text_layer_set_text(s_date_layer, date_buffer);
        last_day = tick_time->tm_mday;
    }

    // Calculate current weekday index
    int new_day_index = tick_time->tm_wday; // Sunday = 0, Saturday = 6
    if (s_current_day_index != new_day_index) {
        s_current_day_index = new_day_index;
        // Trigger redraw of the underline
        layer_mark_dirty(s_underline_layer);
    }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time();
}

// Update the display elements
static void prv_update_display() {
    text_layer_set_text_color(s_time_layer, settings.color_primary);
    text_layer_set_text_color(s_date_layer, settings.color_secondary);
    for (int i = 0; i < 7; i++) {
        text_layer_set_text_color(s_week_layers[i], settings.color_secondary);
        layer_mark_dirty(text_layer_get_layer(s_week_layers[i]));
    }

    layer_mark_dirty(text_layer_get_layer(s_time_layer));
    layer_mark_dirty(text_layer_get_layer(s_date_layer));
    layer_mark_dirty(s_underline_layer);
    window_set_background_color(s_main_window, settings.color_background);
}

// Handle the response from AppMessage
static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
    // Colors
    Tuple *color_background_t = dict_find(iter, MESSAGE_KEY_backgroundColor);
    if (color_background_t) {
        settings.color_background = GColorFromHEX(color_background_t->value->int32);
    }
    Tuple *color_primary_t = dict_find(iter, MESSAGE_KEY_primaryColor);
    if (color_primary_t) {
        settings.color_primary = GColorFromHEX(color_primary_t->value->int32);
    }
    Tuple *color_secondary_t = dict_find(iter, MESSAGE_KEY_secondaryColor);
    if (color_secondary_t) {
        settings.color_secondary = GColorFromHEX(color_secondary_t->value->int32);
    }

    // Save the new settings to persistent storage
    prv_save_settings();
}

static void unobstructed_will_change(GRect final_unobstructed_screen_area, void *context) {
    // Handle changes before the unobstructed area changes
}

static void unobstructed_did_change(void *context) {
    // Handle changes after the unobstructed area changes
    Layer *window_layer = window_get_root_layer(s_main_window);
    GRect bounds = layer_get_unobstructed_bounds(window_layer);

    static GRect last_bounds = { .origin = {0, 0}, .size = {0, 0} };
    if (grect_equal(&bounds, &last_bounds)) {
        return; // No change in bounds, no need to update layout
    }
    last_bounds = bounds;

    // Update the layout based on the new bounds
    // Time Layer
    int time_layer_height = 50;
    int time_layer_y = (bounds.size.h - time_layer_height) / 2;
    layer_set_frame(text_layer_get_layer(s_time_layer), GRect(0, time_layer_y, bounds.size.w - 10, time_layer_height));

    // Date Layer
    int date_layer_height = 30;
    int date_layer_y = (time_layer_y - date_layer_height) / 1;
    layer_set_frame(text_layer_get_layer(s_date_layer), GRect(0, date_layer_y, bounds.size.w - 10, date_layer_height));

    // Weekday Layers
    int day_layer_height = 30;
    int day_layer_y = time_layer_y + time_layer_height + (bounds.size.h - (time_layer_y + time_layer_height) - day_layer_height) / 8;
    for (int i = 0; i < 7; i++) {
        layer_set_frame(text_layer_get_layer(s_week_layers[i]), GRect(10 + i * 18, day_layer_y, 20, day_layer_height));
    }

    // Underline Layer
    layer_set_frame(s_underline_layer, bounds);

    // Redraw the underline
    layer_mark_dirty(s_underline_layer);
}

static void main_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_unobstructed_bounds(window_layer);

    // Set window background color to black
    window_set_background_color(window, settings.color_background);

    // Time Layer
    int time_layer_height = 50;
    int time_layer_y = (bounds.size.h - time_layer_height) / 2;
    s_time_layer = text_layer_create(GRect(0, time_layer_y, bounds.size.w - 10, time_layer_height)); // Vertically centered
    text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentRight);
    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_color(s_time_layer, settings.color_primary); // Change text color to white
    layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

    // Date Layer
    int date_layer_height = 30;
    int date_layer_y = (time_layer_y - date_layer_height) / 1;
    s_date_layer = text_layer_create(GRect(0, date_layer_y, bounds.size.w - 10, date_layer_height)); // Centered between top and time layer
    text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text_alignment(s_date_layer, GTextAlignmentRight);
    text_layer_set_background_color(s_date_layer, GColorClear);
    text_layer_set_text_color(s_date_layer, settings.color_secondary); // Change text color to white
    layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

    // Weekday Layers
    int day_layer_height = 30;
    int day_layer_y = time_layer_y + time_layer_height + (bounds.size.h - (time_layer_y + time_layer_height) - day_layer_height) / 8;
    for (int i = 0; i < 7; i++) {
        s_week_layers[i] = text_layer_create(GRect(10 + i * 18, day_layer_y, 20, day_layer_height)); // Centered between bottom and time layer
        text_layer_set_font(s_week_layers[i], fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
        text_layer_set_text_alignment(s_week_layers[i], GTextAlignmentCenter);
        text_layer_set_background_color(s_week_layers[i], GColorClear);
        text_layer_set_text_color(s_week_layers[i], settings.color_secondary); // Change text color to white
        text_layer_set_text(s_week_layers[i], weekdays[i]);
        layer_add_child(window_layer, text_layer_get_layer(s_week_layers[i]));
    }

    // Underline Layer
    s_underline_layer = layer_create(bounds);
    layer_set_update_proc(s_underline_layer, underline_layer_update_proc);
    layer_add_child(window_layer, s_underline_layer);

    update_time();
}

static void main_window_unload(Window *window) {
    text_layer_destroy(s_time_layer);
    text_layer_destroy(s_date_layer);
    for (int i = 0; i < 7; i++) {
        text_layer_destroy(s_week_layers[i]);
    }
    layer_destroy(s_underline_layer);
}

static void init() {
    prv_load_settings();

    // Listen for AppMessages
    app_message_register_inbox_received(prv_inbox_received_handler);
    app_message_open(128, 128);
    
    s_main_window = window_create();
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload
    });

    window_stack_push(s_main_window, true);
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

    // Subscribe to unobstructed area events
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