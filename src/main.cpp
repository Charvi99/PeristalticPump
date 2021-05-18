#include <mainLib.h>

#include <driver/adc.h>

Global mainVariable(new Pump(), new MQTT(), new Display(), new Control(), new Alarm());
//Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(LEDS_COUNT, LEDS_PIN, CHANNEL);

/* --- JEDNOTLIVE SUBRUTINY --- */
COROUTINE(PumpCoroutine)
{
    COROUTINE_LOOP()
    {
        mainVariable.getPump().loop();
        //COROUTINE_DELAY(15);
        COROUTINE_YIELD();
    }
}

COROUTINE(ControlRoutine)
{
    COROUTINE_LOOP()
    {
        mainVariable.getControl().loop();
        //control->loop();
        COROUTINE_YIELD();
    }
}
COROUTINE(DisplayComunicationRoutine)
{
    COROUTINE_LOOP()
    {
        mainVariable.getDisplay().loop(false, false);
        if (mainVariable.getDisplay().activePage == 0)
        {
            if ((millis() - mainVariable.getLastAlerUpdate()) > 5000)
            {
                mainVariable.getDisplay().menu.contentShow(0);
            }
            COROUTINE_DELAY(200);
        }
        mainVariable.getDisplay().myNex.NextionListen();
        COROUTINE_YIELD();
    }
}
COROUTINE(AlarmRoutine)
{
    COROUTINE_LOOP()
    {
        // ====== SOVIET HYMNA ======
        //tone(BUZZER_PIN, NOTE_C5, CTVRT, BUZZER_CHANNEL);
        /*
        noTone(BUZZER_PIN, BUZZER_CHANNEL);
        tone(BUZZER_PIN, NOTE_G4, OSMIN, BUZZER_CHANNEL);
        noTone(BUZZER_PIN, BUZZER_CHANNEL);
        tone(BUZZER_PIN, NOTE_A4, SESNACT, BUZZER_CHANNEL);
        noTone(BUZZER_PIN, BUZZER_CHANNEL);
        tone(BUZZER_PIN, NOTE_B4, CTVRT, BUZZER_CHANNEL);
        noTone(BUZZER_PIN, BUZZER_CHANNEL);
        tone(BUZZER_PIN, NOTE_E4, OSMIN, BUZZER_CHANNEL);
        noTone(BUZZER_PIN, BUZZER_CHANNEL);
        tone(BUZZER_PIN, NOTE_E4, OSMIN, BUZZER_CHANNEL);
        noTone(BUZZER_PIN, BUZZER_CHANNEL);

        tone(BUZZER_PIN, NOTE_A4, CTVRT, BUZZER_CHANNEL);
        noTone(BUZZER_PIN, BUZZER_CHANNEL);
        tone(BUZZER_PIN, NOTE_G4, OSMIN, BUZZER_CHANNEL);
        noTone(BUZZER_PIN, BUZZER_CHANNEL);
        tone(BUZZER_PIN, NOTE_F4, SESNACT, BUZZER_CHANNEL);
        noTone(BUZZER_PIN, BUZZER_CHANNEL);
        tone(BUZZER_PIN, NOTE_G4, CTVRT, BUZZER_CHANNEL);
        noTone(BUZZER_PIN, BUZZER_CHANNEL);
        tone(BUZZER_PIN, NOTE_C4, OSMIN, BUZZER_CHANNEL);
        noTone(BUZZER_PIN, BUZZER_CHANNEL);
        tone(BUZZER_PIN, NOTE_C4, OSMIN, BUZZER_CHANNEL);
        noTone(BUZZER_PIN, BUZZER_CHANNEL);

        tone(BUZZER_PIN, NOTE_D4, CTVRT, BUZZER_CHANNEL);
        noTone(BUZZER_PIN, BUZZER_CHANNEL);
        tone(BUZZER_PIN, NOTE_D4, OSMIN, BUZZER_CHANNEL);
        noTone(BUZZER_PIN, BUZZER_CHANNEL);
        tone(BUZZER_PIN, NOTE_E4, SESNACT, BUZZER_CHANNEL);
        noTone(BUZZER_PIN, BUZZER_CHANNEL);
        tone(BUZZER_PIN, NOTE_F4, CTVRT, BUZZER_CHANNEL);
        noTone(BUZZER_PIN, BUZZER_CHANNEL);
        tone(BUZZER_PIN, NOTE_F4, OSMIN, BUZZER_CHANNEL);
        noTone(BUZZER_PIN, BUZZER_CHANNEL);
        tone(BUZZER_PIN, NOTE_G4, SESNACT, BUZZER_CHANNEL);
        noTone(BUZZER_PIN, BUZZER_CHANNEL);

        tone(BUZZER_PIN, NOTE_A4, CTVRT, BUZZER_CHANNEL);
        noTone(BUZZER_PIN, BUZZER_CHANNEL);
        tone(BUZZER_PIN, NOTE_B4, OSMIN, BUZZER_CHANNEL);
        noTone(BUZZER_PIN, BUZZER_CHANNEL);
        tone(BUZZER_PIN, NOTE_C5, OSMIN, BUZZER_CHANNEL);
        noTone(BUZZER_PIN, BUZZER_CHANNEL);
        tone(BUZZER_PIN, NOTE_D5, CTVRT, BUZZER_CHANNEL);
        noTone(BUZZER_PIN, BUZZER_CHANNEL);
        tone(BUZZER_PIN, NOTE_G4, OSMIN, BUZZER_CHANNEL);
        noTone(BUZZER_PIN, BUZZER_CHANNEL);
*/
        mainVariable.getAlarm().alarmLoop();
        COROUTINE_YIELD();
    }
}

