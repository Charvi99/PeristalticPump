#include <Display/Display.hpp>

/* --- KONSTRUKTOR DISPLEJE --- */
Display::Display()
{
    myNex.setSerial(Serial);
    myNex.begin(115200);
    activePage = 0;
    switcher = true;
    Serial.print("page 0");
    enterNextionCommand();

    initilazePic();
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
        myNex.writeNum("t3.xcen", 1);
        myNex.writeNum("t3.bco", UNFOCUSED_NAME_BC);
        myNex.writeNum("t3.pco", UNFOCUSED_NAME_PC);
        myNex.writeStr("t3.txt", content);
    }
    else
    {
        myNex.writeNum("t3.xcen", 0);
        myNex.writeNum("t3.bco", FOCUSED_NAME_BC);
        myNex.writeNum("t3.pco", FOCUSED_NAME_PC);
        myNex.writeStr("t3.txt", content);
    }
}

void Display::dispSetMl(int content)
{
    myNex.writeNum("n0.val", content);
}
void Display::dispGraph(int content)
{
    myNex.writeNum("graphVal.val", content);
}
void Display::dispRun(bool content)
{
    if (content == true)
        myNex.writeNum("enable.val", 1);
    else
        myNex.writeNum("enable.val", 0);
}

/* --- HLAVNI SMYCKA PRO DISPLEJ --- */
void Display::loop(bool up, bool down)
{
    myNex.NextionListen();

    if (activePage == 1)
    {
        if (up == true)
        {
            up = false;
            if (menu.searchForSelected() == -1) //nic neni vybrano -> posun v menu
                menu.focusUp();
            else ///neco je vybrano -> zmena hodnoty
                menu.IncreseVal();
        }
        else if (down == true)
        {
            down = false;
            if (menu.searchForSelected() == -1) //nic neni vybrano -> posun v menu
                menu.focusDown();
            else ///neco je vybrano -> zmena hodnoty
                menu.DecreseVal();
        }
        else
        {
            //Serial.println("NOTHING");
        }
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
    settings[1] = {"Mode", "0", 0, ""};
    settings[2] = {"Dose", "0", 0, "ml"};
    settings[3] = {"Direction", "0", 0, ""};
    settings[4] = {"Speed", "0", 0, "%"};
    settings[5] = {"Interval", "0", 0, "s"};
    settings[6] = {"Ramp", "0", 0, "s"};

    for (size_t i = 0; i < 4; i++)
        Items[i].storedItem = i;

    focusedIndex = 0;
    myNex.setSerial(Serial);
    myNex.begin(115200);
    myNex.writeNum("p_" + String(focusedIndex) + ".pic", UNFOCUSED);

    contentShow();
}

/* --- POSUN FOCUSU NA PRVEK O INDEX NIZ --- */
void Menu::focusUp()
{
    enterNextionCommand();
    if (focusedIndex > 0)
    {
        myNex.writeNum("p_" + String(focusedIndex) + ".pic", UNFOCUSED);
        myNex.writeNum("p_" + String(--focusedIndex) + ".pic", FOCUSED);
        contentShow();
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
        contentShow();
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
        disp->activePage = 0;
        Serial.print("page ");
        Serial.print(disp->activePage);
        enterNextionCommand();
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

        contentShow();
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
    contentShow();

    Serial.println("DESELECTED");
    Serial.println("DESELECTED");

    if (mqtt->client.connected())
    {
        DynamicJsonDocument settingsToSend(1024);
        for (size_t i = 0; i < SETTINGS_COUNT; i++)
        {
            settingsToSend[i]["Name"] = disp->menu.settings[i].Name;
            settingsToSend[i]["Value"] = disp->menu.settings[i].Value;
            settingsToSend[i]["Unit"] = disp->menu.settings[i].Unit;
        }
        String settingsToSendString = "";
        serializeJson(settingsToSend, settingsToSendString);
        mqtt->publish("peristaltic/settings", settingsToSendString.c_str());
    }
}

/* --- NAPLNENI TEXTOVYCH OKEN ODPOVIDAJICIM TEXTEM --- */
void Menu::contentShow()
{
    for (size_t i = 0; i < 4; i++)
    {
        displayContentInItem(i, Items[i].storedItem);
    }
}

/* --- POSUN TEXTU PRI POSUNU PRVKU --- */
void Menu::contentUp()
{
    if (Items[0].storedItem > 0)
        for (size_t i = 0; i < 4; i++)
            displayContentInItem(i, Items[i].storedItem - 1);
    else
        contentShow();
}
void Menu::contentDown()
{
    if (Items[3].storedItem < SETTINGS_COUNT - 1)
        for (size_t i = 0; i < 4; i++)
            displayContentInItem(i, Items[i].storedItem + 1);
    else
        contentShow();
}

/* --- NAPLNI TEXTEM KONKRETNI PRVEK--- */
void Menu::displayContentInItem(int item, int content)
{
    enterNextionCommand();

    String itemName = "name_" + String(item) + ".txt";
    myNex.writeStr(itemName, settings[content].Name);
    itemName = "val_" + String(item) + ".txt";
    myNex.writeStr(itemName, settings[content].Value);
    itemName = "unit_" + String(item) + ".txt";
    myNex.writeStr(itemName, settings[content].Unit);
    Items[item].storedItem = content;
}

/* --- ROZBRAZENI ZVYSENE HODNOTY --- */
void Menu::IncreseVal()
{
    settings[Items[searchForSelected()].storedItem].NumValue++;
    settings[Items[searchForSelected()].storedItem].Value = String(settings[Items[searchForSelected()].storedItem].NumValue);
    contentShow();
}

/* --- ROZBRAZENI SNIZENE HODNOTY --- */
void Menu::DecreseVal()
{
    settings[Items[searchForSelected()].storedItem].NumValue--;
    settings[Items[searchForSelected()].storedItem].Value = String(settings[Items[searchForSelected()].storedItem].NumValue);
    contentShow();
}

/* --- ZVYSENA HODNOTA SE ULOZI DO GLOBALNIHO NASTAVENI --- */
void Menu::insertValueIntoTheFreakingSetting(String itemName, int value)
{
    for (size_t i = 0; i < SETTINGS_COUNT; i++)
        if (itemName == settings[i].Name)
        {
            settings[i].NumValue = value;
            settings[i].Value = String(value);
        }
    contentShow();
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

/* --- UKONCOVACI SEKVENCE ZNAKU PRO DISPLEJ --- */
void enterNextionCommand()
{
    Serial.write(0xff);
    Serial.write(0xff);
    Serial.write(0xff);
}
