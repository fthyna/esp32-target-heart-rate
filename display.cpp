#include "display.h"
#include "config.h"
#include "helper.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

TaskHandle_t refreshDisplayTaskHandle;

uint32_t blinkTimer = 0;
uint32_t uploadFadeTimer = 0;
uint8_t isBlinking = 0;
uint8_t uploadIsFading = 0;

void drawUpwardArrow(uint8_t x, uint8_t y) {
	display.drawPixel(x + 2, y + 1, SSD1306_WHITE); // (2,1)
	display.drawPixel(x + 4, y + 1, SSD1306_WHITE); // (4,1)
	display.drawPixel(x + 1, y + 2, SSD1306_WHITE); // (1,2)
	display.drawPixel(x + 5, y + 2, SSD1306_WHITE); // (5,2)
	display.drawPixel(x + 0, y + 3, SSD1306_WHITE); // (5,2)
	display.drawPixel(x + 6, y + 3, SSD1306_WHITE); // (5,2)
	display.drawPixel(x + 3, y + 0, SSD1306_WHITE); // (3,0)
	display.drawPixel(x + 3, y + 1, SSD1306_WHITE); // (3,1)
	display.drawPixel(x + 3, y + 2, SSD1306_WHITE); // (3,2)
	display.drawPixel(x + 3, y + 3, SSD1306_WHITE); // (3,3)
	display.drawPixel(x + 3, y + 4, SSD1306_WHITE); // (3,4)
	display.drawPixel(x + 3, y + 5, SSD1306_WHITE); // (3,5)
	display.drawPixel(x + 3, y + 6, SSD1306_WHITE); // (3,6)
}

void drawUpload() {
	drawUpwardArrow(XPOS_UPLOADING_0, YPOS_UPLOADING_0);
}

void drawCheckmark(uint8_t x, uint8_t y) {
	display.drawPixel(x + 0, y + 3, SSD1306_WHITE); // (0,3)
	display.drawPixel(x + 1, y + 4, SSD1306_WHITE); // (1,4)
	display.drawPixel(x + 2, y + 5, SSD1306_WHITE); // (2,5)
	display.drawPixel(x + 3, y + 6, SSD1306_WHITE); // (3,6)
	display.drawPixel(x + 4, y + 4, SSD1306_WHITE); // (4,4)
	display.drawPixel(x + 4, y + 5, SSD1306_WHITE); // (4,5)
	display.drawPixel(x + 5, y + 2, SSD1306_WHITE); // (5,2)
	display.drawPixel(x + 5, y + 3, SSD1306_WHITE); // (5,3)
	display.drawPixel(x + 6, y + 0, SSD1306_WHITE); // (6,0)
	display.drawPixel(x + 6, y + 1, SSD1306_WHITE); // (6,1)
}

void drawX(uint8_t x, uint8_t y) {
	display.drawPixel(x + 0, y + 0, SSD1306_WHITE); // Top-left corner
	display.drawPixel(x + 6, y + 0, SSD1306_WHITE); // Top-right corner
	display.drawPixel(x + 1, y + 1, SSD1306_WHITE);
	display.drawPixel(x + 5, y + 1, SSD1306_WHITE);
	display.drawPixel(x + 2, y + 2, SSD1306_WHITE);
	display.drawPixel(x + 4, y + 2, SSD1306_WHITE);
	display.drawPixel(x + 3, y + 3, SSD1306_WHITE); // Center
	display.drawPixel(x + 2, y + 4, SSD1306_WHITE);
	display.drawPixel(x + 4, y + 4, SSD1306_WHITE);
	display.drawPixel(x + 1, y + 5, SSD1306_WHITE);
	display.drawPixel(x + 5, y + 5, SSD1306_WHITE);
	display.drawPixel(x + 0, y + 6, SSD1306_WHITE); // Bottom-left corner
	display.drawPixel(x + 6, y + 6, SSD1306_WHITE); // Bottom-right corner
}

void drawUploadDone() {
	drawCheckmark(XPOS_UPLOADING_DONE_0, YPOS_UPLOADING_DONE_0);
	uploadIsFading = 1;
	putTimer(uploadFadeTimer);
}

void drawUploadFail() {
	drawX(XPOS_UPLOADING_DONE_0, YPOS_UPLOADING_DONE_0);
	uploadIsFading = 1;
	putTimer(uploadFadeTimer);
}

void clearUpload() {
	display.fillRect(XPOS_UPLOADING_0, YPOS_UPLOADING_0, WIDTH_UPLOADING, HEIGHT_UPLOADING, BLACK);
	uploadIsFading = 0;
}

void blinkDisplay(uint8_t state) {
	if (state) {
		putTimer(blinkTimer);
		display.fillRect(XPOS_BLINK_0, YPOS_BLINK_0, SIZE_BLINK, SIZE_BLINK, WHITE);
		isBlinking = 1;
	}
	else {
		display.fillRect(XPOS_BLINK_0, YPOS_BLINK_0, SIZE_BLINK, SIZE_BLINK, BLACK);
		isBlinking = 0;
	}
}

