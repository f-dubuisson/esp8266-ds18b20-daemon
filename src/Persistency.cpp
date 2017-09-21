#include <eeprom_esp8266.h>
#include "Persistency.h"

void Persistency::load() {
    EEPROM.begin(64);    

    // Load existing data
	int addr = 0;
	EEPROM.get(addr, wakeUpCount);
	addr += sizeof(wakeUpCount);
	EEPROM.get(addr, temperature);
	addr += sizeof(battery);
	EEPROM.get(addr, battery);
}

int Persistency::getWakeUpCount() {
    return wakeUpCount;
}

float Persistency::getTemperature() {
    return temperature;
}

float Persistency::getBattery() {
    return battery;
}

void Persistency::setWakeUpCount(int value) {
    if (value != wakeUpCount) {
        wakeUpCount = value;
        EEPROM.put(0, wakeUpCount);    
    }
}

void Persistency::setTemperature(float value) {
    if (value != temperature) {
        temperature = value;
        EEPROM.put(sizeof(int), temperature);
    }
}

void Persistency::setBattery(float value) {
    if (value != battery) {
        battery = value;
        EEPROM.put(sizeof(int) + sizeof(float), battery);
    }
}

void Persistency::commit() {
    EEPROM.commit();
}