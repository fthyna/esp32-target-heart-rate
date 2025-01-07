#ifndef IOT_H
#define IOT_H

#include <Arduino.h>
#include <HTTPClient.h>

#define IOT_UPDATE_URL "http://api.thingspeak.com/update?api_key=3RU0HC2VFUYJHMGI"
#define IOT_FIELD_NAME "&field2="
#define IOT_READ_URL "https://api.thingspeak.com/channels/2790514/"

void createThingSpeakUpdateURL(String &url, String &data);
uint8_t updateDatabase(const String &url, HTTPClient &http);
void connectWiFi(const char* ssid, const char* pass);

#endif