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

String Mppt::getPVData()
{
	DynamicJsonDocument pvDoc(1024);
	pvDoc["voltage"] = connectToModbus(0x3100) / 100.00;
	pvDoc["current"] = connectToModbus(0x3101) / 100.00;
  
  // 0x3102 (L) + 0x3103 (H)
  pvDoc["power"] = connectToModbus32(0x3102, 100);
	
  //pvDoc["status"] = connectToModbus(0x3201);
  int status = connectToModbus(0x3201);

  if (status != -1) {
    pvDoc["status"] = (status >> 14) & 0x03;// Bits D15-D14
  }

  // 0x330C (L) + 0x330D (H)
  pvDoc["generated_today"] = connectToModbus32(0x330C, 100);
  // 0x330E (L) + 0x330F (H)
  pvDoc["generated_month"] = connectToModbus32(0x330E, 100);
  // 0x3310 (L) + 0x3311 (H)
  pvDoc["generated_year"] = connectToModbus32(0x3310, 100);
  // 0x3312 (L) + 0x3313 (H)
  pvDoc["generated_total"] = connectToModbus32(0x3312, 100);

	String jsonString;
	serializeJson(pvDoc, jsonString);
	return jsonString;
}

String Mppt::getBatteryData()
{
	DynamicJsonDocument batteryDoc(1024);
	batteryDoc["voltage"] = connectToModbus(0x331A) / 100.00;
	batteryDoc["voltage_min"] = connectToModbus(0x3303) / 100.00;
	batteryDoc["voltage_max"] = connectToModbus(0x3302) / 100.00;

  // 0x331B (L) + 0x331C (H)
  batteryDoc["current"] = connectToModbus32(0x331B, 100);

	batteryDoc["temperature"] = connectToModbus(0x3110) / 100.00;
	batteryDoc["percentage"] = connectToModbus(0x311A) / 1;

  int status_charging = connectToModbus(0x3201);

  if (status_charging != -1) {
	  batteryDoc["charging"] = (status_charging >> 2) & 0x03;
  }

  int status = connectToModbus(0x3200);
  if (status != -1) {
    batteryDoc["temp_status"] = (status >> 4) & 0x0F; // D7-D4
    batteryDoc["volt_status"] = status & 0x0F;     // D3-D0
  }

	String jsonString;
	serializeJson(batteryDoc, jsonString);
	return jsonString;
}

String Mppt::getLoadData()
{
  DynamicJsonDocument loadDoc(1024);
	loadDoc["voltage"] = connectToModbus(0x310C) / 100.00;
	loadDoc["current"] = connectToModbus(0x310D) / 100.00;

  // 0x310E (L) + 0x310F (H)
  loadDoc["power"] = connectToModbus32(0x310E, 100);
	loadDoc["status"] = connectToModbus(0x3202);

  // 0x3304 (L) + 0x3305 (H)
  loadDoc["consumed_today"] = connectToModbus32(0x3304, 100);
  // 0x3306 (L) + 0x3307 (H)
  loadDoc["consumed_month"] = connectToModbus32(0x3306, 100);
  // 0x3308 (L) + 0x3309 (H)
  loadDoc["consumed_year"] = connectToModbus32(0x3308, 100);
  // 0x330A (L) + 0x330B (H)
  loadDoc["consumed_total"] = connectToModbus32(0x330A, 100);

	String jsonString;
	serializeJson(loadDoc, jsonString);
	return jsonString;
}

int Mppt::getLoadMode()
{
  uint8_t rc = node.readHoldingRegisters(0x903D, 1);
  if (rc == node.ku8MBSuccess) {
    return node.getResponseBuffer(0); // 0x0000 = Manual
  } else {
    return -1;
  }
}

bool Mppt::setLoadMode(uint16_t mode)
{
  uint8_t rc = node.writeSingleRegister(0x903D, mode);
  return (rc == node.ku8MBSuccess);
}

int Mppt::getManualLoadSwitch()
{
  uint8_t rc = node.readCoils(2, 1); // Adresse C1
  if (rc == node.ku8MBSuccess) {
    return node.getResponseBuffer(0); // 0=aus, 1=ein
  } else {
    Serial.print("❌ Fehler beim Lesen des Manual Load Status: ");
    Serial.println(rc);
    return -1;
  }
}

// Last manuell ein-/ausschalten (C1)
bool Mppt::setManualLoad(bool on)
{
  // 1️⃣ Sicherstellen, dass Load Mode auf Manual steht
  if (getLoadMode() != 0x0000) {
    Serial.println("⚠️ Load Mode nicht Manual, setze auf Manual...");
    if (!setLoadMode(0x0000)) {
      Serial.println("❌ Fehler beim Setzen des Manual Mode");
      return false;
    }
    delay(500); // Warten, bis der Controller den Manual Mode übernommen hat
  }

  // 2️⃣ Last einschalten/ausschalten
  uint8_t rc = node.writeSingleCoil(2, on ? 1 : 0); // C1 Manual Load
  if (rc == node.ku8MBSuccess) {
    Serial.print("🔄 Load ");
    Serial.println(on ? "eingeschaltet ✅" : "ausgeschaltet ✅");
    return true;
  } else {
    Serial.print("❌ Fehler beim Schalten des Load: ");
    Serial.println(rc);
    return false;
  }
}

// Load zuverlässig toggeln
void Mppt::toggleLoad()
{
  int status = getManualLoadSwitch();
  if (status == -1) {
    Serial.println("❌ Fehler beim Lesen des Manual Load Status");
    return;
  }

  bool newStatus = (status == 0); // aus → an, an → aus
  setManualLoad(newStatus);
}

float Mppt::connectToModbus32(uint16_t addressLow, uint16_t times)
{
   uint8_t result = node.readInputRegisters(addressLow, 2);

    if (result == node.ku8MBSuccess) {
      uint16_t low = node.getResponseBuffer(0);
      uint16_t high = node.getResponseBuffer(1);

      // Kombinieren als signed 32-bit Wert
      int32_t combined = ((int32_t)high << 16) | low;

      return combined / (float)times;
    } else {
      Serial.print("Modbus 32bit Error Code: ");
      Serial.println(result, DEC);
      return NAN;
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