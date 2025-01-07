#ifndef ALARM_H
#define ALARM_H

#include <Arduino.h>

#define TONE_CHANNEL 0  // Channel to use (0–15 for ESP32)
#define TONE_RESOLUTION 8  // PWM resolution in bits (8 is sufficient for tone)
#define TONE_PIN 25  // Pin to output the tone
#define ALERT_FREQUENCY 2000
#define BUZZER_CHECK_INTERVAL_MS 100

enum Pattern {
	LongBeep,
	ShortBeep,
	ExtraShortBeep,
};

void refreshBuzzerTask(void *param);

void initializeBuzzer();
uint32_t getBuzzerDuration(Pattern pattern);
void ringBuzzer(Pattern pattern, uint16_t freq);

#endif