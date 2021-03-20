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
    String JsonSettings;
    String JsonData;
};

extern Global Mover;


void MQTT_Task( void * pvParameters );
void Main_Task( void * pvParameters );