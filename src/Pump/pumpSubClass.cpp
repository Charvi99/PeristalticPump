#include "Pump.h"
#include "pumpSubClass.h"

Sensor::Sensor()
{
    shuntvoltage = 0;
    busvoltage = 0;
    rawCurrent_mA = 0;
    smoothCurrent_mA = 0;
    for (size_t i = 0; i <  (sizeof(rawCurrentArray)/sizeof(rawCurrentArray[0])); i++)
        rawCurrentArray[i] = 0;    
    loadvoltage = 0;
    power_mW = 0;
}

Settings::Settings()
{
    Pin = 2;
    PinDirection = 4;
    MaxCurrent = 200;
    MaxSpeed = 100;
    RotationToMl = 100;
    tubeLenght = 300;
    tubeDiameter = 3;
}

Parameters::Parameters()
{
    Run = false;
    Mode = MANUAL;
    Dose = 50;
    Speed = 100;
    Direction = true;
    Interval = 5;
    RampTime = 1;
}

Status::Status()
{
    Enable = false;
    Running = false;
    ActuallRotationCount = 0;
    WantedRotationCount = 0;
    ActuallRawCurrent = 0;
    filledTubes = false;
    wifiConected = false;
    MQTTConnected = false;
}