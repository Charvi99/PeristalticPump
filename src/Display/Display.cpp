#include <Display/Display.hpp>

/* --- KONSTRUKTOR DISPLEJE --- */
Display::Display()
{
    myNex.setSerial(Serial);
    myNex.begin(115200);
    activePage = 0;
    switcher = true;
    enterNextionCommand();
    Serial.print("page 0");
    enterNextionCommand();

    initilazePic();
    menu.contentShow(activePage);
    dispSetMl(0);
}
/* --- HLAVNI SMYCKA PRO DISPLEJ --- */
void Display::loop(bool up, bool down)
{
    //myNex.NextionListen();

    if (activePage == 1)
    {
        if (up == true && down == false)
        {
            if (menu.searchForSelected() == -1) //nic neni vybrano -> posun v menu
                menu.focusDown();
            else ///neco je vybrano -> zmena hodnoty
                menu.IncreseVal();
            lastInteractionTimeMark = millis();
        }
        else if (down == true && up == false)
        {
            if (menu.searchForSelected() == -1) //nic neni vybrano -> posun v menu
                menu.focusUp();
            else ///neco je vybrano -> zmena hodnoty
                menu.DecreseVal();
            lastInteractionTimeMark = millis();
        }
        if ((millis() - lastInteractionTimeMark > 10000))
        {
            for (size_t i = 0; i < 4; i++)
                menu.Items[i].Selected = false;
            activePage = 0;
            setPage(0);
            menu.contentShow(0);
        }
    }
    else
    {
        // if (mainVariable.getPump().isRunning() == true)
        //{
        dispSetMl(mainVariable.getPump().getMl());
        // }
        float actualCurrent = mainVariable.getPump().getCurrent();
        dispGraph(map(actualCurrent, 0, 600, 2, 75));
        //VAZNE MERIM PROUD TADY???
        if (actualCurrent > 370)
        {
            mainVariable.getPump().pumpDisable();
            mainVariable.alarmStatus = 3;
            mainVariable.getDisplay().dispSetInfo("HIGH CURRENT CHECK  TUBES", true);
        }
        dispRun(mainVariable.getPump().isRunning());
    }
}

/* --- ODESLANI DAT NA DISPLEJ --- */
void Display::dispSetMode(char *content)
{
    myNex.writeStr("t0.txt", content);
}
void Display::dispSetDose(char *content)
{
    //myNex.writeStr("n0.val",content);
}
void Display::dispSetDirection(char *content)
{
    //myNex.writeStr("n0.val",content);
}
void Display::dispSetSpeed(char *content)
{
    //myNex.writeStr("n0.val",content);
}
void Display::dispSetInterval(char *content)
{
    //myNex.writeStr("n0.val",content);
}
void Display::dispSetRamp(char *content)
{
    //myNex.writeStr("n0.val",content);
}
void Display::dispSetInfo(char *content, bool warning)
{
    enterNextionCommand();
    if (warning == false)
    {
        myNex.writeNum("info_p.pic", 12);
        myNex.writeNum("info_t.xcen", 0);
        myNex.writeNum("info_t.bco", UNFOCUSED_NAME_BC);
        myNex.writeNum("info_t.pco", UNFOCUSED_NAME_PC);
        myNex.writeStr("info_t.txt", content);
    }
    else
    {
        myNex.writeNum("info_p.pic", 13);
        myNex.writeNum("info_t.xcen", 1);
        myNex.writeNum("info_t.bco", FOCUSED_NAME_BC);
        myNex.writeNum("info_t.pco", FOCUSED_NAME_PC);
        myNex.writeStr("info_t.txt", content);
        mainVariable.alarmStatus = (int)Alarm::AlarmStatus::WARNING;
    }
    currentInfo = content;
    mainVariable.getAlert() = content;
    mainVariable.getAlertPriority() = warning;
    mainVariable.getLastAlerUpdate() = millis();
}
void Menu::dispSetInfo(char *content, bool warning)
{
    enterNextionCommand();
    if (warning == false)
    {
        myNex.writeNum("info_p.pic", 12);
        myNex.writeNum("info_t.xcen", 0);
        myNex.writeNum("info_t.bco", UNFOCUSED_NAME_BC);
        myNex.writeNum("info_t.pco", UNFOCUSED_NAME_PC);
        myNex.writeStr("info_t.txt", content);
    }
    else
    {
        myNex.writeNum("info_p.pic", 13);
        myNex.writeNum("info_t.xcen", 1);
        myNex.writeNum("info_t.bco", FOCUSED_NAME_BC);
        myNex.writeNum("info_t.pco", FOCUSED_NAME_PC);
        myNex.writeStr("info_t.txt", content);
        mainVariable.alarmStatus = (int)Alarm::AlarmStatus::WARNING;
    }
    mainVariable.getAlert() = content;
    mainVariable.getAlertPriority() = INFO_STATUS_IDLE;
    mainVariable.getLastAlerUpdate() = millis();
}