COROUTINE(MQTTComunicationRoutine)
{
    COROUTINE_LOOP()
    {
        COROUTINE_AWAIT(WiFi.status() == WL_CONNECTED);
        mainVariable.getMQTT().loop();
        COROUTINE_YIELD();
    }
}

/* --- POCATECNI NASTAVENI PROGRAMU --- */
void setup()
{
    Serial.begin(115200);
    mainVariable.getAlarm().strip.setBrightness(80);
    for (size_t i = 0; i < 3; i++)
    {
        mainVariable.getAlarm().showRun();
        tone(BUZZER_PIN, NOTE_C3, 350, BUZZER_CHANNEL);
        noTone(BUZZER_PIN, BUZZER_CHANNEL);
        mainVariable.getAlarm().shutDownLED();
        delay(350);
    }

    Serial.println("Let's setup MQTT");
    mainVariable.getMQTT().MQTTbegin();
    Serial.println("MQTT should work");

    pinMode(36, OUTPUT);
    //pinMode(5, OUTPUT);
    pinMode(15, INPUT_PULLUP);

    pinMode(DRIVER_DIRECTION_PIN_CW, OUTPUT);
    pinMode(DRIVER_DIRECTION_PIN_ACW, OUTPUT);

    //strip.begin();

    ledcSetup(0, 1000, 8);
    ledcAttachPin(DRIVER_ENABLE_PIN, 0);

    mainVariable.getPump().ina219.begin();
    mainVariable.getPump().ina219.setCalibration_32V_2A();

    mainVariable.getDisplay().activePage = 0;
    mainVariable.getDisplay().setPage(0);
    mainVariable.getDisplay().menu.contentShow(0);

    CoroutineScheduler::setup();
}

/* --- SEKVENCNI VYKONAVANI SUBRUTIN --- */

int readLightSensor()
{
    adc1_config_width(ADC_WIDTH_BIT_10);                        //Range 0-1023
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11); //ADC_ATTEN_DB_11 = 0-3,6V
    return adc1_get_raw(ADC1_CHANNEL_0);                        //Read analog
}
int test = 3;
void loop()
{
    if (test < 3)
    { /*
        digitalWrite(DRIVER_DIRECTION_PIN_CW, HIGH);
        digitalWrite(DRIVER_DIRECTION_PIN_ACW, LOW);
        ledcWrite(0, 254);
        delay(1000);

        digitalWrite(DRIVER_DIRECTION_PIN_CW, HIGH);
        digitalWrite(DRIVER_DIRECTION_PIN_ACW, LOW);
        ledcWrite(0, 0);
        delay(500);

        digitalWrite(DRIVER_DIRECTION_PIN_CW, LOW);
        digitalWrite(DRIVER_DIRECTION_PIN_ACW, HIGH);
        ledcWrite(0, 254);
        delay(1000);

        digitalWrite(DRIVER_DIRECTION_PIN_CW, LOW);
        digitalWrite(DRIVER_DIRECTION_PIN_ACW, HIGH);
        ledcWrite(0, 0);
        delay(500);
*/
        digitalWrite(5, HIGH);
        delay(1500);
        digitalWrite(5, LOW);
        delay(500);
        test++;
    }
    else
    {
        CoroutineScheduler::loop();
        AsyncElegantOTA.loop();
    }

}
