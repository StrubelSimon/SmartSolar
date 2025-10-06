#include "Mppt.h"
#include <SPI.h>
#include <ModbusMaster.h>
#include <ArduinoJson.h>

#define RX2 16

#define TX2 17

ModbusMaster node;

Mppt::Mppt()
{
}

bool Mppt::setupModbusRTU()
{
  // Modbus communication via UART //115200
  Serial1.begin(115200, SERIAL_8N1, RX2, TX2);
  delay(1000);
  Serial.println("✅ Modbus RTU ready");
  // Configuration of the Modbus slave ID
  node.begin(1, Serial1);
  
  return true;
}

String Mppt::getBatteryData()
{
	DynamicJsonDocument batteryDoc(1024);
	batteryDoc["voltage"] = connectToModbus(0x331A) / 100.00;
	batteryDoc["voltageMin"] = connectToModbus(0x3303) / 100.00;
	batteryDoc["voltageMax"] = connectToModbus(0x3302) / 100.00;
	batteryDoc["current"] = connectToModbus(0x331B) / 100.00;
  //ToDo:  Calculate the actual current with register L and H
  //Serial.println(connectToModbus(0x331B) / 100.00);
  //Serial.println(connectToModbus(0x331C) / 100.00);
	batteryDoc["temperature"] = connectToModbus(0x3110) / 100.00;
	batteryDoc["percentage"] = connectToModbus(0x311A) / 1;
	batteryDoc["charging"] = connectToModbus(0x3201);
	batteryDoc["health"] = connectToModbus(0x3200);

	String jsonString;
	serializeJson(batteryDoc, jsonString);
	return jsonString;
}

String Mppt::getLoadData()
{
  DynamicJsonDocument loadDoc(1024);
	loadDoc["voltage"] = connectToModbus(0x310C) / 100.00;
	loadDoc["current"] = connectToModbus(0x310D) / 100.00;
	loadDoc["power"] = connectToModbus(0x310E) / 100.00;
	loadDoc["status"] = connectToModbus(0x3202);

	String jsonString;
	serializeJson(loadDoc, jsonString);
	return jsonString;
}

String Mppt::getPVData()
{
	DynamicJsonDocument pvDoc(1024);
	pvDoc["voltage"] = connectToModbus(0x3100) / 100.00;
	pvDoc["current"] = connectToModbus(0x3101) / 100.00;
	pvDoc["power"] = connectToModbus(0x3102) / 100.00;
	pvDoc["status"] = connectToModbus(0x3201);

	String jsonString;
	serializeJson(pvDoc, jsonString);
	return jsonString;
}

int Mppt::getLoadSwitch() {
    uint8_t rc = node.readHoldingRegisters(0x906A, 1);
    if (rc == node.ku8MBSuccess) {
        return node.getResponseBuffer(0); // 0=off, 1=on
    } else {
        return -1; // Fehler
    }
}

bool Mppt::setLoadSwitch(bool on) {
    uint16_t value = on ? 1 : 0;
    uint8_t rc = node.writeSingleRegister(0x906A, value);
    return (rc == node.ku8MBSuccess);
}

int Mppt::getLoadMode() {
    uint8_t rc = node.readHoldingRegisters(0x903D, 1);
    if (rc == node.ku8MBSuccess) {
        return node.getResponseBuffer(0); // 0x0000=Manual
    } else {
        return -1;
    }
}

bool Mppt::setLoadMode(uint16_t mode) {
    uint8_t rc = node.writeSingleRegister(0x903D, mode);
    return (rc == node.ku8MBSuccess);
}

void Mppt::toggleLoad() {
    int mode = getLoadMode();
    if (mode != 0x0000) { // Manual Control
        Serial.println("⚠️ Modus nicht Manual, setze auf Manual...");
        if (!setLoadMode(0x0000)) {
            Serial.println("Fehler beim Setzen des Manual Mode ❌");
            return;
        }
        // Optional: Parameter setzen, falls nötig
        // setLoadParameters(startV, stopV, delaySec);
    }

    int status = getLoadSwitch();
    if (status == -1) {
        Serial.println("Fehler beim Lesen des Load-Status");
        return;
    }

    bool newStatus = (status == 0); // negieren
    if (setLoadSwitch(newStatus)) {
        Serial.print("Load ");
        Serial.println(newStatus ? "eingeschaltet ✅" : "ausgeschaltet ✅");
    } else {
        Serial.println("Fehler beim Setzen des Load-Status ❌");
    }
}

int Mppt::connectToModbus(uint16_t address)
{
	uint8_t result = node.readInputRegisters(address, 1);

	if (result == node.ku8MBSuccess) {
		return node.getResponseBuffer(0);
	} else {
		Serial.print("Modbus Error Code: ");
		Serial.println(result, DEC);

    // Error handling based on the error code
    switch (result) {
      case 0x01: Serial.println("Illegal function"); break;
      case 0x02: Serial.println("Illegal data address"); break;
      case 0x03: Serial.println("Illegal data value"); break;
      case 0x04: Serial.println("Slave device failure"); break;
      case 0xE0: Serial.println("Puffer-Fehler!"); break;
      case 0xE1: Serial.println("Ungültige Antwort!"); break;
      case 0xE2: Serial.println("TIMEOUT - Slave antwortet nicht!"); break;
      case 0xE3: Serial.println("Ungültige Funktion!"); break;
      case 0xE4: Serial.println("CRC-Fehler!"); break;
      case 0xE5: Serial.println("Slave Device Failure!"); break;
      default: Serial.println("Unspecified error"); break;
    }
	}
}