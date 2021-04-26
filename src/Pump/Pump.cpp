#include <Pump/Pump.h>

//======================================
//============ KONSTRUCTOR =============
//======================================

Pump::Pump()
{
    pinMode(SSR_PIN, OUTPUT);

    if (!ina219.begin())
        Serial.println("Failed to find INA219 chip");

    ESP32Encoder::useInternalWeakPullResistors = UP;
    pumpEncoder.attachHalfQuad(ENCODER_A, ENCODER_B);
    pumpEncoder.clearCount();
    ina219.setCalibration_32V_2A();

    ledcSetup(0, 60000, 8);
    ledcAttachPin(DRIVER_ENABLE_PIN, 0);

    stop();
}

/* --- HLAVNI SMYCKA PUMPY --- */
void Pump::loop()
{
    setting(mainVariable.getJSONSettings());

    if (mainVariable.getPump().status.Enable == true)
        run();
    else
        stop();
}

/* --- PREPINANI MEZI MODY --- */
void Pump::run()
{
    switch (parameters.Mode)
    {
    case Parameters::MANUAL:
        runManual();
        break;
    case Parameters::SEMIAUTOMAT:
        runSemiManual();
        break;
    case Parameters::AUTOMAT:
        runDose();
        break;
    case Parameters::INTERVAL:
        runIntervalDose();
        break;
    default:
        stop();
        break;
    }
}

/* --- ALGORITMUS RAMPY --- */
void Pump::rampSpeedAdjustment()
{
    if (isRunning() == false)
    {
        if (parameters.RampTime > 0)
            speedIncrement = parameters.Speed / (parameters.RampTime * 10);
        else
            currentDuty = map(parameters.Speed, 0, 100, MIN_DUTY, MAX_DUTY);
        lastSpeedIncrementTimeMark = millis();
    }
    else if ((millis() - lastSpeedIncrementTimeMark > 100) && (currentDuty < (map(parameters.Speed, 0, 100, MIN_DUTY, MAX_DUTY))))
    {
        currentSpeed += speedIncrement;
        currentDuty = map(currentSpeed, 0, 100, MIN_DUTY, MAX_DUTY);
        lastSpeedIncrementTimeMark = millis();
    }
}
/* --- PLNE MANUALNI MOD VYZADUJE DRZENI TL. START --- */
void Pump::runManual()
{
    rampSpeedAdjustment();
    ledcWrite(0, currentDuty);

    digitalWrite(DRIVER_DIRECTION_PIN_CW, !parameters.Direction);
    digitalWrite(DRIVER_DIRECTION_PIN_ACW, parameters.Direction);

    status.Running = true;
}

/* --- SEMIMANUALNI MOD ZAPNE DAVKOVANI STIKEM TL. START, VYPNE JEJ STISKEM TL. STOP --- */
void Pump::runSemiManual()
{
    rampSpeedAdjustment();
    ledcWrite(0, currentDuty);

    digitalWrite(DRIVER_DIRECTION_PIN_CW, !parameters.Direction);
    digitalWrite(DRIVER_DIRECTION_PIN_ACW, parameters.Direction);

    status.Running = true;
}

/* --- DAVKOVACI MOD PO STISKU TL. START PRENESE POZADOVANE MNOZSTVI OBJEMU, POTE SE VYPNE --- */
void Pump::runDose()
{
    if (status.Running == false)
        status.WantedRotationCount = parameters.Dose * settings.RotationToMl;

    if (abs(getRotation()) >= status.WantedRotationCount)
        stop();
    else
    {
        rampSpeedAdjustment();
        ledcWrite(0, currentDuty);

        digitalWrite(DRIVER_DIRECTION_PIN_CW, !parameters.Direction);
        digitalWrite(DRIVER_DIRECTION_PIN_ACW, parameters.Direction);
        status.Running = true;
    }
}

/* --- INTERVALOVY MOD PO STISKU TL. START ZAPOCNE CYKLUS PRENESENI */
/*  POZADOVANEHO OBJEMU, KTERY OPAKUJE PO UPLYNUTI NASTAVENE DOBY --- */
void Pump::runIntervalDose()
{
    if (currentDuty == 0)
    {
        status.WantedRotationCount = parameters.Dose * settings.RotationToMl;
        lastInterval = 0;
    }

    if (millis() > (lastInterval + (parameters.Interval * 1000)))
    {
        if (abs(getRotation()) >= (parameters.Dose * settings.RotationToMl))
        {
            resetRotation();
            lastInterval = millis();
            status.Running = false;
            currentSpeed = 0;
            currentDuty = 1;
        }
        else
        {
            rampSpeedAdjustment();
            ledcWrite(0, currentDuty);

            digitalWrite(DRIVER_DIRECTION_PIN_CW, !parameters.Direction);
            digitalWrite(DRIVER_DIRECTION_PIN_ACW, parameters.Direction);
            status.Running = true;
        }
    }
}

