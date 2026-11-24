#pragma once

#include <pebble.h>

#define SETTINGS_KEY 8642

// Persisted settings layout (packed, stored under SETTINGS_KEY).
// Do not reorder or resize fields without migrating saved data.
typedef struct ClaySettings {
  GColor color_background;
  GColor color_time;
  GColor color_date;
  GColor color_week;
  GColor color_highlight;
} __attribute__((__packed__)) ClaySettings;