void Display::dispSetMl(long content)
{
    myNex.writeNum("n0.val", content);
    if (content < 999)
        myNex.writeStr("t4.txt", "mL");
    else if (content > 999)
        myNex.writeStr("t4.txt", "L");
}
void Display::dispGraph(int content)
{

    enterNextionCommand();
    myNex.writeNum("graphVal.val", content);
    enterNextionCommand();

    if (mainVariable.getMQTT().client.connected() == true)
    {
        DynamicJsonDocument dataToSend(256);

        dataToSend[0]["Name"] = "Current";
        dataToSend[0]["Value"] = content;
        dataToSend[0]["Unit"] = "mA";

        dataToSend[1]["Name"] = "Flow";
        dataToSend[1]["Value"] = mainVariable.getPump().getMl();
        if (abs(mainVariable.getPump().getMl()) < 1000)
            dataToSend[1]["Unit"] = "mL";
        else
            dataToSend[1]["Unit"] = "L";

        String dataToSendString = "";
        serializeJson(dataToSend, dataToSendString);
        mainVariable.getMQTT().publish("peristaltic/data", dataToSendString.c_str());
    }

    //PO PREHRATI FIRMAVARU BY SE O ZAPISOVANI STARAL INTERNI TIMER DISPLEJE A HODNOTU BYCH VKLADAL DO PROMENNE
    /* myNex.writeNum("tm0.en",1);
     myNex.writeNum("tm1.en",1);
     myNex.writeNum("tm2.en",1);
     myNex.writeNum("tm3.en",1);
    myNex.writeNum("graphVal.val", content);*/
}
void Display::dispRun(bool content)
{
    if (content)
    {
        if (millis() - lastIconUpdateTimeMark > 300)
        {
            if (mainVariable.getPump().parameters.Direction)
            {
                if (switcher2 == true)
                    myNex.writeNum("p1.pic", 1);
                else
                    myNex.writeNum("p1.pic", 2);
            }
            else
            {
                if (switcher2 == true)
                    myNex.writeNum("p1.pic", 14);
                else
                    myNex.writeNum("p1.pic", 15);
            }
            switcher2 = !switcher2;
            lastIconUpdateTimeMark = millis();
        }

        //myNex.writeNum("directionVal.val", mainVariable.getPump().parameters.Direction);
    }
    else
    {
        if (mainVariable.getPump().parameters.Direction)
            myNex.writeNum("p1.pic", 1);
        else
            myNex.writeNum("p1.pic", 14);
    }
}

/* --- KONSTRUKTOR PRVKU MENU --- */
MenuItem::MenuItem()
{
    Name = "Name";
    Value = "Value";
    Unit = "Unit";
    Focused = false;
    Selected = false;
}

