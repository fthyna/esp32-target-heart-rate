#include <Arduino.h>
#include <HTTPClient.h>

#include "tasks.h"
#include "config.h"
#include "sensor.h"
#include "helper.h"
#include "iot.h"
#include "display.h"
#include "intensity.h"
#include "alarm.h"

#include "algorithm_by_rf.h"
#include "modified_RF_algorithm.h"

TaskHandle_t dataCollectionTaskHandle;
TaskHandle_t dataProcessingTaskHandle;
TaskHandle_t dataUploadTaskHandle;

SemaphoreHandle_t sensorBufferMutex;		// Mutex holding access to IR and red buffers
SemaphoreHandle_t hrBufferMutex;			// Mutex holding access to HR history buffer

uint32_t irBuffer[BUFFER_SIZE] = {0};		// Circular buffer to store IR values
uint32_t redBuffer[BUFFER_SIZE] = {0};		// Circular buffer to store red values
uint32_t maxBufferValue = IR_PLOT_MAX_VAL;		// Holds maximum value in IR buffer for display plotting
uint32_t minBufferValue = IR_PLOT_MIN_VAL;

uint8_t nextBufferIndex;				// Buffer next update index
uint8_t bufferSize;						// Used to check whether to process buffer

uint32_t hrBuffer[HR_BUFFER_SIZE] = {0};
uint8_t nextHrBufferIndex;
float hrAvg;

void dataCollectionTask(void *param) {
	TickType_t xLastWakeTime = xTaskGetTickCount();
	while (true) {
		if (xSemaphoreTake(sensorBufferMutex, portMAX_DELAY) == pdTRUE) {
			// printDebug("[collection] reading data");
			readIRToBuffer(irBuffer, redBuffer, nextBufferIndex);
			// printDebug("[collection] read data " + (String)irBuffer[nextBufferIndex] + "," + (String)redBuffer[nextBufferIndex]);
			updateMinMaxBufferValue(minBufferValue, maxBufferValue, irBuffer, nextBufferIndex);
			plotIR(irBuffer, nextBufferIndex, minBufferValue, maxBufferValue);
			incc(nextBufferIndex, BUFFER_SIZE);
			xSemaphoreGive(sensorBufferMutex);
			if (bufferSize < BUFFER_SIZE) bufferSize++;
		}
		#if ENABLE_DEBUG
		else {
			printDebug("[collection] ERR failed to take semaphore\n");
		}
		#endif
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(SAMPLING_INTERVAL_MS));
	}
}

void dataProcessingTask(void *param) {
	TickType_t xLastWakeTime = xTaskGetTickCount();
	static uint8_t lastUpdateBufferIndex = 0;
	static uint32_t irBufferProcessing[BUFFER_SIZE] = {0};
	static uint32_t redBufferProcessing[BUFFER_SIZE] = {0};
	static int8_t sensorBufferFull = 0;

	static int32_t calc_hr;
	static int8_t calc_hrValid;
	static float calc_ratio, calc_correl;

	static float hrSum = 0;
	static uint8_t hrSumWindowFull = 0;
	static uint8_t hrValidBuffer[HR_BUFFER_SIZE] = {0};
	static uint8_t hrValidCount = 0;
	static uint8_t hrWindowOldIndex = 0;

	static uint8_t intensity = 0;

	while (true) {
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(PROCESSING_INTERVAL_MS));
		blinkDisplay(1);
		
		printDebug("[processing] starting process");
		if (xSemaphoreTake(sensorBufferMutex, portMAX_DELAY) == pdTRUE) {
			printDebug("[processing] reading data from buffer: ["
						+ (String)lastUpdateBufferIndex + ":" + (String)nextBufferIndex + "]");
			forc(BUFFER_SIZE, lastUpdateBufferIndex, nextBufferIndex, {
				irBufferProcessing[__idx_forc] = irBuffer[__idx_forc];
				redBufferProcessing[__idx_forc] = redBuffer[__idx_forc];
			});
			lastUpdateBufferIndex = nextBufferIndex;
			if (!sensorBufferFull && bufferSize == BUFFER_SIZE) sensorBufferFull = true;
			xSemaphoreGive(sensorBufferMutex);
			printDebug("[processing] saved sensor buffer data");
		}
		#if ENABLE_DEBUG
		else {
			printDebug("[processing] ERR failed to take sensor semaphore");
		}
		if (nextBufferIndex==0) showBatchTimer();
		#endif

		if (sensorBufferFull) {
			rt_rf_heart_rate(irBufferProcessing, BUFFER_SIZE, lastUpdateBufferIndex, redBufferProcessing,
				&calc_hr, &calc_hrValid, &calc_ratio, &calc_correl);
		}

		xSemaphoreTake(hrBufferMutex, portMAX_DELAY);
		if (calc_hrValid) {
			printDebug("[processing] Saving heart rate to buffer: " + (String)calc_hr);
			hrBuffer[nextHrBufferIndex] = calc_hr;
		}
		else {
			printDebug("[processing] HR is invalid");
			hrBuffer[nextHrBufferIndex] = 0;
		}
		hrValidBuffer[nextHrBufferIndex] = calc_hrValid;
		incc(nextHrBufferIndex, HR_BUFFER_SIZE);

		if (calc_hrValid) {
			hrSum += calc_hr;
			hrValidCount++;
		}
		if (!hrSumWindowFull) {
			if (nextHrBufferIndex == HR_SUM_WINDOW_SIZE) hrSumWindowFull = true;
		}
		else {
			hrSum -= hrBuffer[hrWindowOldIndex];
			hrValidCount -= hrValidBuffer[hrWindowOldIndex];
			incc(hrWindowOldIndex, HR_BUFFER_SIZE);
		}
		hrAvg = (hrValidCount ? hrSum/hrValidCount : 0);
		xSemaphoreGive(hrBufferMutex);

		printDebug("[processing] hrSum:" + (String) hrSum + ", hrValidCount:" + (String) hrValidCount);

		printDebug("[processing] Displaying result");
		
		if (hrAvg < EPSILON) {
			displayHR("--");
		}
		else if (calc_hrValid) {
			displayHR(hrAvg);
		}
		displayValid(calc_hrValid);

		intensity = checkIntensityRange(hrAvg);
		printDebug("[processing] Intensity: " + (String)intensity);

		switch (intensity) {
			case 1:
				ringBuzzer(ShortBeep, ALERT_FREQUENCY);
				break;
			case 2:
				ringBuzzer(LongBeep, ALERT_FREQUENCY);
				break;
			default:
				break;
		}
	}
}

