#ifndef _PERSISTENCY_H
#define _PERSISTENCY_H

class Persistency {
public:
    void load();

    int getWakeUpCount();
    float getTemperature();
    float getBattery();

    void setWakeUpCount(int value);
    void setTemperature(float value);
    void setBattery(float value);

    void commit();

private:
    int wakeUpCount;
    float temperature; 
    float battery;
};

#endif