/* --- KONSTRUKTOR MENU --- */
Menu::Menu()
{
    Items[0].Focused = true;
    settings[0].Name = "<- Back";
    settings[0].Value = "";
    settings[0].Unit = "";

    /* --- MANUÁALNI VKLADANI PRVKU DO DISPLEJE --- */
    settings[1] = {"Mode", "MANUAL", 0, ""};
    settings[2] = {"Dose", "50", 50, "ml"};
    settings[3] = {"Direction", "CW", 0, ""};
    settings[4] = {"Speed", "100", 10, "%"};
    settings[5] = {"Interval", "5", 5, "s"};
    settings[6] = {"Ramp", "1", 1, "s"};
    settings[7] = {"Tube lenght", "30", 30, "mm"};
    settings[8] = {"Tube diameter", "3", 3, "mm"};
    settings[9] = {"MQTT", "ON", 1, ""};

    for (size_t i = 0; i < 4; i++)
        Items[i].storedItem = i;

    focusedIndex = 0;
    myNex.setSerial(Serial);
    myNex.begin(115200);
    myNex.writeNum("p_" + String(focusedIndex) + ".pic", UNFOCUSED);

    contentShow(1);
}

/* --- POSUN FOCUSU NA PRVEK O INDEX NIZ --- */
void Menu::focusUp()
{
    enterNextionCommand();
    if (focusedIndex > 0)
    {
        myNex.writeNum("p_" + String(focusedIndex) + ".pic", UNFOCUSED);
        myNex.writeNum("p_" + String(--focusedIndex) + ".pic", FOCUSED);
        contentShow(1);
    }
    else
    {
        myNex.writeNum("p_0.pic", FOCUSED);
        contentUp();
    }
    Serial.println("UP");
    Serial.println("UP");
}

/* --- POSUN FOCUSU NA PRVEK O INDEX VYS --- */
void Menu::focusDown()
{
    enterNextionCommand();
    if (focusedIndex < 3)
    {
        myNex.writeNum("p_" + String(focusedIndex) + ".pic", UNFOCUSED);
        myNex.writeNum("p_" + String(++focusedIndex) + ".pic", FOCUSED);
        contentShow(1);
    }
    else
    {
        myNex.writeNum("p_3.pic", FOCUSED);
        contentDown();
    }
    Serial.println("DOWN");
    Serial.println("DOWN");
}

/* --- VYHLEDANI FOCUSNUTEHO PRVKU --- */
int Menu::searchForFocused()
{
    for (size_t i = 0; i < 4; i++)
        if (Items[i].Focused == true)
            return i;
    return -1;
}

/* --- VYHLEDANI VYBRANEHO PRVKU --- */
int Menu::searchForSelected()
{
    for (size_t i = 0; i < 4; i++)
        if (Items[i].Selected == true)
            return i;
    return -1;
}

/* --- ZMENA VZHLEDU PRVKU POKUD JE FOCUSNUTY --- */
void Menu::selectFocused()
{
    enterNextionCommand();

    /* --- POKUD JE VYBRAN PRVEK "<- Back" PREPNE SE STRANA --- */
    if (settings[Items[focusedIndex].storedItem].Name == "<- Back")
    {
        mainVariable.getDisplay().activePage = 0;
        mainVariable.getDisplay().setPage(0);
    }
    else
    {
        Items[focusedIndex].Selected = true;
        myNex.writeNum("p_" + String(focusedIndex) + ".pic", SELECTED);
        myNex.writeNum("name_" + String(focusedIndex) + ".bco", FOCUSED_NAME_BC);
        myNex.writeNum("name_" + String(focusedIndex) + ".pco", FOCUSED_NAME_PC);
        myNex.writeNum("val_" + String(focusedIndex) + ".bco", FOCUSED_UNIT_BC);
        myNex.writeNum("val_" + String(focusedIndex) + ".pco", FOCUSED_UNIT_PC);
        myNex.writeNum("unit_" + String(focusedIndex) + ".bco", FOCUSED_UNIT_BC);
        myNex.writeNum("unit_" + String(focusedIndex) + ".pco", FOCUSED_UNIT_PC);

        contentShow(1);
    }

    Serial.println("SELECTED");
    Serial.println("SELECTED");
}

