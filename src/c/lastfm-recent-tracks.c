#include <pebble.h>
#include "lastfm-recent-tracks.h"
#include "settings.h"

static Window     *s_window;
static Layer      *header_layer, *body_layer;
static TextLayer  *s_track_layer,
                  *s_artist_layer,
                  *s_timestamp_layer,
                  *s_total_layer,
                  *s_username_layer;

static char track_data[DATA_SIZE];
static char artist_data[DATA_SIZE];
static char timestamp_data[TIMESTAMP_DATA_SIZE];

static char* tracks[LIMIT];
static char* artists[LIMIT];
static char* timestamps[LIMIT];

static int current = 0;


static bool isCurrentLast() {
  return current == LIMIT-1;
}

static bool isEmpty(char arr[]) {
  return strlen(arr) == 0;
}

void prv_window_update() {
  layer_mark_dirty(header_layer);
  layer_mark_dirty(body_layer);
}

void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *username_tuple = dict_find(iter, MESSAGE_KEY_LastfmUsername);
  if (username_tuple) {
    strncpy(settings.LastfmUsername, username_tuple->value->cstring, sizeof(settings.LastfmUsername) - 1);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "username: %s", settings.LastfmUsername);
  }

  Tuple *show_username_tuple = dict_find(iter, MESSAGE_KEY_ShowUsername);
  if (show_username_tuple) {
    settings.ShowUsername = show_username_tuple->value->int32 == 1;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "showUsername: %s", settings.ShowUsername ? "t" : "f");
  }

  Tuple *show_timestamp_tuple = dict_find(iter, MESSAGE_KEY_ShowTimestamp);
  if (show_timestamp_tuple) {
    settings.ShowTimestamp = show_timestamp_tuple->value->int32 == 1;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "showTimestamp: %s", settings.ShowTimestamp ? "t" : "f");
  }

  Tuple *show_track_total_tuple = dict_find(iter, MESSAGE_KEY_ShowTrackTotal);
  if (show_track_total_tuple) {
    settings.ShowTrackTotal = show_track_total_tuple->value->int32 == 1;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "showTrackTotal: %s", settings.ShowTrackTotal ? "t" : "f");
  }

  char* current_track = track_data;
  char* current_artist = artist_data;
  char* current_timestamp = timestamp_data;

  if (isEmpty(track_data) && isEmpty(artist_data) && isEmpty(timestamp_data)) {
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
  }

  // APP_LOG(APP_LOG_LEVEL_DEBUG, track_data);
  // APP_LOG(APP_LOG_LEVEL_DEBUG, artist_data);
  // APP_LOG(APP_LOG_LEVEL_DEBUG, timestamp_data);

  prv_window_update();
}

static void prv_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  prv_window_update();
}

static void prv_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (current != 0) {
    current--;
    prv_window_update();
  }
}

static void prv_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (!isCurrentLast()) {
    current++;
    prv_window_update();
  }
}

static void prv_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, prv_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, prv_down_click_handler);
}

static void body_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(window_get_root_layer(s_window));
  
#if defined(PBL_ROUND)
  GEdgeInsets body_insets = GEdgeInsets(CHALK_MARGIN_Y, CHALK_MARGIN_X);
#else
  GEdgeInsets body_insets = GEdgeInsets(MARGIN_Y, MARGIN_X);
