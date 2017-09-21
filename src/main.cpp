/*
 Sketch which publishes temperature data from a DS1820 sensor to a MQTT topic.

 This sketch goes in deep sleep mode once the temperature has been sent to the MQTT
 topic and wakes up periodically (configure SLEEP_DELAY_IN_SECONDS accordingly).

 Hookup guide:
 - connect D0 pin to RST pin in order to enable the ESP8266 to wake up periodically
 - DS18B20:
		 + connect VCC (3.3V) to the appropriate DS18B20 pin (VDD)
		 + connect GND to the appopriate DS18B20 pin (GND)
		 + connect D4 to the DS18B20 data pin (DQ)
		 + connect a 4.7K resistor between DQ and VCC.

Initial source code: https://gist.github.com/jeje/57091acf138a92c4176a#file-esp8266_temperature-ino
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <streaming.h>
#include "BufferedMetric.h"
#include "Persistency.h"
#include "config.h"

#define DEBUG

#ifdef DEBUG
#define _print(a) Serial.print(a)
#define _println(a) Serial.println(a)
#define LOG(a) Serial << a;
#else
#define _print(a)
#define _println(a)
#define LOG(a)
#endif

WiFiClient espClient;
PubSubClient client(espClient);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

Persistency persistency;

BufferedMetric temperature(minTemperatureOffset);
BufferedMetric battery(minBatteryOffset);
long wakeUpCount = 0;


ADC_MODE(ADC_VCC);

void setup_wifi() {
	delay(10);
	// We start by connecting to a WiFi network
	LOG(endl << "Connecting to " << ssid << endl)

	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		LOG(".");
	}

	LOG(endl << "WiFi connected; IP address: " << WiFi.localIP() << endl)
}

void setup() {
	// setup serial port
	Serial.begin(115200);
	LOG(endl << "---------------------" << endl);
	LOG("Starting up..." << endl);

	// setup OneWire bus
	DS18B20.begin();

	// Loading EEPROM
	persistency.load();
	wakeUpCount = persistency.getWakeUpCount();
	temperature.setValue(persistency.getTemperature());
	battery.setValue(persistency.getBattery());
}

void setup_mqtt() {
	// Loop until we're reconnected
	client.setServer(mqtt_server, 1883);
	while (!client.connected()) {
		LOG("Attempting MQTT connection: ");
		// Attempt to connect
		if (client.connect("ESP8266Client", mqtt_username, mqtt_password)) {
			LOG("connected" << endl);
		} else {
			LOG("failed, rc=" << client.state() << "; try again in 5 seconds" << endl);
			delay(5000);
		}
	}
	client.loop();
}

float getTemperature() {
	float temp;
	do {
		DS18B20.requestTemperatures(); 
		temp = DS18B20.getTempCByIndex(0);
		delay(100);
	} while (temp == 85.0 || temp == (-127.0));
	return temp;
}

void loop() {
	wakeUpCount ++;
	float newVcc = ESP.getVcc();
	float newTemperature = getTemperature();

	LOG("wake up count: " << wakeUpCount << " (limit=" << maxWakeUpCount << ")" << endl)
	LOG("battery: old=" << battery.getValue() << ", new=" << newVcc << " (minDelta=" << minBatteryOffset << ")" << endl)
	LOG("temperature: old=" << temperature.getValue() << ", new=" << newTemperature << " (minDelta=" << minTemperatureOffset << ")" << endl)

	bool needUpdate = (wakeUpCount >= maxWakeUpCount);
	needUpdate |= battery.updateValue(newVcc);
	needUpdate |= temperature.updateValue(newTemperature);

	if (needUpdate) {
		setup_wifi();
		setup_mqtt();

		LOG("Sending update..." << endl)

		// convert temperature to a string with two digits before the comma and 2 digits for precision and send
		battery.setValue(newVcc);
		persistency.setTemperature(temperature.getValue());
		char temperatureString[6];
		dtostrf(temperature.getValue(), 2, 2, temperatureString);
		client.publish(mqtt_topic_temperature, temperatureString);

		// convert battery to a string with two digits before the comma and 2 digits for precision and send
		temperature.setValue(newTemperature);
		persistency.setBattery(battery.getValue());
		char batteryString[6];
		dtostrf(battery.getValue(), 2, 2, batteryString);
		client.publish(mqtt_topic_battery, batteryString);

		wakeUpCount = 0;
				
		LOG("Closing MQTT connection..." << endl)
		client.disconnect();

		LOG("Closing WiFi connection..." << endl)
		WiFi.disconnect();

		delay(100);
	} else {
		// Nothing to do
		LOG("Nothing interesting to send." << endl);
	}

	persistency.setWakeUpCount(wakeUpCount);
	persistency.commit();
	
	LOG("Entering deep sleep mode for " << SLEEP_DELAY_IN_SECONDS << " seconds..." << endl);
	ESP.deepSleep(SLEEP_DELAY_IN_SECONDS * 1000000, WAKE_RF_DEFAULT);

	delay(500);   // wait for deep sleep to happen
}