/* --- METODA JENZ ZASTAVI PUMPU --- */
void Pump::stop()
{
    status.Running = false;
    status.Enable = false;
    currentSpeed = 0;
    currentDuty = 0;
    digitalWrite(DRIVER_ENABLE_PIN, STOP);

    resetRotation();
    //ledcWrite(0, 256);

    //Serial.println("Prave ted by mela byt pumpa vypnuta");
}

/* --- VYPNUTI/ZAPNUTI PUMPY --- */
void Pump::pumpEnable()
{
    digitalWrite(SSR_PIN, GO);
    status.Enable = true;
}
void Pump::pumpDisable()
{
    digitalWrite(SSR_PIN, STOP);
    stop();
    status.Enable = false;
}

/* --- NAPLNENI TRUBEK --- */
void Pump::fillTubes()
{
    if (status.filledTubes == false)
    {
        int tubeVolume = 3.14159 * settings.tubeLenght * settings.tubeLenght * settings.tubeLenght;
        setDose(tubeVolume);
        setMode(2);
        pumpEnable();
        status.filledTubes = true;
    }
    else
        mainVariable.getDisplay().dispSetInfo("Tubes are already \n filled", true);
}

/* --- METODA PARSUJICI JSON GLOBALNIHO NASTAVENI A JEHO APLIKOVANI DO PROGRAMU V PRIPADE ZMENY --- */
String lastSettings = "";
void Pump::setting(String settings)
{
    if (settings[0] == '[' && settings != lastSettings)
    {
        Serial.println("This is form PUMP SETTINGS");
        Serial.println(settings);
        lastSettings = settings;
        DynamicJsonDocument incomeMessage(1024);
        deserializeJson(incomeMessage, settings);

        for (size_t i = 0; i < 6; i++)
        {
            const char *name = incomeMessage[i]["Name"];
            int val = incomeMessage[i]["Value"];
            Parameters mod;
            if (strcmp(name, "Mode") == 0)
            {
                setMode(val);
            }
            if (strcmp(name, "Dose") == 0)
            {
                setDose(val);
            }
            else if (strcmp(name, "Direction") == 0)
            {
                setDirection(val);
            }
            else if (strcmp(name, "Speed") == 0)
            {
                setSpeed(val);
            }
            if (strcmp(name, "Interval") == 0)
            {
                setInterval(val);
            }
            if (strcmp(name, "Ramp") == 0)
            {
                setRamp(val);
            }
            else
            {
            }
        }
    }
}

/* --- API ZAJISTUJICI VLOZENI NASTAVENI DO NASTAVENI A NA DISPLEJ --- */
void Pump::setDose(unsigned short content)
{
    parameters.Dose = content;
    mainVariable.getDisplay().menu.insertValueIntoTheFreakingSetting("Dose", content, String(content));
}

void Pump::setSpeed(unsigned short content)
{
    if (content > 0)
    {
        if (content < 10)
        {
            if (content < 1)
                parameters.Speed = 10;
            else
                parameters.Speed = content * 10;
        }
        else
        {
            parameters.Speed = 100;
        }
    }

    mainVariable.getDisplay().menu.insertValueIntoTheFreakingSetting("Speed", content, String(parameters.Speed));
}