#endif
  GRect body_bounds = grect_inset(GRect(bounds.origin.x, bounds.origin.y, bounds.size.w, bounds.size.h), body_insets);

  GSize track_size = GSize(TRACK_SIZE_H, TRACK_SIZE_H);
  GSize artist_size = GSize(ARTIST_SIZE_H, ARTIST_SIZE_H);
  GSize timestamp_size = GSize(TIMESTAMP_SIZE_H, TIMESTAMP_SIZE_H);

  if (tracks[current]) {
    static char track_buffer[BUFFER_SIZE];
    static unsigned int track_length;
    if (!isCurrentLast()) {
      track_length = strlen(tracks[current]) - strlen(tracks[current+1]);
    } else {
      track_length = strlen(tracks[current]);
    }
    snprintf(track_buffer, sizeof(track_buffer), "%.*s", track_length, tracks[current]);
    APP_LOG(APP_LOG_LEVEL_DEBUG, track_buffer);
    text_layer_set_text(s_track_layer, track_buffer);

    track_size = text_layer_get_content_size(s_track_layer);
    layer_set_frame(text_layer_get_layer(s_track_layer),
      GRect(
        TRACK_POS_X,
        TRACK_POS_Y,
        body_bounds.size.w,
        track_size.h + MARGIN_Y
      )
    );
  }

  if (artists[current]) {
    static char artist_buffer[BUFFER_SIZE];
    static unsigned int artist_length;
    if (!isCurrentLast()) {
      artist_length = strlen(artists[current]) - strlen(artists[current+1]);
    } else {
      artist_length = strlen(artists[current]);
    }
    snprintf(artist_buffer, sizeof(artist_buffer), "%.*s", artist_length, artists[current]);
    APP_LOG(APP_LOG_LEVEL_DEBUG, artist_buffer);
    text_layer_set_text(s_artist_layer, artist_buffer);

    artist_size = text_layer_get_content_size(s_artist_layer);
    layer_set_frame(text_layer_get_layer(s_artist_layer),
      GRect(
        ARTIST_POS_X,
        TRACK_POS_Y + track_size.h + MARGIN_Y,
        body_bounds.size.w,
        artist_size.h + MARGIN_Y
      )
    );
  }

  if (!settings.ShowTimestamp) {
    layer_set_hidden(text_layer_get_layer(s_timestamp_layer), true);
  } else {
    layer_set_hidden(text_layer_get_layer(s_timestamp_layer), false);
    if (timestamps[current]) {
      static char timestamp_buffer[TIMESTAMP_BUFFER_SIZE];
      static unsigned int timestamp_length;
      if (!isCurrentLast()) {
        timestamp_length = strlen(timestamps[current]) - strlen(timestamps[current+1]);
      } else {
        timestamp_length = strlen(timestamps[current]);
      }
      snprintf(timestamp_buffer, sizeof(timestamp_buffer), "%.*s", timestamp_length, timestamps[current]);
      APP_LOG(APP_LOG_LEVEL_DEBUG, timestamp_buffer);
      text_layer_set_text(s_timestamp_layer, timestamp_buffer);

      timestamp_size = text_layer_get_content_size(s_timestamp_layer);
      layer_set_frame(text_layer_get_layer(s_timestamp_layer),
        GRect(
          TIMESTAMP_POS_X,
          TRACK_POS_Y + track_size.h + MARGIN_Y + artist_size.h + MARGIN_Y,
          body_bounds.size.w,
          timestamp_size.h + MARGIN_Y
        )
      );
    }
  }
}

static void header_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(window_get_root_layer(s_window));
  GRect header_bounds = GRect(0, 0, bounds.size.w, PBL_IF_ROUND_ELSE(CHALK_HEADER_SIZE_H, HEADER_SIZE_H));
  graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorDarkCandyAppleRed, GColorBlack));
  graphics_fill_rect(ctx, header_bounds, 0, 0);

  bool hasUsername = settings.LastfmUsername && strlen(settings.LastfmUsername) > 0;

  if (hasUsername) {
    text_layer_set_text(s_username_layer, settings.LastfmUsername);
    layer_set_hidden(text_layer_get_layer(s_username_layer), !settings.ShowUsername);
  } else {
    layer_set_hidden(text_layer_get_layer(s_username_layer), settings.ShowUsername);
  }

  if (!settings.ShowTrackTotal) {
    layer_set_hidden(text_layer_get_layer(s_total_layer), true);
  } else {
    layer_set_hidden(text_layer_get_layer(s_total_layer), false);
    static char total_buffer[HEADER_BUFFER_SIZE];
    snprintf(total_buffer, sizeof(total_buffer), "%d/%d", current+1, LIMIT);
    APP_LOG(APP_LOG_LEVEL_DEBUG, total_buffer);
    text_layer_set_text(s_total_layer, total_buffer);
  }
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  GRect header_bounds = GRect(bounds.origin.x, bounds.origin.y, bounds.size.w, PBL_IF_ROUND_ELSE(CHALK_HEADER_SIZE_H, HEADER_SIZE_H));
  header_layer = layer_create(header_bounds);
  
