#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

#define NUM_SAMPLES 100

ESP8266WiFiMulti WiFiMulti;

void setup() {

	Serial.begin(115200);
	// Serial.setDebugOutput(true);

	Serial.println();
	Serial.println();
	Serial.println("Serial started");

	WiFi.mode(WIFI_STA);
	WiFiMulti.addAP("Protospace", "yycmakers");

	pinMode(A0, INPUT);

	delay(5000);
}

void loop() {
	// wait for WiFi connection
	if ((WiFiMulti.run() == WL_CONNECTED)) {

		std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

		client->setInsecure();

		HTTPClient https;

		Serial.print("[HTTPS] begin...\n");
		if (https.begin(*client, "https://api.my.protospace.ca/stats/alarm/")) {
			Serial.print("[DATA] Get light data...\n");

			long total = 0;

			for (int i=0; i < NUM_SAMPLES; i++) {
				total += analogRead(A0);
				delay(10);
			}

			total /= NUM_SAMPLES;

			Serial.printf("[DATA] Average value read: %d\n", total);

			Serial.print("[HTTPS] POST...\n");

			// start connection and send HTTP header
			https.addHeader("Content-Type", "application/x-www-form-urlencoded");
			int httpCode = https.POST("data=" + String(total));

			// httpCode will be negative on error
			if (httpCode > 0) {
				// HTTP header has been send and Server response header has been handled
				Serial.printf("[HTTPS] POST... code: %d\n", httpCode);

				// file found at server
				if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
					String payload = https.getString();
					Serial.println(payload);
				}
			} else {
				Serial.printf("[HTTPS] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
			}

			https.end();
		} else {
			Serial.printf("[HTTPS] Unable to connect\n");
		}

		Serial.println("Waiting 60s");
		delay(60*1000);
	} else {
		Serial.println("[WIFI] Error, wait 10s before trying again...");
		delay(10000);
	}
}