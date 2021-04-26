#include "Alarm/Alarm.h"

Alarm::Alarm()
{
    //dont have idea about it
    strip.begin();
}
void Alarm::alarmLoop()
{
    switch (status)
    {
    case Status::RUNNING:
        showRun();
        LEDBreathing();
        break;
    case Status::WARNING:
        showWarning();
        break;
    case Status::STOPED:
        showStop();
        LEDBreathing();
        break;
    case Status::ERROR:
        showError();
        break;
    case Status::MQTT_CONECTED:
        showMQTT();
        LEDBreathing();
        break;
    default:
        showStop();
        LEDBreathing();
        break;
    }
    beeperLoop();
}

void Alarm::showRun()
{
    //strip.setLedColorData(i, strip.Wheel((i * 256 / LEDS_COUNT + j) & 255));
    strip.setAllLedsColor(0, 254, 0);
}
void Alarm::showStop()
{
    strip.setAllLedsColor(0, 0, 0);
}
void Alarm::showWarning()
{
    strip.setAllLedsColor(254,254,0);
}
void Alarm::showError()
{
    strip.setAllLedsColor(254,0,0);
}
void Alarm::showMQTT()
{
    strip.setAllLedsColor(0,0,254);
}
void Alarm::LEDBreathing()
{
    if (millis() - lastStripUpdateTimeMark > 10)
    {
        if (stripBrightnessRise == true && stripBrightnessLvl < 255)
        {
            stripBrightnessLvl++;
            if (stripBrightnessLvl > 254)
                stripBrightnessRise = false;
        }
        else if (stripBrightnessRise == false && stripBrightnessLvl > 1)
        {
            stripBrightnessLvl--;
            if (stripBrightnessLvl <= 1)
                stripBrightnessRise = true;
        }
        strip.setBrightness(stripBrightnessLvl);
        lastStripUpdateTimeMark = millis();
    }
}
void Alarm::SendData()
{
    if (mainVariable.getMQTT().client.connected() && millis() - lastDataSendTimeMark > DATA_SEND_FREQUENCY)
    {
        DynamicJsonDocument settingsToSend(1024);
        for (size_t i = 0; i < DATA_COUNT; i++)
        {
        settingsToSend[i]["Name"] = settings[i].Name;
        settingsToSend[i]["Value"] = settings[i].Value;
        settingsToSend[i]["Unit"] = settings[i].Unit;
        }
        
        
        String settingsToSendString = "";
        serializeJson(settingsToSend, settingsToSendString);
        mainVariable.getMQTT().publish("peristaltic/data", settingsToSendString.c_str());
        lastDataSendTimeMark = millis();
    }
}