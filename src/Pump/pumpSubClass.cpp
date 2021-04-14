#include "Pump.h"
#include "pumpSubClass.h"

Sensor::Sensor()
{
    shuntvoltage = 0;
    busvoltage = 0;
    current_mA = 0;
    loadvoltage = 0;
    power_mW = 0;
}

Settings::Settings()
{
    Pin = 2;
    PinDirection = 4;
    MaxCurrent = 200;
    MaxSpeed = 100;
    RotationToMl = 500;
    tubeLenght = 300;
    tubeDiameter = 3;
}

Parameters::Parameters()
{
    Run = false;
    Mode = MANUAL;
    Dose = 10;
    Speed = 100;
    Direction = true;
    Interval = 0;
    RampTime = 0;
}

Status::Status()
{
    Enable = false;
    Running = false;
    ActuallRotationCount = 0;
    WantedRotationCount = 0;
    ActuallCurrent = 0;
    filledTubes = false;
    wifiConected = false;
    MQTTConnected = false;
}