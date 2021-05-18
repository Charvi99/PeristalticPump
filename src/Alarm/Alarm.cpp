#include "Alarm/Alarm.h"

Alarm::Alarm()
{
    //dont have idea about it
    status = STOPED;
    strip.begin();
}
void Alarm::alarmLoop()
{
    status = (AlarmStatus)mainVariable.alarmStatus;

    if ((status == MQTT_CONECTED || status == WARNING || status == ERROR))
    {
        stripUpdate();
        mainVariable.alarmStatus = (int)Alarm::AlarmStatus::RUNNING;
    }
    else if (millis() - lastStatusUpdateTimeMark > 2000)
    {
        lastStatusUpdateTimeMark = millis();
        stripUpdate();
    }

    //beeperLoop();
}
void Alarm::stripUpdate()
{
    switch (status)
    {
    case AlarmStatus::RUNNING:
        showRun();
        LEDBreathing();
        break;
    case AlarmStatus::WARNING:
        showWarning();
        break;
    case AlarmStatus::STOPED:
        showStop();
        LEDBreathing();
        break;
    case AlarmStatus::ERROR:
        showError();
        break;
    case AlarmStatus::MQTT_CONECTED:
        showMQTT();
        LEDBreathing();
        break;
    default:
        showWarning();
        LEDBreathing();
        break;
    }
}

void Alarm::showRun()
{
    strip.setAllLedsColor(0, 254, 0);
}
void Alarm::showStop()
{
    strip.setAllLedsColor(0, 0, 0);
}
void Alarm::showWarning()
{
    strip.setAllLedsColor(254, 254, 0);
}
void Alarm::showError()
{
    strip.setAllLedsColor(254, 0, 0);
}
void Alarm::showMQTT()
{
    strip.setAllLedsColor(0, 0, 254);
}

void Alarm::shutDownLED()
{
    strip.setAllLedsColor(0, 0, 0);
}
void Alarm::LEDBreathing()
{
    if (millis() - lastStripUpdateTimeMark > 10)
    {
        if (stripBrightnessRise == true && stripBrightnessLvl < MAX_BRIGHTNESS)
        {
            stripBrightnessLvl++;
            if (stripBrightnessLvl > MAX_BRIGHTNESS)
                stripBrightnessRise = false;
        }
        else if (stripBrightnessRise == false && stripBrightnessLvl > MIN_BRIGHTNESS)
        {
            stripBrightnessLvl--;
            if (stripBrightnessLvl <= MIN_BRIGHTNESS)
                stripBrightnessRise = true;
        }
        strip.setBrightness(stripBrightnessLvl);
        lastStripUpdateTimeMark = millis();
    }
}
void Alarm::SendData()
{
}

void Alarm::setStatus()
{
    status = AlarmStatus::RUNNING;
    return;
}