/* --- ZMENA VZHLEDU PRVKU PRI VRACENI DO NORMALU --- */
void Menu::deselectFocused()
{
    enterNextionCommand();

    Items[focusedIndex].Selected = false;
    myNex.writeNum("p_" + String(focusedIndex) + ".pic", FOCUSED);
    myNex.writeNum("name_" + String(focusedIndex) + ".bco", UNFOCUSED_NAME_BC);
    myNex.writeNum("name_" + String(focusedIndex) + ".pco", UNFOCUSED_NAME_PC);
    myNex.writeNum("val_" + String(focusedIndex) + ".bco", UNFOCUSED_UNIT_BC);
    myNex.writeNum("val_" + String(focusedIndex) + ".pco", UNFOCUSED_UNIT_PC);
    myNex.writeNum("unit_" + String(focusedIndex) + ".bco", UNFOCUSED_UNIT_BC);
    myNex.writeNum("unit_" + String(focusedIndex) + ".pco", UNFOCUSED_UNIT_PC);
    contentShow(1);

    Serial.println("DESELECTED");
    Serial.println("DESELECTED");

    /* --- APLIKOVANI ZMEN NA MQTT --- */

    if (settings[9].NumValue == 1)
    {
        if (mainVariable.getMQTT().client.connected() == false)
            mainVariable.getMQTT().MQTTbegin();
    }
    else if (settings[9].NumValue == 0)
    {
        if (mainVariable.getMQTT().client.connected() == true)
            mainVariable.getMQTT().client.disconnect();
    }

    /* --- ODESLANI NOVE NASTAVENYCH DAT PRES MQTT --- */

    if (mainVariable.getMQTT().client.connected())
    {
        DynamicJsonDocument settingsToSend(1024);
        for (size_t i = 1; i < SETTINGS_COUNT; i++)
        {
            settingsToSend[i - 1]["Name"] = settings[i].Name;
            settingsToSend[i - 1]["Value"] = settings[i].NumValue;
            settingsToSend[i - 1]["Unit"] = settings[i].Unit;
        }
        String settingsToSendString = "";
        serializeJson(settingsToSend, settingsToSendString);
        mainVariable.getMQTT().publish("peristaltic/settings", settingsToSendString.c_str());
    }
}

/* --- NAPLNENI TEXTOVYCH OKEN ODPOVIDAJICIM TEXTEM --- */
void Menu::contentShow(int page)
{
    if (page == 0)
    {
        enterNextionCommand();
        myNex.writeStr("t0.txt", settings[1].Value);
        String infoText = "";

        if (settings[1].NumValue == 1)
        {
            infoText += settings[4].Name + ": " + settings[4].Value + " [" + settings[4].Unit + "]" + "\r" + "\n";
            infoText += settings[6].Name + ": " + settings[6].Value + " [" + settings[6].Unit + "]" + "\r" + "\n";
            infoText += settings[9].Name + ": " + settings[9].Value + " [-]" + "\r" + "\n";
        }
        else if (settings[1].NumValue == 2)
        {
            infoText += settings[2].Name + ": " + settings[2].Value + " [" + settings[2].Unit + "]" + "\r" + "\n";
            infoText += settings[6].Name + ": " + settings[6].Value + " [" + settings[6].Unit + "]" + "\r" + "\n";
            infoText += settings[4].Name + ": " + settings[4].Value + " [" + settings[4].Unit + "]" + "\r" + "\n";
            infoText += settings[9].Name + ": " + settings[9].Value + " [-]" + "\r" + "\n";
        }
        else if (settings[1].NumValue == 3)
        {
            infoText += settings[4].Name + ": " + settings[4].Value + " [" + settings[4].Unit + "]" + "\r" + "\n";
            infoText += settings[2].Name + ": " + settings[2].Value + " [" + settings[2].Unit + "]" + "\r" + "\n";
            infoText += settings[5].Name + ": " + settings[5].Value + " [" + settings[5].Unit + "]" + "\r" + "\n";
            infoText += settings[6].Name + ": " + settings[6].Value + " [" + settings[6].Unit + "]" + "\r" + "\n";
            infoText += settings[9].Name + ": " + settings[9].Value + " [-]" + "\r" + "\n";
        }
        else
        {
            infoText += settings[4].Name + ": \r" + settings[4].Value + " \r[" + settings[4].Unit + "]" + "\r" + "\r" + "\r" + "\n";
            infoText += settings[6].Name + ": \r\r" + settings[6].Value + " \r\r\r\r\r[" + settings[6].Unit + "]" + "\r" + "\n";
            infoText += settings[9].Name + ": \r" + settings[9].Value + " \r[-]" + "\r" + "\n";
        }
        int n = infoText.length();
        char infoText_array[n + 1];
        strcpy(infoText_array, infoText.c_str());
        dispSetInfo(infoText_array, false);
    }
    else if (page == 1)
    {
        for (size_t i = 0; i < 4; i++)
        {
            displayContentInItem(i, Items[i].storedItem);
        }
    }
}
void Menu::newLine()
{
    Serial.write(0x0d);
    Serial.write(0x0a);
}

