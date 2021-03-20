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
    setting(Mover.JsonSettings);
    if (pumpa->status.Enable == true)
        run();
    else
        stop();
}

/* --- PREPINANI MEZI MODY --- */
void Pump::run()
{
    switch (parameters.Mode)
    {
    case MANUAL:
        runManual();
        break;
    case SEMIAUTOMAT:
        runSemiManual();
        break;
    case AUTOMAT:
        runDose();
        break;
    case INTERVAL:
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
    analogWrite(settings.Pin, parameters.Speed, parameters.Speed);
    status.Running = true;
}

/* --- SEMIMANUALNI MOD ZAPNE DAVKOVANI STIKEM TL. START, VYPNE JEJ STISKEM TL. STOP --- */
void Pump::runSemiManual()
{
    analogWrite(settings.Pin, parameters.Speed, parameters.Speed);
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
    analogWrite(settings.Pin, 0, parameters.Speed);
}

/* --- VYPNUTI/ZAPNUTI PUMPY --- */
void Pump::pumpEnable()
{
    status.Enable = true;
}
void Pump::pumpDisable()
{
    status.Enable = false;
    stop();
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

            if (strcmp(name, "Mode") == 0)
            {
                Pump::mode mod;
                if (val == 0)
                    mod = MANUAL;
                else if (val == 1)
                    mod = SEMIAUTOMAT;
                else if (val == 2)
                    mod = AUTOMAT;
                else if (val == 3)
                    mod = INTERVAL;
                else
                    mod = MANUAL;
                setMode(mod);
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
    disp->menu.insertValueIntoTheFreakingSetting("Dose", content);
}

void Pump::setSpeed(unsigned short content)
{
    if (content < settings.MaxSpeed)
        parameters.Speed = content;
    disp->menu.insertValueIntoTheFreakingSetting("Speed", content);
}

void Pump::setDirection(bool content)
{
    parameters.Direction = content;
    //digitalWrite(0, param.Direction);
    disp->menu.insertValueIntoTheFreakingSetting("Direction", content);
}
void Pump::setInterval(unsigned long content)
{
    parameters.Interval = content;
    disp->menu.insertValueIntoTheFreakingSetting("Interval", content);
}
void Pump::setRamp(unsigned short content)
{
    parameters.RampTime = content;
    disp->menu.insertValueIntoTheFreakingSetting("Ramp", content);
}
void Pump::setMode(Pump::mode content)
{
    parameters.Mode = content;

    if (content == MANUAL)
        disp->dispSetMode("MANUAL");
    else if (content == SEMIAUTOMAT)
        disp->dispSetMode("SEMIAUTOMAT");
    else if (content == AUTOMAT)
        disp->dispSetMode("AUTOMAT");
    else if (content == INTERVAL)
        disp->dispSetMode("INTERVAL");
    else
        disp->dispSetMode("MANUAL");
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

    //float current = getCurrent();
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
