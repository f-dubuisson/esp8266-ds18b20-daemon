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
#include <eeprom_esp8266.h>
#include "BufferedMetric.h"
#include "config.h"

#define DEBUG

#ifdef DEBUG
#define _print(a) Serial.print(a)
#define _println(a) Serial.println(a)
#else
#define _print(a)
#define _println(a)
#endif

WiFiClient espClient;
PubSubClient client(espClient);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

BufferedMetric temperature(minTemperatureOffset);
BufferedMetric battery(minBatteryOffset);
long wakeUpCount = 0;


ADC_MODE(ADC_VCC);

void setup_wifi() {
	delay(10);
	// We start by connecting to a WiFi network
	_println();
	_print("Connecting to ");
	_print(ssid);

	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		_print(".");
	}

	_println("");
	_print("WiFi connected; IP address: ");
	_println(WiFi.localIP());
}

void setup() {
	// setup serial port
	Serial.begin(115200);
	_println("");
	_println("---------------------");
	_println("Starting up...");

	// setup OneWire bus
	DS18B20.begin();

	EEPROM.begin(64);

	int addr = 0;
	float value;
	EEPROM.get(addr, wakeUpCount);
	addr += sizeof(wakeUpCount);
	EEPROM.get(addr, value);
	addr += sizeof(value);
	temperature.updateValue(value);
	EEPROM.get(addr, value);
	addr += sizeof(value);
	battery.updateValue(value);
}

void setup_mqtt() {
	// Loop until we're reconnected
	client.setServer(mqtt_server, 1883);
	while (!client.connected()) {
		_print("Attempting MQTT connection...");
		// Attempt to connect
		if (client.connect("ESP8266Client", mqtt_username, mqtt_password)) {
			_println("connected");
		} else {
			_print("failed, rc=");
			_print(client.state());
			_println(" try again in 5 seconds");
			// Wait 5 seconds before retrying
			delay(5000);
		}
	}
	client.loop();
}

float getTemperature() {
	_println("Requesting DS18B20 temperature...");
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

	bool needUpdate = (wakeUpCount >= maxWakeUpCount);
	needUpdate |= battery.updateValue(newVcc);
	needUpdate |= temperature.updateValue(newTemperature);

	int addr = 0;
	addr += sizeof(long);
	
	if (needUpdate) {
		battery.setValue(newVcc);
		temperature.setValue(newTemperature);

		setup_wifi();
		setup_mqtt();

		// convert temperature to a string with two digits before the comma and 2 digits for precision and send
		EEPROM.put(addr, temperature.getValue());
		addr += sizeof(float);
		char temperatureString[6];
		dtostrf(temperature.getValue(), 2, 2, temperatureString);
		_print("Sending temperature: ");
		_println(temperatureString);
		client.publish(mqtt_topic_temperature, temperatureString);

		// convert battery to a string with two digits before the comma and 2 digits for precision and send
		EEPROM.put(addr, battery.getValue());
		addr += sizeof(float);
		char batteryString[6];
		dtostrf(battery.getValue(), 2, 2, batteryString);
		_print("Sending battery: ");
		_println(batteryString);
		client.publish(mqtt_topic_battery, batteryString);

		wakeUpCount = 0;
				
		_println("Closing MQTT connection...");
		client.disconnect();

		_println("Closing WiFi connection...");
		WiFi.disconnect();

		delay(100);
	} else {
		// Nothing to do
		_println("Nothing interesting to send.");
	}

	EEPROM.put(0, wakeUpCount);
	EEPROM.commit();
	
	_print("Entering deep sleep mode for ");
	_print(SLEEP_DELAY_IN_SECONDS);
	_println(" seconds...");
	ESP.deepSleep(SLEEP_DELAY_IN_SECONDS * 1000000, WAKE_RF_DEFAULT);

	delay(500);   // wait for deep sleep to happen
}
