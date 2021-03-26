#include "mainLib.h"

Global::Global()
{
    Pump _pump;
    MQTT _mqtt;
    Display _disp;
    Control _control;

    _JSONsettings = "";
    _JsonData = "";
}
/*
Pump Global::getPump()
{
    return _pump;
}
MQTT Global::getMQTT()
{
    return _mqtt;
}
Display Global::getDisplay()
{
    return _disp;
}
Control Global::getControl()
{
    return _control;
}
*/
String &Global::getJSONSettings()
{
    return _JSONsettings;
}
String &Global::getJSONData()
{
    return _JsonData;
}