#if defined(PBL_ROUND)
  GEdgeInsets body_insets = GEdgeInsets(CHALK_MARGIN_Y, CHALK_MARGIN_X);
#else
  GEdgeInsets body_insets = GEdgeInsets(MARGIN_Y, MARGIN_X);
#endif
  GRect body_bounds = grect_inset(GRect(bounds.origin.x, bounds.origin.y, bounds.size.w, bounds.size.h), body_insets);
  body_layer = layer_create(body_bounds);

  s_username_layer = text_layer_create(GRect(PBL_IF_ROUND_ELSE(TOTAL_POS_X, TOTAL_POS_X + MARGIN_X), PBL_IF_ROUND_ELSE(CHALK_TOTAL_POS_Y, TOTAL_POS_Y), PBL_IF_ROUND_ELSE(bounds.size.w, 3*bounds.size.w/4 - MARGIN_X), TOTAL_SIZE_H));
  text_layer_set_text_alignment(s_username_layer, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft));
  text_layer_set_font(s_username_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_background_color(s_username_layer, GColorClear);
  text_layer_set_text_color(s_username_layer, GColorWhite);
  layer_add_child(header_layer, text_layer_get_layer(s_username_layer));

  s_total_layer = text_layer_create(GRect(PBL_IF_ROUND_ELSE(bounds.origin.x, 3*bounds.size.w/4), TOTAL_POS_Y, PBL_IF_ROUND_ELSE(bounds.size.w, bounds.size.w/4 - MARGIN_X), TOTAL_SIZE_H));
  text_layer_set_text_alignment(s_total_layer, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentRight));
  text_layer_set_font(s_total_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_background_color(s_total_layer, GColorClear);
  text_layer_set_text_color(s_total_layer, GColorWhite);
  layer_add_child(header_layer, text_layer_get_layer(s_total_layer));

  s_track_layer = text_layer_create(GRect(TRACK_POS_X, TRACK_POS_Y, body_bounds.size.w, TRACK_SIZE_H));
  text_layer_set_text_alignment(s_track_layer, GTextAlignmentCenter);
  text_layer_set_font(s_track_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_overflow_mode(s_track_layer, GTextOverflowModeWordWrap);
  text_layer_set_background_color(s_track_layer, GColorClear);
  layer_add_child(body_layer, text_layer_get_layer(s_track_layer));
  text_layer_set_text(s_track_layer, "Loading");

  s_artist_layer = text_layer_create(GRect(ARTIST_POS_X, ARTIST_POS_Y, body_bounds.size.w, ARTIST_SIZE_H));
  text_layer_set_text_alignment(s_artist_layer, GTextAlignmentCenter);
  text_layer_set_font(s_artist_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_overflow_mode(s_artist_layer, GTextOverflowModeWordWrap);
  text_layer_set_background_color(s_artist_layer, GColorClear);
  layer_add_child(body_layer, text_layer_get_layer(s_artist_layer));

  s_timestamp_layer = text_layer_create(GRect(TIMESTAMP_POS_X, TIMESTAMP_POS_Y, body_bounds.size.w, TIMESTAMP_SIZE_H));
  text_layer_set_text_alignment(s_timestamp_layer, GTextAlignmentCenter);
  text_layer_set_font(s_timestamp_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_background_color(s_timestamp_layer, GColorClear);
  layer_add_child(body_layer, text_layer_get_layer(s_timestamp_layer));

  layer_add_child(window_layer, header_layer);
  layer_add_child(window_layer, body_layer);

  layer_set_update_proc(header_layer, header_update_proc);
  layer_set_update_proc(body_layer, body_update_proc);

  prv_window_update();
}

static void prv_window_unload(Window *window) {
  text_layer_destroy(s_username_layer);
  text_layer_destroy(s_total_layer);
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
  layer_destroy(header_layer);
  layer_destroy(body_layer);
  window_destroy(s_window);
}

int main(void) {
  prv_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

  app_event_loop();
  prv_deinit();
}