/* --- POSUN TEXTU PRI POSUNU PRVKU --- */
void Menu::contentUp()
{
    if (Items[0].storedItem > 0)
        for (size_t i = 0; i < 4; i++)
            displayContentInItem(i, Items[i].storedItem - 1);
    else
        contentShow(1);
}
void Menu::contentDown()
{
    if (Items[3].storedItem < SETTINGS_COUNT - 1)
        for (size_t i = 0; i < 4; i++)
            displayContentInItem(i, Items[i].storedItem + 1);
    else
        contentShow(1);
}

/* --- NAPLNI TEXTEM KONKRETNI PRVEK--- */
void Menu::displayContentInItem(int item, int content)
{
    enterNextionCommand();

    String itemName = "name_" + String(item) + ".txt";
    myNex.writeStr(itemName, settings[content].Name);

    if (settings[content].Name == "Mode")
    {
        itemName = "val_" + String(item) + ".font";
        myNex.writeNum(itemName, 0);

        itemName = "val_" + String(item) + ".txt";
        myNex.writeStr(itemName, settings[content].Value);
    }
    else
    {
        itemName = "val_" + String(item) + ".font";
        myNex.writeNum(itemName, 4);

        itemName = "val_" + String(item) + ".txt";
        myNex.writeStr(itemName, settings[content].Value);
    }

    itemName = "unit_" + String(item) + ".txt";
    myNex.writeStr(itemName, settings[content].Unit);

    Items[item].storedItem = content;
}

/* --- ROZBRAZENI ZVYSENE HODNOTY --- */
void Menu::IncreseVal()
{
    unsigned int index = Items[searchForSelected()].storedItem;
    if (settings[index].Name == "Mode")
    {
        if (settings[index].NumValue > 4)
            settings[index].NumValue = 1;

        mainVariable.getPump().setMode((++settings[index].NumValue));
    }
    else if (settings[index].Name == "Dose")
    {
        settings[index].NumValue += 10;
        mainVariable.getPump().setDose(settings[index].NumValue);
    }
    else if (settings[index].Name == "Direction")
    {

        if ((settings[index].NumValue) == 0)
        {
            settings[index].NumValue = 1;
            mainVariable.getPump().setDirection(true);
        }
        else
        {
            settings[index].NumValue = 0;
            mainVariable.getPump().setDirection(false);
        }
    }
    else if (settings[index].Name == "Speed")
    {
        if (settings[index].NumValue < 10)
            mainVariable.getPump().setSpeed(++settings[index].NumValue);
    }
    else if (settings[index].Name == "Interval")
    {
        mainVariable.getPump().setInterval(++settings[index].NumValue);
    }
    else if (settings[index].Name == "Ramp")
    {
        mainVariable.getPump().setRamp(++settings[index].NumValue);
    }
    else if (settings[index].Name == "Tube lenght")
    {
        mainVariable.getPump().setTubeLenght(++settings[index].NumValue);
    }
    else if (settings[index].Name == "Tube diameter")
    {
        mainVariable.getPump().setTubeDiameter(++settings[index].NumValue);
    }
    else if (settings[index].Name == "MQTT")
    {
        if (settings[index].NumValue == 0)
        {
            settings[index].NumValue = 1;
            insertValueIntoTheFreakingSetting(settings[index].Name, 1, "ON");
        }
        else
        {
            settings[index].NumValue = 0;
            insertValueIntoTheFreakingSetting(settings[index].Name, 0, "OFF");
        }
    }
    contentShow(1);
}

