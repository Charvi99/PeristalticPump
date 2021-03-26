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

class Global
{
public:
    Global();
   // ~Global();
    /*
    Pump getPump();
    MQTT getMQTT();
    Display getDisplay();
    Control getControl();
*/
    String &getJSONSettings();
    String &getJSONData();

private:
    /*   Pump _pump;
    MQTT _mqtt;
    Display _disp;
    Control _control;
*/
    String _JSONsettings;
    String _JsonData;
};

extern Global mainVariable;

void MQTT_Task(void *pvParameters);
void Main_Task(void *pvParameters);