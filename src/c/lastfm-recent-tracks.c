#include <pebble.h>

static Window *s_window;
static Layer *track_layer;
static TextLayer *s_track_layer, *s_artist_layer, *s_timestamp_layer;
static char *track, *artist, *timestamp;
static char tracks[2048];
static char artists[1024];
static char timestamps[1024];

static char* tracksArr[3];

static int LIMIT = 3;
static int current_track = 0;

static void prv_window_update() {
  layer_mark_dirty(track_layer);
}

static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {

  char* currTrack = tracks;

  for (int i=0; i<LIMIT; i++) {
    Tuple *track_tuple = dict_find(iter, 100 * i);
    if (track_tuple) {
      //APP_LOG(APP_LOG_LEVEL_DEBUG, track_tuple->value->cstring);
      tracksArr[i] = currTrack;
      strcat(tracks, track_tuple->value->cstring);
      currTrack = &tracks[strlen(tracks)];
      APP_LOG(APP_LOG_LEVEL_DEBUG, tracks);
      APP_LOG(APP_LOG_LEVEL_DEBUG, tracksArr[i]);
    }

    // Tuple *artist_tuple = dict_find(iter, 100 * i + 1);
    // if (artist_tuple) {
    //   artist = artist_tuple->value->cstring;
    //   //APP_LOG(APP_LOG_LEVEL_DEBUG, artist);
    // }

    // Tuple *timestamp_tuple = dict_find(iter, 100 * i + 2);
    // if (timestamp_tuple) {
    //   timestamp = timestamp_tuple->value->cstring;
    //   //APP_LOG(APP_LOG_LEVEL_DEBUG, timestamp);
    // }
  }

  char track_buffer[1024];
  static unsigned int trackLen;
  APP_LOG(APP_LOG_LEVEL_INFO, "%d - %d", strlen(tracksArr[0]), strlen(tracksArr[1]));
  trackLen = strlen(tracksArr[0]) - strlen(tracksArr[1]);
  snprintf(track_buffer, sizeof(track_buffer), "%.*s", trackLen, tracksArr[0]);
  track = track_buffer;
  APP_LOG(APP_LOG_LEVEL_INFO, track);

  prv_window_update();
}

void prv_load_settings() {
  app_message_register_inbox_received(prv_inbox_received_handler);
  app_message_open(256, 256);
}

static void prv_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_track_layer, "Select");
}

static void prv_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (current_track != 0) {
    current_track--;
  }
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "DOWN %d", current_track);
  // char track_buffer[1024];
  // static unsigned int trackLen;
  // if (current_track != LIMIT-1) {
  //   trackLen = strlen(tracksArr[current_track]) - strlen(tracksArr[current_track+1]);
  //   snprintf(track_buffer, sizeof(track_buffer), "%.*s", trackLen, tracksArr[current_track]);
  // } else {
  //   //snprintf(track_buffer, sizeof(track_buffer), "%s", tracksArr[current_track]);
  // }
  // track = track_buffer;
  // APP_LOG(APP_LOG_LEVEL_INFO, track);
  prv_window_update();
}

static void prv_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (current_track != LIMIT-1) {
    current_track++;
  }
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "UP %d", current_track);
  // char track_buffer[1024];
  // static unsigned int trackLen;
  // if (current_track != LIMIT-1) {
  //   trackLen = strlen(tracksArr[current_track]) - strlen(tracksArr[current_track+1]);
  //   snprintf(track_buffer, sizeof(track_buffer), "%.*s", trackLen, tracksArr[current_track]);
  // } else {
  //   //snprintf(track_buffer, sizeof(track_buffer), "%s", tracksArr[current_track]);
  // }
  // track = track_buffer;
  // APP_LOG(APP_LOG_LEVEL_INFO, track);
  prv_window_update();
}

static void prv_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, prv_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, prv_down_click_handler);
}

static void track_update_proc(Layer *layer, GContext *ctx) {
  if (track) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, track);
    text_layer_set_text(s_track_layer, track);
  }

  // if (artist) {
  //   //static char artist_buffer[1024];
  //   //snprintf(artist_buffer, sizeof(artist_buffer), "%s", artist);
  //   //APP_LOG(APP_LOG_LEVEL_DEBUG, artist_buffer);
  //   //text_layer_set_text(s_artist_layer, artist_buffer);
  // }

  // if (timestamp) {
  //   //static char timestamp_buffer[1024];
  //   //snprintf(timestamp_buffer, sizeof(timestamp_buffer), "%s", timestamp);
  //   //APP_LOG(APP_LOG_LEVEL_DEBUG, timestamp_buffer);
  //   //text_layer_set_text(s_timestamp_layer, timestamp_buffer);
  // }
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  track_layer = layer_create(bounds);

  s_track_layer = text_layer_create(GRect(0, 0, bounds.size.w, 20));
  text_layer_set_text_alignment(s_track_layer, GTextAlignmentCenter);
  layer_add_child(track_layer, text_layer_get_layer(s_track_layer));

  // s_artist_layer = text_layer_create(GRect(0, 20, bounds.size.w, 20));
  // text_layer_set_text_alignment(s_artist_layer, GTextAlignmentCenter);
  // layer_add_child(track_layer, text_layer_get_layer(s_artist_layer));

  // s_timestamp_layer = text_layer_create(GRect(0, 40, bounds.size.w, 20));
  // text_layer_set_text_alignment(s_timestamp_layer, GTextAlignmentCenter);
  // layer_add_child(track_layer, text_layer_get_layer(s_timestamp_layer));

  layer_add_child(window_layer, track_layer);

  layer_set_update_proc(track_layer, track_update_proc);

  prv_window_update();
}

static void prv_window_unload(Window *window) {
  text_layer_destroy(s_track_layer);
  text_layer_destroy(s_artist_layer);
  text_layer_destroy(s_timestamp_layer);
}

static void prv_init(void) {
  prv_load_settings();

  s_window = window_create();
  window_set_click_config_provider(s_window, prv_click_config_provider);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  const bool animated = true;
  window_stack_push(s_window, animated);
}

static void prv_deinit(void) {
  layer_destroy(track_layer);
  window_destroy(s_window);
}

int main(void) {
  prv_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

  app_event_loop();
  prv_deinit();
}