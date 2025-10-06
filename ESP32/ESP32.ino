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
const char* mqtt_user = SECRET_MQTT_USER;
const char* mqtt_pass = SECRET_MQTT_PASS;

WiFiClient espClient;
PubSubClient client(espClient);
Mppt mpptController;

#define LED_PIN 48
#define NUM_LEDS 1
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// --- Funktionen ---
void setLEDColor(uint8_t r, uint8_t g, uint8_t b) {
  strip.setPixelColor(0, strip.Color(r, g, b));
  strip.show();
}

void setup_wifi() {
  Serial.println();
  Serial.print("Verbinde mit WLAN: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWLAN verbunden!");
  Serial.print("IP Adresse: ");
  Serial.println(WiFi.localIP());
}

bool subscribeTopic(const char* topic) {
  if (!client.connected()) {
    Serial.println("⚠️ MQTT nicht verbunden, Abonnement fehlgeschlagen");
    return false;
  }

  if (client.subscribe(topic)) {
    Serial.print("✅ Erfolgreich abonniert: ");
    Serial.println(topic);
    return true;
  } else {
    Serial.print("❌ Abonnement fehlgeschlagen: ");
    Serial.println(topic);
    return false;
  }
}

void toggleLoadStatus() {
  int status = mpptController.getLoadSwitch();
  if (status == 1) Serial.println("Load ist AN");
  else if (status == 0) Serial.println("Load ist AUS");
  else {
    Serial.println("Fehler beim Lesen des Load-Status");
    return;
  }

  bool newStatus = (status == 0); // negieren
  bool success = mpptController.setLoadSwitch(newStatus);
  if (success) {
    Serial.print("Load ");
    Serial.println(newStatus ? "eingeschaltet ✅" : "ausgeschaltet ✅");

    String statusMsg = newStatus ? "on" : "off";
    if (client.connected()) {
      client.publish("modbus/load/status", statusMsg.c_str());
      Serial.print("Status veröffentlicht: ");
      Serial.println(statusMsg);
    }
  } else {
    Serial.println("Fehler beim Setzen des Load-Status ❌");
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  Serial.print("Nachricht empfangen [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(msg);

    if (String(topic) == "modbus/load/toggle") {
        mpptController.toggleLoad(); // automatisch Modus prüfen und Load toggeln
    }
}

bool reconnect() {
  Serial.print("Versuche MQTT-Verbindung...");
  if (client.connect("ESP32Client", mqtt_user, mqtt_pass)) {
    Serial.println("verbunden");
    subscribeTopic("modbus/load/toggle");
    return true;
  } else {
    Serial.print("fehlgeschlagen, rc=");
    Serial.println(client.state());
    return false;
  }
}

// --- Setup ---
void setup() {
  Serial.begin(9600);
  setup_wifi();

  client.setServer(mqtt_server, MQTT_PORT);
  client.setCallback(callback);

  mpptController.setupModbusRTU();

  strip.begin();
  strip.show();
  strip.setBrightness(30);

  // Watchdog konfigurieren
  esp_task_wdt_config_t wdt_config = {
    .timeout_ms = 15000,
    .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
    .trigger_panic = true
  };
  esp_task_wdt_init(&wdt_config);
  esp_task_wdt_add(NULL);
}

// --- Loop ---
void loop() {
  esp_task_wdt_reset();

  // WLAN prüfen
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ WLAN verloren, versuche reconnect...");
    WiFi.reconnect();
  }

  // MQTT prüfen
  static unsigned long lastReconnectAttempt = 0;
  unsigned long now = millis();
  if (!client.connected()) {
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      reconnect();
    }
  } else {
    client.loop();
  }

  // alle 2 Sekunden Daten senden
  static unsigned long lastMsg = 0;
  if (now - lastMsg > 2000) {
    lastMsg = now;

    setLEDColor(0, 255, 0); // LED grün = senden aktiv

    esp_task_wdt_reset();
    String batteryData = mpptController.getBatteryData();
    String pvData      = mpptController.getPVData();
    String loadData    = mpptController.getLoadData();
    esp_task_wdt_reset();

    if (client.connected()) {
      client.publish("modbus/battery", batteryData.c_str());
      client.publish("modbus/pv", pvData.c_str());
      client.publish("modbus/load", loadData.c_str());
    }

    setLEDColor(255, 0, 0); // LED rot = fertig
  }

  delay(10);
}
