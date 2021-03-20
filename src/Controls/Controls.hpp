#pragma once
#include <mainLib.h>
#include <ezButton.h>
#include "Button2.h"

#define START_BTN 26
#define STOP_BTN 27

#define CONTROL_ENCODER_BTN 16
#define CONTROL_ENCODER_A 4
#define CONTROL_ENCODER_B 17

#define MANUAL_BTN 4
#define SEMIAUTOMAT_BTN 5
#define AUTOMAT_BTN 6
#define INTERVAL_BTN 7

class Control
{
public:
    Button2 start_btn ;
    Button2 stop_btn;
    Button2 encoder_btn;

    bool Up;
    bool Down;

    Control();
    void controlSetup();
    void encoderLoop();
    void loop();
    ESP32Encoder controlEncoder;
};
extern Control *control;
    
void buttonStateLoop(Button2 &btn);