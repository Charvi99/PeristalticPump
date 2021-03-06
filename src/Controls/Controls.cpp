#include <Controls/Controls.hpp>

Control::Control()
{
    /* --- NASTAVENI TLACITEK --- */
    start_btn.setDebounceTime(10);
    stop_btn.setDebounceTime(10);
    encoder_btn.setDebounceTime(20);

    start_btn = Button2(START_BTN);
    stop_btn = Button2(STOP_BTN);
    encoder_btn = Button2(CONTROL_ENCODER_BTN);

    start_btn.setPressedHandler(buttonPressLoop);
    start_btn.setReleasedHandler(buttonReleaseLoop);
    stop_btn.setPressedHandler(buttonPressLoop);
    stop_btn.setLongClickTime(10000);
    stop_btn.setLongClickHandler(buttonLongPressLoop);
    encoder_btn.setClickHandler(buttonPressLoop);

    /* --- PINY KLAVESNICE --- */
    pinMode(19, INPUT_PULLUP);
    pinMode(23, INPUT_PULLUP);
    pinMode(18, INPUT_PULLUP);
    pinMode(5, INPUT_PULLUP);

    pinMode(START_BTN, INPUT_PULLUP);
    pinMode(STOP_BTN, INPUT_PULLUP);
    pinMode(CONTROL_ENCODER_BTN, INPUT_PULLUP);

    /* --- NASTAVENI OVLADACIHO ENKODERU --- */
    ESP32Encoder::useInternalWeakPullResistors = UP;
    controlEncoder.attachHalfQuad(CONTROL_ENCODER_A, CONTROL_ENCODER_B);
    controlEncoder.clearCount();

    Up = false;
    Down = false;
}

/* --- HANDLER PRO TLACITKA --- */
void buttonReleaseLoop(Button2 &btn)
{
    if ((btn == mainVariable.getControl().start_btn) && (mainVariable.getPump().parameters.Mode == mainVariable.getPump().parameters.MANUAL))
    {
        mainVariable.getPump().pumpDisable();
        mainVariable.getPump().stop();
        Serial.println("tlacitko start pusteno");
    }
}
void buttonPressLoop(Button2 &btn)
{
    /* --- STISKEM START ZAPNU PUMPU --- */
    Serial.println("something pressed");

    if (btn == mainVariable.getControl().start_btn)
    {
        mainVariable.getPump().pumpEnable();
        mainVariable.getMQTT().publish("peristaltic/run", "START");
        mainVariable.getMQTT().publish("peristaltic/confirm", "ok");

        Serial.println("tlacitko start stisknuto");
        mainVariable.getAlarm().showRun();
        mainVariable.alarmStatus = 0;
    }
    /* --- STISKEM STOP VYPNU PUMPU --- */
    else if (btn == mainVariable.getControl().stop_btn)
    {
        mainVariable.getPump().pumpDisable();
        mainVariable.getPump().stop();
        mainVariable.getMQTT().publish("peristaltic/run", "STOP");
        mainVariable.getMQTT().publish("peristaltic/confirm", "ok");

        Serial.println("tlacitko stop stisknuto");
        mainVariable.getAlarm().showStop();
        mainVariable.alarmStatus = 1;
    }

    else if (btn == mainVariable.getControl().encoder_btn)
    {
        Serial.println("enc pressed");
        mainVariable.getDisplay().lastInteractionTimeMark = millis();
        
        /* --- STISKEM ENKODERU ZOBRAZIM NASTAVENI --- */
        if (mainVariable.getDisplay().activePage == 0)
        {
            if (mainVariable.getPump().isRunning())
            {
                mainVariable.getDisplay().dispSetInfo("Settings can't be change when pump is running", true);
            }
            else
            {
                mainVariable.getDisplay().activePage = 1;
                mainVariable.getDisplay().setPage(1);
            }
        }
        /* --- STISKEM ENKODERU V NASTAENI AKTIVUJI FOCUSNUTY PRVEK --- */
        else if (mainVariable.getDisplay().activePage == 1)
        {
            if (mainVariable.getDisplay().switcher == true)
                mainVariable.getDisplay().menu.selectFocused();
            else if (mainVariable.getDisplay().switcher == false)
                mainVariable.getDisplay().menu.deselectFocused();

            mainVariable.getDisplay().switcher = !mainVariable.getDisplay().switcher;
        }
    }
}
void buttonLongPressLoop(Button2 &btn)
{
    mainVariable.getDisplay().dispSetInfo("Rebooting in 5 seckonds",true);
    delay(5000);
    ESP.restart();
}

