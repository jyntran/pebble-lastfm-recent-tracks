#pragma once

#include <pebble.h>
#define SETTINGS_KEY 1

typedef struct ClaySettings {
  char LastfmUsername[32];
  bool ShowUsername;
  bool ShowTimestamp;
  bool ShowTrackTotal;
} ClaySettings;

struct ClaySettings settings;

void prv_load_settings();
void prv_inbox_received_handler();