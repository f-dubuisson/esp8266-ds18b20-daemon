#ifndef _CONFIG_H
#define _CONFIG_H

#define SLEEP_DELAY_IN_SECONDS  30
#define ONE_WIRE_BUS            D1      // DS18B20 pin

const char* ssid = "xxxxxx";
const char* password = "xxxxxxx";

const char* mqtt_server = "xxx.xxx.xxx.xxx";
const char* mqtt_username = "username"; // or NULL
const char* mqtt_password = "password"; // or NULL
const char* mqtt_topic_temperature = "/sensors/test/temperature";
const char* mqtt_topic_battery = "/sensors/test/battery";

const long minTemperatureOffset = 0; // any change in value lower than this offset will be ignored
const long maxTemperatureDelay = 0; // send the temperature value after this delay, even if the value did not change

const long maxWakeUpCount = 5; // send value after this count, even if no data has changed

#endif
