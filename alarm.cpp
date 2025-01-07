#include "alarm.h"
#include "config.h"
#include "helper.h"

uint32_t buzzerTimer = 0;
uint32_t buzzerDuration = 0;
uint8_t buzzerIsRinging = 0;

TaskHandle_t refreshBuzzerTaskHandle;

uint32_t getBuzzerDuration(Pattern pattern) {
	switch (pattern) {
		case LongBeep:
			return 490;
			break;
		case ShortBeep:
			return 190;
			break;
		case ExtraShortBeep:
			return 90;
		default:
			return 0;
			break;
	}
}

void ringBuzzer(Pattern pattern, uint16_t freq = ALERT_FREQUENCY) {
	ledcWriteTone(TONE_CHANNEL, freq);
	putTimer(buzzerTimer);
	buzzerIsRinging = 1;
	buzzerDuration = getBuzzerDuration(pattern);
}

void refreshBuzzerTask(void *param) {
	TickType_t xLastWakeTime = xTaskGetTickCount();
	while (true) {
		if (buzzerIsRinging && getTimeElapsed(buzzerTimer) > buzzerDuration) {
			ledcWriteTone(TONE_CHANNEL, 0);
			buzzerIsRinging = 0;
		}
		vTaskDelay(pdMS_TO_TICKS(BUZZER_CHECK_INTERVAL_MS));
	}
}

void initializeBuzzer() {
	ledcSetup(TONE_CHANNEL, 0, TONE_RESOLUTION);
	ledcAttachPin(PIN_BUZZER, TONE_CHANNEL);

	xTaskCreatePinnedToCore(
		refreshBuzzerTask,
		"Refresh Display",
		2000,
		NULL,
		3,
		&refreshBuzzerTaskHandle,
		1
	);
	// check tone
	ringBuzzer(ShortBeep);
}