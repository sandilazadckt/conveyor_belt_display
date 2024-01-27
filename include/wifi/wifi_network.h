#include <WiFi.h>
#include <PubSubClient.h>
String deviceadd = "5555555";

const char *ssid = "SSLAIOT";
const char *password = "snowtexssliota";

// const char *ssid = "iot";
// const char *password = "470525pA";

String conveyorBeltRecived = "conveyorbelt/recived/" + deviceadd;
String deviceUpdateAll = "/update/url/";
String deviceUpdateAllOne = "/update/url/" + deviceadd;

const char *mqtt_server = "172.16.16.4";
int connectToWiFi();
void callback(char *topic, byte *message, unsigned int length);

WiFiClient espClient, espClient2;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;



void reconnect()
{
    // Loop until we're reconnected
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect(deviceadd.c_str()))
        {
            Serial.println("connected");
            // Subscribe
            client.subscribe(deviceUpdateAll.c_str());
            client.subscribe(deviceUpdateAllOne.c_str());
            client.subscribe(conveyorBeltRecived.c_str());
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(1000);
        }
    }
}

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.println("Connected to AP successfully!");
}

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.println("Disconnected from WiFi access point");
    Serial.print("WiFi lost connection. Reason: ");
    Serial.println(info.wifi_sta_disconnected.reason);
    Serial.println("Trying to Reconnect");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("Connecting to WiFi...");
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
}

void initWiFi()
{

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print('.');
        delay(500);
    }
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    Serial.println(WiFi.localIP());
}