#include "sensor.h"
#include "helper.h"
#include "display.h"
#include "algorithm_by_RF.h"

MAX30105 particleSensor;					// MAX30102 pulse oximetry sensor

uint32_t batchTimer = 0;
uint32_t sampleTimer = 0;

// Read values into buffers and increment index
void readIRToBuffer(uint32_t *irbuff, uint32_t *redbuff, uint8_t idx) {
	irbuff[idx] = particleSensor.getIR();
	redbuff[idx] = particleSensor.getRed();
	putTimer(sampleTimer);
}

void updateMinMaxBufferValue(uint32_t &mn, uint32_t &mx, uint32_t *buff, uint8_t idx) {
	static uint32_t cur_max = 0;
	static uint32_t cur_min = UINT32_MAX;
	static uint32_t val;
	val = buff[idx];
	if (idx != 0) {
		if (val > cur_max) cur_max = val;
		if (val < cur_min) cur_min = val;
	}
	else {
		mx = cur_max;
		mn = cur_min;
		cur_max = 0;
		cur_min = UINT32_MAX;
	}
}

void initializeSensor() {
	while (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
		Serial.println("MAX30105 was not found. Check wiring!");
		delay(2000);
	}
	particleSensor.setup();
	particleSensor.setPulseAmplitudeRed(0x24);		// Red LED 
	particleSensor.setPulseAmplitudeIR(0x24);		// IR LED
	particleSensor.setPulseAmplitudeGreen(0x00);	// Turn off Green if not needed
}

void showBatchTimer() {
	showTimer(batchTimer);
}

void showSampleTimer() {
	showTimer(sampleTimer);
}