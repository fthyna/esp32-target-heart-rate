#ifndef IOT_H
#define IOT_H

#include <Arduino.h>
#include <HTTPClient.h>

#define IOT_UPDATE_URL "http://api.thingspeak.com/update?api_key=YOUR-API-KEY"
#define IOT_FIELD_NAME "YOUR-FIELD-NAME"
#define IOT_READ_URL "https://api.thingspeak.com/channels/YOUR-THINGSPEAK-CHANNEL-ID/"

void createThingSpeakUpdateURL(String &url, String &data);
uint8_t updateDatabase(const String &url, HTTPClient &http);
void connectWiFi(const char* ssid, const char* pass);

#endif
