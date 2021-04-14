#include <Pump/Pump.h>

//======================================
//============ KONSTRUCTOR =============
//======================================

Pump::Pump()
{
    pinMode(settings.Pin, OUTPUT);

    if (!ina219.begin())
        Serial.println("Failed to find INA219 chip");

    ESP32Encoder::useInternalWeakPullResistors = UP;
    pumpEncoder.attachHalfQuad(ENCODER_A, ENCODER_B);
    pumpEncoder.clearCount();
    ina219.setCalibration_32V_2A();

    pinMode(SSR_PIN, OUTPUT);
    analogWrite(SSR_PIN, 255);
    stop();
}
void Pump::setup()
{
    pinMode(settings.Pin, OUTPUT);

    if (!ina219.begin())
        Serial.println("Failed to find INA219 chip");

    ESP32Encoder::useInternalWeakPullResistors = UP;
    pumpEncoder.attachHalfQuad(ENCODER_A, ENCODER_B);
    pumpEncoder.clearCount();
    ina219.setCalibration_32V_2A();
}

/* --- HLAVNI SMYCKA PUMPY --- */
void Pump::loop()
{
    setting(mainVariable.getJSONSettings());
    //settings.MaxSpeed++;
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

/* --- PLNE MANUALNI MOD VYZADUJE DRZENI TL. START --- */
void Pump::runManual()
{
    //Serial.println("Manual Running");
    //analogWrite(settings.Pin, parameters.Speed, parameters.Speed);
    digitalWrite(DRIVER_ENABLE_PIN, GO);
    digitalWrite(DRIVER_DIRECTION_PIN_CW, !parameters.Direction);
    digitalWrite(DRIVER_DIRECTION_PIN_ACW, parameters.Direction);
    Serial.println("Jedu MANUAL");

    status.Running = true;
}

/* --- SEMIMANUALNI MOD ZAPNE DAVKOVANI STIKEM TL. START, VYPNE JEJ STISKEM TL. STOP --- */
void Pump::runSemiManual()
{
    //analogWrite(settings.Pin, parameters.Speed, parameters.Speed);
    digitalWrite(DRIVER_ENABLE_PIN, GO);
    digitalWrite(DRIVER_DIRECTION_PIN_CW, !parameters.Direction);
    digitalWrite(DRIVER_DIRECTION_PIN_ACW, parameters.Direction);
    Serial.println("Jedu SEMIMANUAL");

    status.Running = true;
}

/* --- DAVKOVACI MOD PO STISKU TL. START PRENESE POZADOVANE MNOZSTVI OBJEMU, POTE SE VYPNE --- */
void Pump::runDose()
{
    if (status.Running == false)
        status.WantedRotationCount = parameters.Dose * settings.RotationToMl;
    status.Running = true;
    analogWrite(settings.Pin, parameters.Speed, parameters.Speed);

    if (getRotation() >= status.WantedRotationCount)
        stop();
}

/* --- INTERVALOVY MOD PO STISKU TL. START ZAPOCNE CYKLUS PRENESENI */
/*  POZADOVANEHO OBJEMU, KTERY OPAKUJE PO UPLYNUTI NASTAVENE DOBY --- */
void Pump::runIntervalDose()
{
    //Serial.println("interval Running");
    long cycleNumber = parameters.Dose * settings.RotationToMl;
    for (size_t i = 0; i < cycleNumber; i++)
    {
        /* code */
    }
}

/* --- METODA JENZ ZASTAVI PUMPU --- */
void Pump::stop()
{
    status.Running = false;
    status.Enable = false;
    analogWrite(SSR_PIN, 0);
    digitalWrite(DRIVER_ENABLE_PIN, STOP);
    digitalWrite(DRIVER_DIRECTION_PIN_CW, HIGH);
    digitalWrite(DRIVER_DIRECTION_PIN_CW, LOW);
    Serial.println("Prave ted by mela byt pumpa vypnuta");
}

/* --- VYPNUTI/ZAPNUTI PUMPY --- */
void Pump::pumpEnable()
{
    status.Enable = true;
    analogWrite(SSR_PIN, 255);
}
void Pump::pumpDisable()
{
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
    if (content < 10)
        if (content < settings.MaxSpeed)
            parameters.Speed = content * 10;
    mainVariable.getDisplay().menu.insertValueIntoTheFreakingSetting("Speed", content, String(content * 10));
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

/* --- SMYCKA PRO MERENI PROUDU (POPR DALSICH HODNOT) --- */
void Pump::measurementLoop()
{
    //Serial.println("Measurement Running");

    float current = getCurrent();
    //Serial.println(current);
    /*        
        Serial.print("Proud: ");
        Serial.print(current);
        Serial.println(" [mA]");
        Serial.print("Otacky: ");
        Serial.print(getRotation());
        Serial.println(" [rot]");
        Serial.println();
    */
}

/* --- METODY NACITAJICI HODNOTY ZE SENZORU PROUDU A OTACEK --- */
void Pump::updateSensor()
{
    sensor.shuntvoltage = ina219.getShuntVoltage_mV();
    sensor.busvoltage = ina219.getBusVoltage_V();
    sensor.current_mA = ina219.getCurrent_mA();
    sensor.power_mW = ina219.getPower_mW();
    sensor.loadvoltage = sensor.busvoltage + (sensor.shuntvoltage / 1000);
}
float Pump::getCurrent()
{
    Pump::updateSensor();
    status.ActuallCurrent = sensor.current_mA;
    return sensor.current_mA;
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
