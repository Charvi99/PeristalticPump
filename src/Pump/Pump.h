#pragma once
#include <Arduino.h>
#include <analogWrite.h>
#include "Adafruit_INA219.h"
#include <ESP32Encoder.h>
#include <Display/Display.hpp>
#include <ArduinoJson.h>

#include "pumpSubClass.h"

#define ENCODER_A 32
#define ENCODER_B 33

#define SSR_PIN 34
#define ENABLE_PIN 13

#define DRIVER_ENABLE_PIN 13
#define GO 1
#define STOP 0

#define DRIVER_DIRECTION_PIN_CW 12
#define DRIVER_DIRECTION_PIN_ACW 14

extern Adafruit_INA219 ina219;
extern ESP32Encoder pumpEncoder;

class Pump
{

public:
    Status status;
    Parameters parameters;
    Settings settings;
    Sensor sensor;

    Adafruit_INA219 ina219;
    ESP32Encoder pumpEncoder;

    Pump();
    void setup();

    void loop();
    void run();
    void runManual();
    void runSemiManual();
    void runDose();
    void runIntervalDose();
    void stop();

    void pumpEnable();
    void pumpDisable();

    void fillTubes();

    void setting(String settings);
    void setDose(unsigned short ml);
    void setSpeed(unsigned short speed);
    void setDirection(bool direction);
    void setInterval(unsigned long interval);
    void setRamp(unsigned short time);
    void setMode(unsigned int mode);

    void setTubeLenght(unsigned short current);
    void setTubeDiameter(unsigned short current);
    void setMaxCurrent(unsigned short current);
    void setMaxSpeed(unsigned short speed);
    unsigned short getMaxCurrent();
    unsigned short getMaxSpeed();
    bool isRunning();

    void IRAM_ATTR measurementLoop();
    void updateSensor();
    float getCurrent();
    long getRotation();
    long getMl();
    void resetRotation();
};

extern Pump *pumpa;