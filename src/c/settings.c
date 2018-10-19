#include "settings.h"

static void prv_default_settings() {
  settings.ShowUsername = true;
  settings.ShowTimestamp = true;
  settings.ShowTrackTotal = true;
}

void prv_load_settings() {
  prv_default_settings();
  
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));

  app_message_register_inbox_received(prv_inbox_received_handler);
  app_message_open(512, 512);
}
