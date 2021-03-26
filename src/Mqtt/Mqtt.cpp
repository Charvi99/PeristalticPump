
#include <MQTT\MQTT.hpp>
#include <mainLib.h>

/* --- KONSTRUKTOR MQTT --- */
MQTT::MQTT()
{
  ssid = "Charvi99";
  password = "charvijebozi";
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
  setupWifi();

  client.connect("ESP32", mqtt_user, mqtt_pass);
  Serial.println(client.connected());
  client.subscribe("peristaltic/settings");
  client.publish("peristaltic/data", "tady ESP32 k tvym sluzbam");
  client.setKeepAlive(5);
  client.setBufferSize(1024);
}

/* ---  PRIPOJENI K WIFI --- */
void MQTT::setupWifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
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

  for (int i = 0; i < length; i++)
  {
    //Serial.print((char)message[i]);
    if (topic[12] == 'd') //topic == peristlatic/data
      mainVariable.getJSONData() += (char)message[i];
    if (topic[12] == 's') //topic == peristlatic/settings
      mainVariable.getJSONSettings() += (char)message[i];
  }
  Serial.println(mainVariable.getJSONSettings());
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
  if (!client.connected())
    reconnect();
  client.loop();
}
