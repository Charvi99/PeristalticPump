#pragma once
#include "Pump.h"

#define COUNT_OF_SAMPLES 5

class Sensor
{
public:
    Sensor();
    float shuntvoltage;
    float busvoltage;
    float rawCurrent_mA;
    float loadvoltage;
    float power_mW;
    float rawCurrentArray[COUNT_OF_SAMPLES];
    float smoothCurrent_mA;
};

class Settings
{
public:
    Settings();
    int Pin;
    int PinDirection;
    unsigned short MaxCurrent;
    unsigned short MaxSpeed;
    unsigned int RotationToMl;
    unsigned int tubeLenght;
    unsigned int tubeDiameter;
};

class Parameters
{
public:
    enum Mode
    {
        MANUAL,
        SEMIAUTOMAT,
        AUTOMAT,
        INTERVAL
    };

    Parameters();
    bool Run;
    Mode Mode;
    unsigned short Dose;
    unsigned short Speed;
    bool Direction;
    unsigned long Interval;
    unsigned short RampTime;
};

class Status
{
public:
    Status();
    bool Enable;
    bool Running;
    long ActuallRotationCount;
    long WantedRotationCount;
    unsigned short ActuallRawCurrent;
    bool filledTubes;
    bool wifiConected;
    bool MQTTConnected;
    long duty;
};