void refreshDisplayTask(void *param) {
	TickType_t xLastWakeTime = xTaskGetTickCount();
	while (true) {
		if (isBlinking && getTimeElapsed(blinkTimer) > SCREEN_BLINK_INTERVAL_MS) blinkDisplay(0);
		if (uploadIsFading && getTimeElapsed(uploadFadeTimer) > UPLOAD_FADE_INTERVAL_MS) clearUpload();

		updateDisplay();
		vTaskDelayUntil(&xLastWakeTime, (REFRESH_INTERVAL_MS));
	}
}

void plotIR(uint32_t *buff, uint8_t idx, uint32_t min_val = IR_PLOT_MIN_VAL, uint32_t max_val = IR_PLOT_MAX_VAL) {
	min_val -= IR_PLOT_PADDING_VAL; // add padding
	max_val += IR_PLOT_PADDING_VAL;
	if (max_val-min_val > IR_PLOT_RANGE_VAL) // if too far, pin min_val to max_val
		min_val = IR_PLOT_RANGE_VAL < max_val ? max_val - IR_PLOT_RANGE_VAL : 0;
	uint32_t val_range = max_val - min_val;
	display.fillRect(idx, IR_PLOT_TOP_Y, 1, IR_PLOT_RANGE_Y, BLACK);
	uint32_t val = buff[idx];
	if (val >= min_val) {
		if (val >= max_val) val = max_val;
		val -= min_val;
		uint16_t height_y = (val) * (IR_PLOT_RANGE_Y) / (val_range);
		uint16_t plot_y = SCREEN_HEIGHT - height_y;
		display.fillRect(idx, plot_y, 1, height_y, WHITE);
	}
}

void displayText(const char *text) {
	display.setCursor(XPOS_TEXT_0, YPOS_TEXT_0);
	display.fillRect(
		XPOS_TEXT_0,
		YPOS_TEXT_0,
		XPOS_VALID_C-RADIUS_VALID,
		TEXT_SIZE_BIG*TEXT_SIZE_SCALE_PX,
		BLACK);
	display.setTextSize(TEXT_SIZE_BIG);
	display.print(text);
}
void displayText(String text) {
	display.setCursor(XPOS_TEXT_0, YPOS_TEXT_0);
	display.fillRect(
		XPOS_TEXT_0,
		YPOS_TEXT_0,
		XPOS_VALID_C-RADIUS_VALID,
		TEXT_SIZE_BIG*TEXT_SIZE_SCALE_PX,
		BLACK);
	display.setTextSize(TEXT_SIZE_BIG);
	display.print(text);
}

void displayHR(int hr) {
	display.setCursor(XPOS_HR_0, YPOS_HR_0);
	display.fillRect(
		XPOS_HR_0,
		YPOS_HR_0,
		XPOS_BPM_0-XPOS_HR_0,
		TEXT_SIZE_HR*TEXT_SIZE_SCALE_PX,
		BLACK);
	display.setTextSize(TEXT_SIZE_HR);
	display.print(hr);
}
void displayHR(const char *hr) {
	display.setCursor(XPOS_HR_0, YPOS_HR_0);
	display.fillRect(XPOS_HR_0, YPOS_HR_0, XPOS_BPM_0-XPOS_HR_0, TEXT_SIZE_HR*TEXT_SIZE_SCALE_PX, BLACK);
	display.setTextSize(TEXT_SIZE_HR);
	display.print(hr);
}
void displayBPM() {
	display.setCursor(XPOS_BPM_0, YPOS_BPM_0);
	display.setTextSize(TEXT_SIZE_BIG);
	display.print("BPM");
}

void displayValidOutline() {
	display.fillCircle(XPOS_VALID_C, YPOS_VALID_C, RADIUS_VALID, WHITE);
}

void displayValid(int8_t valid) {
	if (valid) display.fillCircle(XPOS_VALID_C, YPOS_VALID_C, RADIUS_VALID-1, WHITE);
	else display.fillCircle(XPOS_VALID_C, YPOS_VALID_C, RADIUS_VALID-1, BLACK);
}

void updateDisplay() {
	display.display();
}

void initializeDisplay()
{
	while (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
		#if ENABLE_DEBUG
		Serial.println("SSD1306 allocation failed");
		#endif
		delay(DEVICE_INIT_WAIT);
	}
	display.clearDisplay();
	display.setTextSize(2);
	display.setTextColor(WHITE);
	display.setCursor(0, 0);
	displayHR("--");
	displayBPM();
	displayValidOutline();
	displayValid(0);
	display.display();

	xTaskCreatePinnedToCore(
		refreshDisplayTask,
		"Refresh Display",
		2000,
		NULL,
		3,
		&refreshDisplayTaskHandle,
		1
	);
	#if ENABLE_DEBUG
	printDebug("Initialized refresh display task");
	#endif
}