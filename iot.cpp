#include "iot.h"
#include <WiFi.h>

void createThingSpeakUpdateURL(String &url, String &data) {
	url = IOT_UPDATE_URL;
	url += IOT_FIELD_NAME;
	url += data;
}

void connectWiFi(const char* ssid, const char* pass) {
	WiFi.begin(ssid, pass);
	while(WiFi.status() != WL_CONNECTED) {
		delay(1000);
		Serial.println("Connecting to WiFi...");
	}
	Serial.println("WiFi connected");
}

bool isInternetAvailable() {
	WiFiClient client;
	return client.connect("8.8.8.8", 53); // Try connecting to Google's DNS server
}

uint8_t updateDatabase(const String &url, HTTPClient &http) {
	// check if wifi is connected
	if (WiFi.status() != WL_CONNECTED) {
		Serial.println("Wi-Fi not connected. Skipping upload.");
		return 0;
	}
	if (!isInternetAvailable()) {
		Serial.println("Wi-Fi connected without internet. Skipping upload.");
		return 0;
	}

	uint8_t status = 0;
	http.begin(url);
	int httpResponseCode = http.GET();

	if (httpResponseCode == 200) {
		String response = http.getString();
		if (response == "0") {
			Serial.println("ThingSpeak error: Rate limit exceeded");
		}
		else {
			Serial.println("Updated ThingSpeak");
			status = 1; // ok
		}
	}
	else {
		Serial.println("HTTP error: " + String(httpResponseCode));
	}

	http.end();
	return status;
}