/* --- REAKCE NA AKCE ENKODERU --- */
void Control::encoderLoop()
{

    long currentCount = controlEncoder.getCount();
    if (currentCount != 0 && mainVariable.getDisplay().activePage == 1)
    {
        if (0 < currentCount)
        {
            Down = true;
            controlEncoder.clearCount();
            Serial.println("ENCODER DOWN");
            Serial.print("DOWN =");
            Serial.println(Down);
        }
        else if (0 > currentCount)
        {
            Up = true;
            controlEncoder.clearCount();
            Serial.println("ENCODER UP");
            Serial.print("UP =");
            Serial.println(Up);
        }
        mainVariable.getDisplay().loop(Up, Down);
        Up = false;
        Down = false;
    }
    else if (currentCount != 0 && mainVariable.getDisplay().activePage == 0)
    {
        if (0 < currentCount)
        {
            controlEncoder.clearCount();
            Serial.println("ENCODER DOWN");
            Serial.print("DOWN =");
            Serial.println(Down);

            if (mainVariable.getDisplay().menu.settings[4].NumValue > 1)
                mainVariable.getPump().setSpeed(mainVariable.getDisplay().menu.settings[4].NumValue - 1);
        }
        else if (0 > currentCount)
        {
            controlEncoder.clearCount();
            Serial.println("ENCODER UP");
            Serial.print("UP =");
            Serial.println(Up);

            if (mainVariable.getDisplay().menu.settings[4].NumValue < 10)
                mainVariable.getPump().setSpeed(mainVariable.getDisplay().menu.settings[4].NumValue + 1);
        }

        mainVariable.getDisplay().menu.contentShow(0);
    }
}

/* --- KONTROLA STAVU TLACITEK, KLAVESNICE A ENKODERU--- */
void Control::loop()
{
    start_btn.loop();
    stop_btn.loop();
    encoder_btn.loop();

    encoderLoop();

    if ((mainVariable.getPump().isRunning() == false) && (digitalRead(19) == LOW || digitalRead(23) == LOW || digitalRead(18) == LOW || digitalRead(5) == LOW))
    {
        unsigned pom = 0;
        if (digitalRead(19) == LOW)
            pom = 1;
        else if (digitalRead(23) == LOW)
            pom = 0;
        else if (digitalRead(18) == LOW)
            pom = 3;
        else if (digitalRead(5) == LOW)
            pom = 2;

        mainVariable.getPump().setMode(pom);
        if (mainVariable.getMQTT().client.connected())
        {
            DynamicJsonDocument settingsToSend(1024);
            for (size_t i = 0; i < SETTINGS_COUNT-1; i++)
            {
                settingsToSend[i]["Name"] = mainVariable.getDisplay().menu.settings[i+1].Name;
                settingsToSend[i]["Value"] = mainVariable.getDisplay().menu.settings[i+1].NumValue;
                settingsToSend[i]["Unit"] = mainVariable.getDisplay().menu.settings[i+1].Unit;
            }

            String settingsToSendString = "";
            serializeJson(settingsToSend, settingsToSendString);
            mainVariable.getMQTT().publish("peristaltic/settings", settingsToSendString.c_str());
        }
        mainVariable.getDisplay().menu.contentShow(0);
    }
    else if ((mainVariable.getPump().isRunning() == true) && (digitalRead(19) == LOW || digitalRead(23) == LOW || digitalRead(18) == LOW || digitalRead(5) == LOW))
    {
        mainVariable.getDisplay().dispSetInfo("Mode can't be change when pump is running", true);
        infoUpdateTimeMark = millis();
    }
    if (mainVariable.getDisplay().currentInfo == "Mode can't be change when pump is running" && (millis() - infoUpdateTimeMark > 2000))
        mainVariable.getDisplay().menu.contentShow(0);
}