#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer, *s_date_layer;
static TextLayer *s_week_layers[7]; // One text layer per weekday
static Layer *s_underline_layer;
static int s_current_day_index = 0;

static const char *weekdays[] = {"S", "M", "T", "W", "T", "F", "S"};
static const int underline_positions[] = {10, 28, 46, 64, 82, 100, 118}; // x-coordinates for each day

static void underline_layer_update_proc(Layer *layer, GContext *ctx) {
    graphics_context_set_stroke_width(ctx, 3);
    graphics_context_set_stroke_color(ctx, GColorWhite); // Change underline color to white
    
    // Calculate the x position based on the current day index
    GRect day_bounds = layer_get_frame(text_layer_get_layer(s_week_layers[s_current_day_index]));
    int x_position = day_bounds.origin.x + (day_bounds.size.w / 2) - 9; // Center the underline within the text layer
    GRect underline_rect = GRect(x_position, 140, 18, 2); // Vertical position, Width, and Height of underline
    graphics_draw_rect(ctx, underline_rect);
}

static void update_time() {
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    static char time_buffer[6];
    static char date_buffer[12];

    strftime(time_buffer, sizeof(time_buffer), clock_is_24h_style() ? "%H:%M" : "%l:%M", tick_time);
    strftime(date_buffer, sizeof(date_buffer), "%B %d", tick_time);

    text_layer_set_text(s_time_layer, time_buffer);
    text_layer_set_text(s_date_layer, date_buffer);

    // Calculate current weekday index
    s_current_day_index = tick_time->tm_wday; // Sunday = 0, Saturday = 6
    
    // Trigger redraw of the underline
    layer_mark_dirty(s_underline_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time();
}

static void main_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // Set window background color to black
    window_set_background_color(window, GColorBlack);

    // Date Layer
    s_date_layer = text_layer_create(GRect(0, 30, bounds.size.w - 10, 30));
    text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text_alignment(s_date_layer, GTextAlignmentRight);
    text_layer_set_background_color(s_date_layer, GColorClear);
    text_layer_set_text_color(s_date_layer, GColorWhite); // Change text color to white
    layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

    // Time Layer
    int time_layer_height = 50;
    int time_layer_y = (bounds.size.h - time_layer_height) / 2;
    s_time_layer = text_layer_create(GRect(0, time_layer_y, bounds.size.w - 10, time_layer_height)); // Vertically centered
    text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentRight);
    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_color(s_time_layer, GColorWhite); // Change text color to white
    layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

    // Weekday Layers
    for (int i = 0; i < 7; i++) {
        s_week_layers[i] = text_layer_create(GRect(10 + i * 18, 110, 20, 30));
        text_layer_set_font(s_week_layers[i], fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
        text_layer_set_text_alignment(s_week_layers[i], GTextAlignmentCenter);
        text_layer_set_background_color(s_week_layers[i], GColorClear);
        text_layer_set_text_color(s_week_layers[i], GColorWhite); // Change text color to white
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
    s_main_window = window_create();
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload
    });

    window_stack_push(s_main_window, true);
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
    window_destroy(s_main_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
