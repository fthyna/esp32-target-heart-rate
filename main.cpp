/* TODO
> Test circular buffer
> Test circular buffer processing
*/

#include <Wire.h>
#include "MAX30105.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "algorithm_by_RF.h"
#include "modified_RF_algorithm.h"

#include "config.h"
#include "helper.h"
#include "display.h"
#include "iot.h"
#include "sensor.h"
#include "tasks.h"
#include "alarm.h"
#include "intensity.h"

void setup()
{
	Serial.begin(115200);
	
	Wire.begin(PIN_SDA, PIN_SCL, I2C_SPEED);
	
	initializeSensor();
	initializeDisplay();
	initializeBuzzer();
	initializeKarvonen();
	displayText("Lo:" + (String)((int)getLoHR()) + "BPM");
	delay(1000);
	displayText("Hi:" + (String)((int)getHiHR()) + "BPM");
	delay(1000);
	displayText("WiFi...");
	connectWiFi(WIFI_SSID, WIFI_PASSWORD);
	displayText("Terhubung");
	delay(200);
	displayText("KOMPOR");
	initializeTasks();
}

void loop()
{
	delay(1000);
}