
#include <MQTT\MQTT.hpp>
#include <mainLib.h>

AsyncElegantOtaClass AsyncElegantOTA;
AsyncWebServer server(80);
/* --- KONSTRUKTOR MQTT --- */
MQTT::MQTT()
{
  ssid = "CharviWifi";
  password = "xkvqqoac";
  mqtt_server = "mqtt.flespi.io";
  mqtt_user = "fZetFr07fPpzFxegftTmp7sSs1zOcbJgYNzu2op73Euw1wB6Nkh3hzLxo9ap8JvX";
  mqtt_pass = "";
  client.setClient(espClient);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

/* --- METODA ZRIZUJICI PRIPOJENI K WIFI A VYTVORENI KLIENTA --- */
void MQTT::MQTTbegin()
{
  mainVariable.getDisplay().activePage = 0;
  mainVariable.getDisplay().setPage(0);
  mainVariable.getDisplay().menu.contentShow(0);

  setupWifi();

  client.connect("ESP32", mqtt_user, mqtt_pass,"peristaltic/confirm",2,0,"disconnected",1);
  Serial.println(client.connected());

  client.subscribe("peristaltic/settings");
  client.subscribe("peristaltic/run");
  client.subscribe("peristaltic/confirm");

  client.setKeepAlive(5);
  client.setBufferSize(1024);
  client.publish("peristaltic/confirm", "connected");
}

/* ---  PRIPOJENI K WIFI --- */
void MQTT::setupWifi()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    mainVariable.getDisplay().dispSetInfo("", false);
    char *ssid_char;
    if (WiFi.status() != WL_CONNECTED)
    {
      ssid_char = new char[strlen(ssid) + 1];
      strcpy(ssid_char, ssid);

      test = "Connecting to:";
      test = test + "\r" + "\n";
      test += ssid_char;
      char *info = new char[test.length() + 1];
      strcpy(info, test.c_str());

      mainVariable.getDisplay().dispSetInfo(info, false);
      test = test + "\r" + "\n";
    }
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
      test += ". ";
      char *info_tecky = new char[test.length() + 1];
      strcpy(info_tecky, test.c_str());
      mainVariable.getDisplay().dispSetInfo(info_tecky, false);
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    test = "Sucessfully connected to:\r\n";
    test += ssid_char;
    char *info_tecky = new char[test.length() + 1];
    strcpy(info_tecky, test.c_str());
    mainVariable.getDisplay().dispSetInfo(info_tecky, false);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", "Hi! I am ESP32.");
    });

    AsyncElegantOTA.begin(&server); // Start ElegantOTA
    server.begin();
    Serial.println("HTTP server started");
  }
}

/* --- METODA VOLANA PRI ODPOJENI KLIENTA --- */
void MQTT::reconnect()
{
  if (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32", mqtt_user, mqtt_pass))
    {
      Serial.println("connected");
      subscribe("peristaltic/settings");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
    }
  }
}

/* --- HANDLER PRO PRICHOZI ZPRAVY --- */
void callback(char *topic, byte *message, unsigned int length)
{
  //Serial.print("Message arrived on topic: ");
  //Serial.print(topic);
  //Serial.print(". Message: ");
  String incomingMessage = "";
  Serial.println();
  for (int i = 0; i < length; i++)
  {
    incomingMessage += (char)message[i];
    Serial.print((char)message[i]);
  }
  Serial.println();

  if (topic[12] == 'c') //topic == peristlatic/confirm
  {
    //mainVariable.getJSONData() = incomingMessage;
    if (incomingMessage == "MQTT connected")
    {
      mainVariable.getMQTT().publish("peristaltic/confirm", "connected");
      mainVariable.alarmStatus = (int)Alarm::AlarmStatus::MQTT_CONECTED;
      mainVariable.getAlarm().lastStatusUpdateTimeMark = millis();
    }
  }
  else if (topic[12] == 's' && mainVariable.getPump().isRunning() == false) //topic == peristlatic/settings
  {
    mainVariable.getJSONSettings() = incomingMessage;
    mainVariable.getMQTT().publish("peristaltic/confirm", "ok");
    mainVariable.alarmStatus = (int)Alarm::AlarmStatus::MQTT_CONECTED;
    mainVariable.getAlarm().lastStatusUpdateTimeMark = millis();
  }
  else if (topic[12] == 'r') //topic == peristlatic/run
  {
    if (incomingMessage == "START_P")
    {
      mainVariable.getPump().pumpEnable();
      mainVariable.alarmStatus = (int)Alarm::AlarmStatus::MQTT_CONECTED;
      mainVariable.getAlarm().lastStatusUpdateTimeMark = millis();
    }
    else if (incomingMessage == "STOP_P")
    {
      mainVariable.getPump().pumpDisable();
      mainVariable.alarmStatus = (int)Alarm::AlarmStatus::MQTT_CONECTED;
      mainVariable.getAlarm().lastStatusUpdateTimeMark = millis();
    }
    mainVariable.getMQTT().publish("peristaltic/confirm", "ok");
  }
}

/* --- API PRO ODESLANI ZPRAVY --- */
void MQTT::publish(char *topic, const char *message)
{
  client.publish(topic, message);
}

/* --- API PRO ZAPOCNUTI ODEBIRANI TOPICU --- */
void MQTT::subscribe(const char *topic)
{
  client.subscribe(topic);
}

/* --- HLAVNI MQTT SMYCKA KONTROLUJICI PRICHOZI ZPRAVY A OBNOVENI SPOJENI SE SERVEREM --- */
void MQTT::loop()
{
  if (!client.connected() && mainVariable.getDisplay().menu.settings[9].NumValue == 1)
    reconnect();
  client.loop();
}
