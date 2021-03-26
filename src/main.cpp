#include <mainLib.h>

Pump *pumpa = new Pump();
Display *disp = new Display();
Control *control = new Control();
MQTT *mqtt = new MQTT();

Global mainVariable;

/* --- JEDNOTLIVE SUBRUTINY --- */
COROUTINE(MotorCoroutine)
{
    COROUTINE_LOOP()
    {
        
        //mainVariable->getPump().loop();
        pumpa->loop();
        COROUTINE_YIELD();
    }
}

COROUTINE(ControlRoutine)
{
    COROUTINE_LOOP()
    {
        //mainVariable->getControl().loop();
        control->loop();
        COROUTINE_YIELD();
    }
}
COROUTINE(DisplayComunicationRoutine)
{
    COROUTINE_LOOP()
    {
        //mainVariable->getDisplay().loop(false, false);
        disp->loop(false, false);
        COROUTINE_YIELD();
    }
}
COROUTINE(AlarmRoutine)
{
    COROUTINE_LOOP()
    {
        //button.loop();
        COROUTINE_YIELD();
    }
}
COROUTINE(MQTTComunicationRoutine)
{
    COROUTINE_LOOP()
    {
        COROUTINE_AWAIT(WiFi.status() == WL_CONNECTED);
        //mainVariable->getMQTT().loop();
        mqtt->loop();
        COROUTINE_YIELD();
    }
}

/* --- POCATECNI NASTAVENI PROGRAMU --- */
void setup()
{
    Serial.begin(115200);

    Serial.println("Let's setup MQTT");
    mqtt->MQTTbegin();
    Serial.println("MQTT should work");

    pinMode(36, OUTPUT);
    pinMode(15, INPUT_PULLUP);

    CoroutineScheduler::setup();
}

/* --- SEKVENCNI VYKONAVANI SUBRUTIN --- */
void loop()
{
    CoroutineScheduler::loop();
}
