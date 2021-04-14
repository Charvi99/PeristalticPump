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

String &Global::getJSONSettings()
{
    return _JSONsettings;
}
String &Global::getJSONData()
{
    return _JsonData;
}
