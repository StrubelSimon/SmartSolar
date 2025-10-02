#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h"
#include "Mppt.h"
#include <Adafruit_NeoPixel.h>
#include "esp_task_wdt.h"

// WLAN Einstellungen
const char* ssid = SECRET_SSID;
const char* password = SECRET_PASS;

// MQTT-Broker
const char* mqtt_server = MQTT_IP;

// MQTT Login
const char* mqtt_user = SECRET_MQTT_USER;
const char* mqtt_pass = SECRET_MQTT_PASS;

WiFiClient espClient;
PubSubClient client(espClient);
Mppt mpptController;

#define LED_PIN 48
#define NUM_LEDS 1
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

unsigned long lastReconnectAttempt = 0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Verbinde mit WLAN: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WLAN verbunden!");
  Serial.print("IP Adresse: ");
  Serial.println(WiFi.localIP());
}

void setLEDColor(uint8_t r, uint8_t g, uint8_t b) {
  strip.setPixelColor(0, strip.Color(r, g, b));
  strip.show();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Nachricht empfangen [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void setup() {
  Serial.begin(9600);

  setup_wifi();
  client.setServer(mqtt_server, MQTT_PORT);
  client.setCallback(callback);

  mpptController.setupModbusRTU();

  strip.begin();
  strip.show();
  strip.setBrightness(30); 

  esp_task_wdt_config_t wdt_config = {
    .timeout_ms = 15000,   // 15 Sekunden Timeout
    .idle_core_mask = (1 << portNUM_PROCESSORS) - 1, // beide Kerne überwachen
    .trigger_panic = true
  };

  esp_task_wdt_init(&wdt_config);
  esp_task_wdt_add(NULL);
}


bool reconnect() {
  Serial.print("Versuche MQTT-Verbindung...");
  if (client.connect("ESP32Client", mqtt_user, mqtt_pass)) {
    Serial.println("verbunden");
    client.subscribe("esp32/topic");
    return true;
  } else {
    Serial.print("fehlgeschlagen, rc=");
    Serial.println(client.state());
    return false;
  }
}

void loop() {
  esp_task_wdt_reset();
  // --- WLAN Verbindung prüfen ---
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ WLAN verloren, versuche non-blocking Reconnect...");
    WiFi.reconnect(); // kurz versuchen (nicht blockierend)
  }

  // --- MQTT Verbindung prüfen ---
  if (!client.connected()) {
    static unsigned long lastReconnectAttempt = 0;
    unsigned long now = millis();

    if (now - lastReconnectAttempt > 5000) {  // alle 5 Sekunden versuchen
      lastReconnectAttempt = now;
      if (reconnect()) {
        lastReconnectAttempt = 0;  // Reset wenn erfolgreich
      }
    }
  } else {
    client.loop();  // nur aufrufen wenn verbunden
  }

  // --- alle 2 Sekunden Daten senden ---
  static long lastMsg = 0;
  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;

    setLEDColor(0, 255, 0); // LED grün = senden aktiv

    // --- MPPT Daten ---
    esp_task_wdt_reset();
    String batteryData = mpptController.getBatteryData();
    esp_task_wdt_reset();
    String pvData      = mpptController.getPVData();
    esp_task_wdt_reset();
    String loadData    = mpptController.getLoadData();
    esp_task_wdt_reset();

    // --- MQTT publish ---
    if (client.connected()) { // nur senden, wenn MQTT aktiv
      client.publish("modbus/battery", batteryData.c_str());
      client.publish("modbus/pv", pvData.c_str());
      client.publish("modbus/load", loadData.c_str());
    }

    setLEDColor(255, 0, 0); // LED rot = fertig
  }

  delay(10); // kleine Pause, entlastet CPU / WiFi Stack
}
