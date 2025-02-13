#pragma once
#include <pebble.h>

#define SETTINGS_KEY 8642

// This whole file taken from https://github.com/pebble-examples/clay-example/

// A structure containing our settings
typedef struct ClaySettings {
  
  GColor color_background;
  GColor color_primary;
  GColor color_secondary;

} __attribute__((__packed__)) ClaySettings;

static void prv_default_settings();
static void prv_load_settings();
static void prv_save_settings();
static void prv_update_display();
static void prv_inbox_received_handler(DictionaryIterator *iter, void *context);
static void main_window_load(Window *window);
static void main_window_unload(Window *window);
static void init(void);
static void deinit(void);