void Pump::setDirection(bool content)
{
    parameters.Direction = content;
    //digitalWrite(0, param.Direction);
    String pom = "";

    if (content)
        pom = "CW";
    else
        pom = "ACW";
    mainVariable.getDisplay().menu.insertValueIntoTheFreakingSetting("Direction", content, pom);
}
void Pump::setInterval(unsigned long content)
{
    parameters.Interval = content;
    mainVariable.getDisplay().menu.insertValueIntoTheFreakingSetting("Interval", content, String(content));
}
void Pump::setRamp(unsigned short content)
{
    parameters.RampTime = content;
    mainVariable.getDisplay().menu.insertValueIntoTheFreakingSetting("Ramp", content, String(content));
}
void Pump::setMode(unsigned int val)
{
    char *pom = "";
    if (val == 0)
    {
        pom = "MANUAL";
        parameters.Mode = Parameters::MANUAL;
    }

    else if (val == 1)
    {
        pom = "SEMIAUTOMAT";
        parameters.Mode = Parameters::SEMIAUTOMAT;
    }

    else if (val == 2)
    {
        pom = "AUTOMAT";
        parameters.Mode = Parameters::AUTOMAT;
    }

    else if (val == 3)
    {
        pom = "INTERVAL";
        parameters.Mode = Parameters::INTERVAL;
    }

    else
    {
        pom = "MANUAL";
        parameters.Mode = Parameters::MANUAL;
    }

    mainVariable.getDisplay().dispSetMode(pom);
    String subPom = String(pom).substring(0, 6);
    mainVariable.getDisplay().menu.insertValueIntoTheFreakingSetting("Mode", val, subPom);
}

void Pump::setTubeLenght(unsigned short content)
{
    settings.tubeLenght = content;
    mainVariable.getDisplay().menu.insertValueIntoTheFreakingSetting("Tube lenght", content, String(content));
}
void Pump::setTubeDiameter(unsigned short content)
{
    settings.tubeDiameter = content;
    mainVariable.getDisplay().menu.insertValueIntoTheFreakingSetting("Tube diameter", content, String(content));
}
void Pump::setMaxCurrent(unsigned short content)
{
    settings.MaxCurrent = content;
}

unsigned short Pump::getMaxCurrent()
{
    return settings.MaxCurrent;
}
void Pump::setMaxSpeed(unsigned short content)
{
    settings.MaxSpeed = content;
}
unsigned short Pump::getMaxSpeed()
{
    return settings.MaxSpeed;
}

bool Pump::isRunning()
{
    return status.Running;
}


/* --- METODY NACITAJICI HODNOTY ZE SENZORU PROUDU A OTACEK --- */
void Pump::updateSensor()
{
    sensor.shuntvoltage = ina219.getShuntVoltage_mV();
    sensor.busvoltage = ina219.getBusVoltage_V();
    sensor.rawCurrent_mA = ina219.getCurrent_mA();
    sensor.power_mW = ina219.getPower_mW();
    sensor.loadvoltage = sensor.busvoltage + (sensor.shuntvoltage / 1000);
}
float Pump::getCurrent()
{
    Pump::updateSensor();
    status.ActuallRawCurrent = sensor.rawCurrent_mA;
    sensor.rawCurrentArray[COUNT_OF_SAMPLES - 1] = sensor.rawCurrent_mA;
    float avrageOfCurrent = 0;

    for (size_t i = 0; i < COUNT_OF_SAMPLES; i++)
        avrageOfCurrent += sensor.rawCurrentArray[i];
    for (size_t i = 0; i < COUNT_OF_SAMPLES - 1; i++)
        sensor.rawCurrentArray[i] = sensor.rawCurrentArray[i + 1];

    sensor.smoothCurrent_mA = avrageOfCurrent / COUNT_OF_SAMPLES;

    //Serial.print("RAW Current: ");
    Serial.print(ina219.getCurrent_mA()); //raw current
    Serial.print(",");

    /*Serial.print("RAW Array: [");
    for (size_t i = 0; i < COUNT_OF_SAMPLES - 1; i++)
    {
        Serial.print(sensor.rawCurrentArray[i]);
        Serial.print("");
    }
    Serial.println("]");*/

    //Serial.print("Smooth Current: ");
    /* Serial.println(sensor.smoothCurrent_mA); //smooth current
    long currentToSend = map(sensor.smoothCurrent_mA, 0, 600, 0, 75);
    enterNextionCommand();
    Serial.print("add 17,0,");
    Serial.print(currentToSend);
    enterNextionCommand();
    Serial.print("add 17,1,");
    Serial.print(currentToSend - 1);
    enterNextionCommand();
    Serial.print("add 17,2,");
    Serial.print(currentToSend - 2);
    enterNextionCommand();
*/
    return sensor.smoothCurrent_mA;
}
long Pump::getRotation()
{
    status.ActuallRotationCount = pumpEncoder.getCount();
    return status.ActuallRotationCount;
}
long Pump::getMl()
{
    return getRotation() * settings.RotationToMl;
}
void Pump::resetRotation()
{
    status.ActuallRotationCount = 0;
    pumpEncoder.clearCount();
}
