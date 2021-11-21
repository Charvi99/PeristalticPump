#pragma once
#include <Arduino.h>

#include <Pump\Pump.h>
#include <Mqtt\Mqtt.hpp>
#include <Display/Display.hpp>
#include <Controls/Controls.hpp>
#include <Alarm/Alarm.h>
#include <AceCommon.h>
#include <AceRoutine.h>
#include <FreeRTOS.h>

using namespace ace_routine;

#define INFO_STATUS_OK 0
#define INFO_STATUS_WARNING 1
#define INFO_STATUS_ERROR 2
#define INFO_STATUS_IDLE 3

class Pump;
class MQTT;
class Display;
class Control;
class Alarm;

class Global
{
public:
    Global(Pump *newPump, MQTT *newMQTT, Display *newDisp, Control *newControl, Alarm *newAlarm) : _pump(*newPump), _mqtt(*newMQTT), _disp(*newDisp), _control(*newControl), _alarm(*newAlarm) {}
    // ~Global();

    Pump &getPump();
    MQTT &getMQTT();
    Display &getDisplay();
    Control &getControl();
    Alarm &getAlarm();

    String &getJSONSettings();
    String &getJSONData();
    String &getAlert();
    int &getAlertPriority();
    unsigned long &getLastAlerUpdate();
    int alarmStatus;
private:
    Pump &_pump;
    MQTT &_mqtt;
    Display &_disp;
    Control &_control;
    Alarm &_alarm;

    String _JSONsettings;
    String _JsonData;

    String _Alert;
    int _AlertPriority;
    unsigned long _LastAlertUpdate;
};

extern Global mainVariable;

void MQTT_Task(void *pvParameters);
void Main_Task(void *pvParameters);