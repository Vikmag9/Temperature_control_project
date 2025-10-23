#include <WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// WiFi
const char *ssid = "lab@i17";
const char *password = "lab@i17!";

// MQTT Broker
const char *mqtt_broker = "lab.bpm.in.tum.de";
const char *topic = "temperature/value";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

// DS18B20 Setup
#define ONE_WIRE_BUS 4  // GPIO4 on esp32
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message: ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
    Serial.println("-----------------------");
}

void setup() {
    Serial.begin(115200);
    
    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the Wi-Fi network");

    // MQTT 
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    while (!client.connected()) {
        String client_id = "esp32-client-";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
        if (client.connect(client_id.c_str())) {
            Serial.println("Public EMQX MQTT broker connected");
        } else {
            Serial.print("failed with state ");
            Serial.print(client.state());
            delay(2000);
        }
    }
    client.subscribe(topic);

    sensors.begin();
}

void loop() {
    client.loop();

    // Request temperature reading
    sensors.requestTemperatures();
    float temperatureC = sensors.getTempCByIndex(0);

    // Convert float to string
    char tempString[10];
    dtostrf(temperatureC, 1, 2, tempString);

    // Publish temperature
    client.publish(topic, tempString);

    Serial.print("Published temperature: ");
    Serial.println(tempString);

    delay(10000);  // Wait 10 seconds before next reading
}