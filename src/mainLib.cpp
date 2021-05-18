#include "mainLib.h"


Pump& Global::getPump()
{
    return _pump;
}
MQTT& Global::getMQTT()
{
    return _mqtt;
}
Display& Global::getDisplay()
{
    return _disp;
}
Control& Global::getControl()
{
    return _control;
}
Alarm& Global::getAlarm()
{
    return _alarm;
}

String &Global::getJSONSettings()
{
    return _JSONsettings;
}
String &Global::getJSONData()
{
    return _JsonData;
}
String &Global::getAlert()
{
    return _Alert;
}
int &Global::getAlertPriority()
{
    return _AlertPriority;
}
unsigned long &Global::getLastAlerUpdate()
{
    return _LastAlertUpdate;
}
