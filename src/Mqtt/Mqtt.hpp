#pragma once
#include <EspMQTTClient.h>
#include <ArduinoJson.h>
#include <mainLib.h>
#include <Display/Display.hpp>

class MQTT
{
private:
    const char *ssid;
    const char *password;
    const char *mqtt_server;
    const char *mqtt_user;
    const char *mqtt_pass;

public:
    WiFiClient espClient;
    PubSubClient client;

    MQTT();
    void publish(char *topic, const char *message);
    void subscribe(const char *topic);
    void reconnect();

    void MQTTbegin();
    void setupWifi();

    void loop();
    void MQTTSetrup();
};
extern MQTT *mqtt;

void callback(char *topic, byte *message, unsigned int length);