/* --- ROZBRAZENI SNIZENE HODNOTY --- */
void Menu::DecreseVal()
{
    unsigned int index = Items[searchForSelected()].storedItem;
    if ((settings[index].NumValue - 1) >= 0)
        if (settings[index].Name == "Mode")
        {
            if (settings[index].NumValue <= 1)
                settings[index].NumValue = 4;

            mainVariable.getPump().setMode((--settings[index].NumValue));
        }
        else if (settings[index].Name == "Dose")
        {
            if (settings[index].NumValue > 10)
                settings[index].NumValue -= 10;
            mainVariable.getPump().setDose(settings[index].NumValue);
            insertValueIntoTheFreakingSetting(settings[index].Name, settings[index].NumValue, String(settings[index].NumValue));
        }
        else if (settings[index].Name == "Direction")
        {
            if ((settings[index].NumValue) == 0)
            {
                settings[index].NumValue = 1;
                mainVariable.getPump().setDirection(true);
            }
            else
            {
                settings[index].NumValue = 0;
                mainVariable.getPump().setDirection(false);
            }
        }
        else if (settings[index].Name == "Speed")
        {
            if (settings[index].NumValue > 2)
                mainVariable.getPump().setSpeed(--settings[index].NumValue);
        }
        else if (settings[index].Name == "Interval")
        {
            if (settings[index].NumValue > 2)
                mainVariable.getPump().setInterval(--settings[index].NumValue);
        }
        else if (settings[index].Name == "Ramp")
        {
            if (settings[index].NumValue > 1)
                mainVariable.getPump().setRamp(--settings[index].NumValue);
        }
        else if (settings[index].Name == "Tube lenght")
        {
            if (settings[index].NumValue > 1)
                mainVariable.getPump().setTubeLenght(--settings[index].NumValue);
        }
        else if (settings[index].Name == "Tube diameter")
        {
            if (settings[index].NumValue > 1)
                mainVariable.getPump().setTubeDiameter(--settings[index].NumValue);
        }
        else if (settings[index].Name == "MQTT")
        {
            if (settings[index].NumValue == 0)
            {
                settings[index].NumValue = 1;
                insertValueIntoTheFreakingSetting(settings[index].Name, 1, "ON");
            }
            else
            {
                settings[index].NumValue = 0;
                insertValueIntoTheFreakingSetting(settings[index].Name, 0, "OFF");
            }
        }
    contentShow(1);
}

/* --- ZVYSENA HODNOTA SE ULOZI DO GLOBALNIHO NASTAVENI --- */
void Menu::insertValueIntoTheFreakingSetting(String itemName, int numValue, String value)
{
    for (size_t i = 0; i < SETTINGS_COUNT; i++)
        if (itemName == settings[i].Name)
        {
            settings[i].NumValue = numValue;
            settings[i].Value = value;
        }
    contentShow(1);
}

/* --- ZOBRAZENI POZADI VSECH PRVKU MENU --- */
void Display::initilazePic()
{
    enterNextionCommand();
    Serial.print("p_0.pic=9");
    enterNextionCommand();
    Serial.print("p_0.pic=9");
    enterNextionCommand();
    Serial.print("p_1.pic=9");
    enterNextionCommand();
    Serial.print("p_2.pic=9");
    enterNextionCommand();
    Serial.print("p_3.pic=9");
    enterNextionCommand();
}

