#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include "algorithm_by_RF.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define TEXT_SIZE_HR 4
#define TEXT_SIZE_BIG 2
#define TEXT_SIZE_SMALL 1
#define TEXT_SIZE_SCALE_PX 8

#define XPOS_BLINK_0 0
#define YPOS_BLINK_0 16
#define SIZE_BLINK 4
#define XPOS_TEXT_0 0
#define YPOS_TEXT_0 0
#define XPOS_HR_0 16
#define YPOS_HR_0 16
#define XPOS_BPM_0 90
#define YPOS_BPM_0 32
#define XPOS_VALID_C 119
#define YPOS_VALID_C 8
#define RADIUS_VALID 6

#define XPOS_UPLOADING_0 94
#define YPOS_UPLOADING_0 55
#define XPOS_UPLOADING_DONE_0 (XPOS_UPLOADING_0+10)
#define YPOS_UPLOADING_DONE_0 YPOS_UPLOADING_0
#define WIDTH_UPLOADING 24
#define HEIGHT_UPLOADING 7

#define IR_PLOT_TOP_Y 48
#define IR_PLOT_BOT_Y 64
#define IR_PLOT_MAX_VAL	142000
#define IR_PLOT_RANGE_VAL 4000
#define IR_PLOT_PADDING_VAL 500

#define PLOT_INTERVAL_MS 50

#define IR_PLOT_MIN_VAL	(IR_PLOT_MAX_VAL-IR_PLOT_RANGE_VAL)
#define IR_PLOT_RANGE_Y (IR_PLOT_BOT_Y-IR_PLOT_TOP_Y)
#define IR_PLOT_MIN_X (SCREEN_WIDTH-BUFFER_SIZE)/2
#define IR_PLOT_MAX_X (SCREEN_WIDTH+BUFFER_SIZE)/2

void drawUpload();
void drawUploadDone();
void drawUploadFail();
void clearUpload();

void blinkDisplay(uint8_t state);
void refreshDisplayTask(void *param);

void plotIR(uint32_t *buff, uint8_t idx, uint32_t minplot, uint32_t maxplot);
void displayText(const char *text);
void displayText(String text);
void displayHR(int hr);
void displayHR(const char *hr);
void displayValidOutline();
void displayValid(int8_t valid);
void updateDisplay();
void initializeDisplay();

#endif