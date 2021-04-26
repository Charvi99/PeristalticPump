#include <mainLib.h>

#include <driver/adc.h>

Global mainVariable(new Pump(), new MQTT(), new Display(), new Control(), new Alarm());

/* --- JEDNOTLIVE SUBRUTINY --- */
COROUTINE(MotorCoroutine)
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
        //disp->loop(false, false);
        if (mainVariable.getDisplay().activePage == 0)
            COROUTINE_DELAY(150);
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
        COROUTINE_YIELD();
    }
}
COROUTINE(AlarmRoutineLED)
{
    COROUTINE_LOOP()
    {
        /* for (int j = 0; j < 255; j += 2)
        {
            for (int i = 0; i < LEDS_COUNT; i++)
            {
                strip.setLedColorData(i, strip.Wheel((i * 256 / LEDS_COUNT + j) & 255));
            }
            strip.show();
            delay(5);
        }*/
        //mainVariable.getPump().getCurrent();
        COROUTINE_DELAY(150);

        COROUTINE_YIELD();
    }
}
COROUTINE(MQTTComunicationRoutine)
{
    COROUTINE_LOOP()
    {
        COROUTINE_AWAIT(WiFi.status() == WL_CONNECTED);
        mainVariable.getMQTT().loop();
        //mqtt->loop();
        COROUTINE_YIELD();
    }
}

/* --- POCATECNI NASTAVENI PROGRAMU --- */
void setup()
{
    Serial.begin(115200);
    //Serial.println("Let's setup MQTT");
    //mainVariable.getMQTT().MQTTbegin();
    //Serial.println("MQTT should work");

    pinMode(36, OUTPUT);
    //pinMode(2, OUTPUT);
    pinMode(15, INPUT_PULLUP);

    pinMode(12, OUTPUT);
    pinMode(13, OUTPUT);

    CoroutineScheduler::setup();

    ledcSetup(0, 1000, 8);
    ledcAttachPin(DRIVER_ENABLE_PIN, 0);

    digitalWrite(SSR_PIN, GO);
    delay(1000);
    digitalWrite(SSR_PIN, STOP);

}

/* --- SEKVENCNI VYKONAVANI SUBRUTIN --- */

int readLightSensor()
{
    adc1_config_width(ADC_WIDTH_BIT_10);                        //Range 0-1023
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11); //ADC_ATTEN_DB_11 = 0-3,6V
    return adc1_get_raw(ADC1_CHANNEL_0);                        //Read analog
}
int test = 0;
void loop()
{
    /* if (test <= 1)
    {

        digitalWrite(DRIVER_ENABLE_PIN, GO);
        digitalWrite(DRIVER_DIRECTION_PIN_CW, HIGH);
        digitalWrite(DRIVER_DIRECTION_PIN_CW, LOW);
        delay(1000);

        digitalWrite(DRIVER_ENABLE_PIN, STOP);
        digitalWrite(DRIVER_DIRECTION_PIN_CW, HIGH);
        digitalWrite(DRIVER_DIRECTION_PIN_CW, LOW);
        delay(1000);

        test++;
    }
    else if (test > 2)*/
    CoroutineScheduler::loop();
}
