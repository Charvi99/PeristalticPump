#pragma once
#include "Pump.h"

class Sensor
{
public:
    Sensor();
    float shuntvoltage;
    float busvoltage;
    float current_mA;
    float loadvoltage;
    float power_mW;
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
    unsigned short ActuallCurrent;
};