#ifndef MPPT_H
#define MPPT_H

#include <ArduinoJson.h>
#include <SPI.h>
#include <ModbusMaster.h> // docu https://4-20ma.io/ModbusMaster/group__register.html#ga9094a4770bf9fac0abe2f34aac3a40ec

class Mppt
{
  public:
    Mppt();
    bool setupModbusRTU();
    String getBatteryData();
    String getLoadData();
    String getPVData();
  private:
    int connectToModbus(uint16_t address);
};

#endif