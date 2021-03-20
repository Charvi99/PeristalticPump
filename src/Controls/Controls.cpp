#include <Controls/Controls.hpp>

Control::Control()
{
    /* --- NASTAVENI TLACITEK --- */
    start_btn.setDebounceTime(30);
    stop_btn.setDebounceTime(30);
    encoder_btn.setDebounceTime(50);

    start_btn = Button2(START_BTN);
    stop_btn = Button2(STOP_BTN);
    encoder_btn = Button2(CONTROL_ENCODER_BTN);

    start_btn.setPressedHandler(buttonStateLoop);
    stop_btn.setPressedHandler(buttonStateLoop);
    encoder_btn.setClickHandler(buttonStateLoop);

    /* --- PINY KLAVESNICE --- */
    pinMode(19, INPUT_PULLUP);
    pinMode(23, INPUT_PULLUP);
    pinMode(18, INPUT_PULLUP);
    pinMode(5, INPUT_PULLUP);

    /* --- PINY OVLADACIHO ENKODERU --- */
    pinMode(32, INPUT_PULLUP);
    pinMode(33, INPUT_PULLUP);
    pinMode(12, INPUT_PULLUP);

    /* --- NASTAVENI OVLADACIHO ENKODERU --- */
    ESP32Encoder::useInternalWeakPullResistors = UP;
    controlEncoder.attachHalfQuad(CONTROL_ENCODER_A, CONTROL_ENCODER_B);
    controlEncoder.clearCount();

    Up = false;
    Down = false;
}

/* --- HANDLER PRO TLACITKA --- */

void buttonStateLoop(Button2 &btn)
{
    /* --- STISKEM START ZAPNU PUMPU --- */

    if (btn == control->start_btn)
    {
        pumpa->pumpEnable();
        Serial.println("tlacitko start stisknuto");
    }
    /* --- STISKEM STOP VYPNU PUMPU --- */
    else if (btn == control->stop_btn)
    {
        pumpa->pumpDisable();
        Serial.println("tlacitko stop stisknuto");
    }

    else if (btn == control->encoder_btn)
    {
        /* --- STISKEM ENKODERU ZOBRAZIM NASTAVENI --- */
        if (disp->activePage == 0)
        {
            disp->activePage = 1;
            enterNextionCommand();
            Serial.print("page ");
            Serial.print(disp->activePage);
            enterNextionCommand();

            disp->initilazePic();
            disp->menu.contentShow();
        }
        /* --- STISKEM ENKODERU V NASTAENI AKTIVUJI FOCUSNUTY PRVEK --- */
        else if (disp->activePage == 1)
        {
            if (disp->switcher == true)
                disp->menu.selectFocused();
            else if (disp->switcher == false)
                disp->menu.deselectFocused();

            disp->switcher = !disp->switcher;
        }
    }
    /*
    else if (manual_btn.isPressed() && !pumpa->isRunning())
    {
            pumpa->setMode(Pump::mode::MANUAL);
    }
    else if (semiautomat_btn.isPressed() && !pumpa->isRunning())
    {
            pumpa->setMode(Pump::mode::SEMIAUTOMAT);
    }
    else if (automat_btn.isPressed() && !pumpa->isRunning())
    {
            pumpa->setMode(Pump::mode::AUTOMAT);
    }
    else if (interval_btn.isPressed() && !pumpa->isRunning())
    {
            pumpa->setMode(Pump::mode::INTERVAL);
    }
    if (start_btn.isReleased())
    {
        //pumpa->runDisable()
        Serial.println("tlacitko start pusteno");
    }
    */
}

/* --- REAKCE NA AKCE ENKODERU --- */
void Control::encoderLoop()
{
    long currentCount = controlEncoder.getCount();
    if (currentCount != 0)
    {
        if (0 > currentCount)
        {
            Down = true;
            controlEncoder.clearCount();
            Serial.println("ENCODER DOWN");
            Serial.print("DOWN =");
            Serial.println(Down);
            disp->loop(Up, Down);
        }
        else if (0 < currentCount)
        {
            Up = true;
            controlEncoder.clearCount();
            Serial.println("ENCODER UP");
            Serial.print("UP =");
            Serial.println(Up);
            disp->loop(Up, Down);
        }
        Up = false;
        Down = false;
    }
}

/* --- KONTROLA STAVU TLACITEK, KLAVESNICE A ENKODERU--- */
void Control::loop()
{
    start_btn.loop();
    stop_btn.loop();
    encoder_btn.loop();

    encoderLoop();
    if ((pumpa->status.Running == false) && (digitalRead(19) == LOW || digitalRead(23) == LOW || digitalRead(18) == LOW || digitalRead(5) == LOW))
    {
        if (digitalRead(19) == LOW)
            pumpa->setMode(Pump::mode::MANUAL);
        else if (digitalRead(23) == LOW)
            pumpa->setMode(Pump::mode::SEMIAUTOMAT);
        else if (digitalRead(18) == LOW)
            pumpa->setMode(Pump::mode::AUTOMAT);
        else if (digitalRead(5) == LOW)
            pumpa->setMode(Pump::mode::INTERVAL);
    }
    else if ((pumpa->status.Running == true) && (digitalRead(19) == LOW || digitalRead(23) == LOW || digitalRead(18) == LOW || digitalRead(5) == LOW))
        disp->dispSetInfo("Pri behu nelze zmenit mod rizeni", false);
}