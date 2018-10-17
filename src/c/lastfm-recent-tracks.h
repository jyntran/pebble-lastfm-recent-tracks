#pragma once

#define LIMIT 3

#define DATA_SIZE 6144
#define BUFFER_SIZE 2048
#define HEADER_BUFFER_SIZE 8

#define TRACK_POS_X 0
#define TRACK_POS_Y 40
#define TRACK_SIZE_H 32

#define ARTIST_POS_X 0
#define ARTIST_POS_Y 80
#define ARTIST_SIZE_H 32

#define TIMESTAMP_DATA_SIZE 72
#define TIMESTAMP_BUFFER_SIZE 24
#define TIMESTAMP_POS_X 0
#define TIMESTAMP_POS_Y 112
#define TIMESTAMP_SIZE_H 24

#define HEADER_SIZE_H 30

#define CHALK_HEADER_SIZE_H 48

#define TOTAL_POS_X 0
#define TOTAL_POS_Y 4
#define TOTAL_SIZE_H 18

#define CHALK_TOTAL_POS_Y 24

#define USERNAME_POS_X 0
#define USERNAME_POS_Y 4
#define USERNAME_SIZE_H 18

#define MARGIN_X 6
#define MARGIN_Y 6

#define CHALK_MARGIN_X 18
#define CHALK_MARGIN_Y 18

void prv_window_update();