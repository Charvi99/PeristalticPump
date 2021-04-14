#pragma once
#include <Arduino.h>

#include <Pump\Pump.h>
#include <Mqtt\Mqtt.hpp>
#include <Display/Display.hpp>
#include <Controls/Controls.hpp>
#include <AceCommon.h>
#include <AceRoutine.h>
#include <FreeRTOS.h>

using namespace ace_routine;

class Pump;
class MQTT;
class Display;
class Control;

class Global
{
public:
     Global(Pump* newPump, MQTT* newMQTT, Display* newDisp, Control* newControl):
     _pump(*newPump), _mqtt(*newMQTT), _disp(*newDisp), _control(*newControl) {}
    // ~Global();

    Pump &getPump();
    MQTT &getMQTT();
    Display &getDisplay();
    Control &getControl();

    String &getJSONSettings();
    String &getJSONData();

private:
    Pump &_pump;
    MQTT &_mqtt;
    Display &_disp;
    Control &_control;

    String _JSONsettings;
    String _JsonData;
};

extern Global mainVariable;

void MQTT_Task(void *pvParameters);
void Main_Task(void *pvParameters);