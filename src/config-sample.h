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

#endif
