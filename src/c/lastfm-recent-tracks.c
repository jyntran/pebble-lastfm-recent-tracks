#include <pebble.h>
#include "lastfm-recent-tracks.h"

static Window *s_window;
static Layer *track_layer, *header_layer;
static TextLayer *s_track_layer, *s_artist_layer, *s_timestamp_layer, *s_total_layer;

static char track_data[4096];
static char artist_data[4096];
static char timestamp_data[4096];

static int current = 0;

static char* tracks[LIMIT];
static char* artists[LIMIT];
static char* timestamps[LIMIT];

static bool isCurrentLast() {
  return current == LIMIT-1;
}

static void prv_window_update() {
  layer_mark_dirty(track_layer);
  layer_mark_dirty(header_layer);
}

static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
  char* current_track = track_data;
  char* current_artist = artist_data;
  char* current_timestamp = timestamp_data;

  for (int i=0; i<LIMIT; i++) {
    Tuple *track_tuple = dict_find(iter, 100 * i);
    if (track_tuple) {
      tracks[i] = current_track;
      strcat(track_data, track_tuple->value->cstring);
      current_track = &track_data[strlen(track_data)];
      APP_LOG(APP_LOG_LEVEL_DEBUG, tracks[i]);
    }

    Tuple *artist_tuple = dict_find(iter, 100 * i + 1);
    if (artist_tuple) {
      artists[i] = current_artist;
      strcat(artist_data, artist_tuple->value->cstring);
      current_artist = &artist_data[strlen(artist_data)];
      APP_LOG(APP_LOG_LEVEL_DEBUG, artists[i]);
    }

    Tuple *timestamp_tuple = dict_find(iter, 100 * i + 2);
    if (timestamp_tuple) {
      timestamps[i] = current_timestamp;
      strcat(timestamp_data, timestamp_tuple->value->cstring);
      current_timestamp = &timestamp_data[strlen(timestamp_data)];
      APP_LOG(APP_LOG_LEVEL_DEBUG, timestamps[i]);
    }
  }

  APP_LOG(APP_LOG_LEVEL_DEBUG, track_data);
  APP_LOG(APP_LOG_LEVEL_DEBUG, artist_data);
  APP_LOG(APP_LOG_LEVEL_DEBUG, timestamp_data);

  prv_window_update();
}

void prv_load_settings() {
  app_message_register_inbox_received(prv_inbox_received_handler);
  app_message_open(256, 256);
}

static void prv_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  prv_window_update();
}

static void prv_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (current != 0) {
    current--;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "current track index: %d", current);
    prv_window_update();
  }
}

static void prv_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (!isCurrentLast()) {
    current++;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "current track index: %d", current);
    prv_window_update();
  }
}

static void prv_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, prv_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, prv_down_click_handler);
}

static void track_update_proc(Layer *layer, GContext *ctx) {
  if (tracks[current]) {
    static char track_buffer[2048];
    static unsigned int track_length;
    if (!isCurrentLast()) {
      track_length = strlen(tracks[current]) - strlen(tracks[current+1]);
    } else {
      track_length = strlen(tracks[current]);
    }
    snprintf(track_buffer, sizeof(track_buffer), "%.*s", track_length, tracks[current]);
    APP_LOG(APP_LOG_LEVEL_DEBUG, track_buffer);
    text_layer_set_text(s_track_layer, track_buffer);
  }

  if (artists[current]) {
    static char artist_buffer[2048];
    static unsigned int artist_length;
    if (!isCurrentLast()) {
      artist_length = strlen(artists[current]) - strlen(artists[current+1]);
    } else {
      artist_length = strlen(artists[current]);
    }
    snprintf(artist_buffer, sizeof(artist_buffer), "%.*s", artist_length, artists[current]);
    APP_LOG(APP_LOG_LEVEL_DEBUG, artist_buffer);
    text_layer_set_text(s_artist_layer, artist_buffer);
  }

  if (timestamps[current]) {
    static char timestamp_buffer[2048];
    static unsigned int timestamp_length;
    if (!isCurrentLast()) {
      timestamp_length = strlen(timestamps[current]) - strlen(timestamps[current+1]);
    } else {
      timestamp_length = strlen(timestamps[current]);
    }
    snprintf(timestamp_buffer, sizeof(timestamp_buffer), "%.*s", timestamp_length, timestamps[current]);
    APP_LOG(APP_LOG_LEVEL_DEBUG, timestamp_buffer);
    text_layer_set_text(s_timestamp_layer, timestamp_buffer);
  }
}

static void header_update_proc(Layer *layer, GContext *ctx) {
  static char total_buffer[8];
  snprintf(total_buffer, sizeof(total_buffer), "%d/%d", current+1, LIMIT);
  APP_LOG(APP_LOG_LEVEL_DEBUG, total_buffer);
  text_layer_set_text(s_total_layer, total_buffer);
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  track_layer = layer_create(bounds);
  header_layer = layer_create(bounds);

  s_track_layer = text_layer_create(GRect(0, 40, bounds.size.w, 20));
  text_layer_set_text_alignment(s_track_layer, GTextAlignmentCenter);
  layer_add_child(track_layer, text_layer_get_layer(s_track_layer));

  s_artist_layer = text_layer_create(GRect(0, 60, bounds.size.w, 20));
  text_layer_set_text_alignment(s_artist_layer, GTextAlignmentCenter);
  layer_add_child(track_layer, text_layer_get_layer(s_artist_layer));

  s_timestamp_layer = text_layer_create(GRect(0, 80, bounds.size.w, 20));
  text_layer_set_text_alignment(s_timestamp_layer, GTextAlignmentCenter);
  layer_add_child(track_layer, text_layer_get_layer(s_timestamp_layer));

  s_total_layer = text_layer_create(GRect(0, 20, bounds.size.w, 20));
  text_layer_set_text_alignment(s_total_layer, GTextAlignmentCenter);
  layer_add_child(header_layer, text_layer_get_layer(s_total_layer));

  layer_add_child(window_layer, header_layer);
  layer_add_child(window_layer, track_layer);

  text_layer_set_text(s_track_layer, "LOADING");

  layer_set_update_proc(track_layer, track_update_proc);
  layer_set_update_proc(header_layer, header_update_proc);

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
  layer_destroy(header_layer);
  window_destroy(s_window);
}

int main(void) {
  prv_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

  app_event_loop();
  prv_deinit();
}