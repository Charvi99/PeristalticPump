#pragma once
#include <EasyNextionLibrary.h>
#include <Controls/Controls.hpp>
#include <Display/Menu.h>
#include <mainLib.h>

#define SETTINGS_COUNT 11

#define UNFOCUSED 9
#define FOCUSED 10
#define SELECTED 11

#define UNFOCUSED_NAME_BC 7058
#define UNFOCUSED_NAME_PC 65535
#define UNFOCUSED_UNIT_BC 4555
#define UNFOCUSED_UNIT_PC 65535
#define FOCUSED_NAME_BC 65333
#define FOCUSED_NAME_PC 4555
#define FOCUSED_UNIT_BC 7058
#define FOCUSED_UNIT_PC 65333

extern int activePage;
class MenuItem
{
public:
    String Name;
    String Value;
    String Unit;
    bool Focused;
    bool Selected;
    unsigned int storedItem;
    MenuItem();
};
class Menu
{
public:
    struct Settings
    {
        String Name;
        String Value;
        unsigned short NumValue;
        String Unit;
    };

    MenuItem Items[4];
    MenuItem ItemPom;
    Settings settings[SETTINGS_COUNT];

    int focusedIndex;
    int selectedIndex;
    EasyNex myNex;

    Menu();

    void focusUp();
    void focusDown();
    void selectFocused();
    void deselectFocused();
    void setValue(String content);
    int searchForFocused();
    int searchForSelected();

    void contentShow(int page);
    void contentUp();
    void contentDown();
    void displayContentInItem(int item, int content);

    void IncreseVal();
    void DecreseVal();

    void insertValueIntoTheFreakingSetting(String itemName, int numValue, String value);
    void dispSetInfo(char *content, bool warning);
    void newLine();
};

class Display
{
public:
    Menu menu;
    EasyNex myNex;
    bool switcher;
    int page;
    int activePage;
    char *currentInfo = "";
    bool switcher2 = false;
    unsigned long lastIconUpdateTimeMark = 0;
    Display();

    void loop(bool up, bool down);

    void setPage(int page);

    void dispSetMode(char *content);
    void dispSetDose(char *content);
    void dispSetDirection(char *content);
    void dispSetSpeed(char *content);
    void dispSetInterval(char *content);
    void dispSetRamp(char *content);
    void dispSetInfo(char *content, bool warning);
    void dispSetMl(int content);
    void dispGraph(int content);
    void dispRun(bool content);

    void initilazePic();
};

extern Display *disp;
void enterNextionCommand();
