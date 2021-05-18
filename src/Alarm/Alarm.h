#pragma once
#include <Tone32.h>
#include <Freenove_WS2812_Lib_for_ESP32.h>
#include "mainLib.h"

#define DATA_SEND_FREQUENCY 500
#define DATA_COUNT 3

#define BUZZER_PIN 26
#define BUZZER_CHANNEL 1
#define SESNACT 190
#define OSMIN 375
#define CTVRT 750
#define PUL 1500
#define CELA 3000

#define LEDS_COUNT 16
#define LEDS_PIN 25
#define CHANNEL 2

#define MAX_BRIGHTNESS 100
#define MIN_BRIGHTNESS 20

class Alarm
{
public:
    Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(LEDS_COUNT, LEDS_PIN, CHANNEL);
    bool stripBrightnessRise = false;
    int stripBrightnessLvl = MAX_BRIGHTNESS;
    unsigned long lastStripUpdateTimeMark = 0;

    unsigned long lastDataSendTimeMark = 0;
    unsigned long lastStatusUpdateTimeMark = 0;

    int beeperFrequency = 0;
    int beeperDurationTime = 0;
    bool beeperEnable = false;

    enum AlarmStatus
    {
        RUNNING,
        STOPED,
        WARNING,
        ERROR,
        MQTT_CONECTED,
        MQTT_DISCPNECTED,
        MQTT_RECEIVING
    };

    int status;

    Alarm();

    void showRun();
    void showStop();
    void showWarning();
    void showError();
    void showMQTT();
    void shutDownLED();

    void alarmLoop();
    void beeperLoop();
    void LEDBreathing();
    void stripUpdate();

    void setStatus();

    void SendData();
};