void dataUploadTask(void *param) {
	HTTPClient http;
	static uint8_t lastUpdateHRIndex = 0;

	while (true) {
		vTaskDelay(pdMS_TO_TICKS(UPLOAD_INTERVAL_MS));
		
		drawUpload();

		xSemaphoreTake(hrBufferMutex, portMAX_DELAY);
		Serial.print(lastUpdateHRIndex);
		Serial.print(" ");
		Serial.println(nextHrBufferIndex);
		String jsonData = "[";
		forc(HR_BUFFER_SIZE, lastUpdateHRIndex, nextHrBufferIndex, {
			if (__idx_forc != lastUpdateHRIndex) jsonData += ",";
			jsonData += String(hrBuffer[__idx_forc]);
		});
		lastUpdateHRIndex = nextHrBufferIndex;
		xSemaphoreGive(hrBufferMutex);
		jsonData += "]";

		// Upload to ThingSpeak
		String url;
		createThingSpeakUpdateURL(url, jsonData);

		#if ENABLE_IOT
		url = "http://api.thingspeak.com/update?api_key=3RU0HC2VFUYJHMGI&field1=" + (String)hrAvg;
		if (updateDatabase(url, http))
			drawUploadDone();
		else 
			drawUploadFail();
		#elif ENABLE_DEBUG
		Serial.print("Data: ");
		printDebug(jsonData);
		Serial.print("URL:  ");
		printDebug(url);
		#endif
	}
}

void initializeTasks() {
	nextBufferIndex = 0;
	nextHrBufferIndex = 0;
	bufferSize = 0;

	#if ENABLE_DEBUG
	Serial.print("Initializing tasks...");
	#endif
	sensorBufferMutex = xSemaphoreCreateMutex();
	hrBufferMutex = xSemaphoreCreateMutex();
	putTimer(batchTimer);
	putTimer(sampleTimer);
	
	xTaskCreatePinnedToCore(
		dataCollectionTask,
		"Data Collection",
		2000,
		NULL,
		4,
		&dataCollectionTaskHandle,
		1
	);
	#if ENABLE_DEBUG
	printDebug("Initialized data collection task");
	#endif
	
	xTaskCreatePinnedToCore(
		dataProcessingTask,
		"Data Processing",
		2000,
		NULL,
		2,
		&dataProcessingTaskHandle,
		1
	);
	#if ENABLE_DEBUG
	printDebug("Initialized data processing task");
	#endif

	xTaskCreatePinnedToCore(
		dataUploadTask,
		"Data Upload",
		4000,
		NULL,
		1,
		&dataUploadTaskHandle,
		1
	);
	#if ENABLE_DEBUG
	printDebug("Initialized data upload task");
	#endif
}