void Display::setPage(int page)
{
    if (page == 0)
    {
        enterNextionCommand();
        Serial.print("page 0");
        enterNextionCommand();
        menu.contentShow(0);
    }
    else if (page == 1)
    {
        enterNextionCommand();
        Serial.print("page 1");
        enterNextionCommand();

        initilazePic();
        menu.focusUp();
        menu.contentShow(1);
    }
}

/* --- UKONCOVACI SEKVENCE ZNAKU PRO DISPLEJ --- */
void enterNextionCommand()
{
    Serial.write(0xff);
    Serial.write(0xff);
    Serial.write(0xff);
}

void trigger1()
{
    if (!mainVariable.getPump().isRunning())
        mainVariable.getPump().resetRotation();
    else
        mainVariable.getDisplay().dispSetInfo("Counter can't be nullify when pump is running", true);
}

void trigger2()
{
    if (!mainVariable.getPump().isRunning())
    {
        bool set = mainVariable.getPump().parameters.Direction;
        mainVariable.getPump().setDirection(!set);
        mainVariable.getDisplay().menu.contentShow(0);

        if (mainVariable.getMQTT().client.connected())
        {
            DynamicJsonDocument settingsToSend(1024);
            for (size_t i = 0; i < SETTINGS_COUNT - 1; i++)
            {
                settingsToSend[i]["Name"] = mainVariable.getDisplay().menu.settings[i + 1].Name;
                settingsToSend[i]["Value"] = mainVariable.getDisplay().menu.settings[i + 1].NumValue;
                settingsToSend[i]["Unit"] = mainVariable.getDisplay().menu.settings[i + 1].Unit;
            }

            String settingsToSendString = "";
            serializeJson(settingsToSend, settingsToSendString);
            mainVariable.getMQTT().publish("peristaltic/settings", settingsToSendString.c_str());
        }
    }
    else
        mainVariable.getDisplay().dispSetInfo("Direction can't be change when pump is running", true);
}
void trigger3()
{
    if (mainVariable.getPump().isRunning())
    {
        mainVariable.getDisplay().dispSetInfo("Settings can't be change when pump is running", true);
    }
    else
    {
        mainVariable.getDisplay().lastInteractionTimeMark = millis();
        mainVariable.getDisplay().activePage = 1;
        mainVariable.getDisplay().setPage(1);
    }
}
void trigger4()
{
    if (!mainVariable.getPump().isRunning())
    {
        int set = (int)mainVariable.getPump().parameters.Mode;
        mainVariable.getPump().setMode(++set);
        mainVariable.getDisplay().menu.contentShow(0);

        if (mainVariable.getMQTT().client.connected())
        {
            DynamicJsonDocument settingsToSend(1024);
            for (size_t i = 0; i < SETTINGS_COUNT - 1; i++)
            {
                settingsToSend[i]["Name"] = mainVariable.getDisplay().menu.settings[i + 1].Name;
                settingsToSend[i]["Value"] = mainVariable.getDisplay().menu.settings[i + 1].NumValue;
                settingsToSend[i]["Unit"] = mainVariable.getDisplay().menu.settings[i + 1].Unit;
            }

            String settingsToSendString = "";
            serializeJson(settingsToSend, settingsToSendString);
            mainVariable.getMQTT().publish("peristaltic/settings", settingsToSendString.c_str());
        }
    }
    else
        mainVariable.getDisplay().dispSetInfo("Mode can't be change when pump is running", true);
}
void trigger5()
{
    Serial.println("");
    Serial.println("trig5");
    Serial.println("");
}
void trigger6()
{
    Serial.println("");
    Serial.println("trig6");
    Serial.println("");
}
void trigger7()
{
    Serial.println("");
    Serial.println("trig7");
    Serial.println("");
}
void trigger8()
{
    Serial.println("");
    Serial.println("trig8");
    Serial.println("");
}
void trigger9()
{
    Serial.println("");
    Serial.println("trig9");
    Serial.println("");
}
void trigger10()
{
    Serial.println("");
    Serial.println("trig10");
    Serial.println("");
}
