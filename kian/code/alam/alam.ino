// SHT31 - read and set high alert temperature set and clear and test

// see https://sensirion.com/products/catalog/SHT31-DIS-B/
// datasheet and alert mode application note

// to use this code the file Docements>Arduino>libraries>Adafruit_SHT31_Library>Adafruit_SHT31.h
//    requires line 66 editing to comment out the private keyword, e.g
//  //private:
//  this gives the code below access to the private members of class Adafruit_SHT31

#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_SHT31.h"

// parameters for test
#define alertPin 13                       // sht31 alert pin


float highAlertTemperatureSet = 29.0f;    // high alert set
float highAlertTemperatureClear = 26.0f;  // high alert cleared

Adafruit_SHT31 sht31 = Adafruit_SHT31();

// alert interrupt ISR - set alertFlag on change of state
volatile int alertFlag = 0;
void alertISR(void) {
  alertFlag = 1;  // set alert indicator
}

void setup() {
  Serial.begin(115200);
  if (!sht31.begin(0x44)) {  // Set to 0x45 for alternate I2C address
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }
  Serial.println("\n\nsht31 alert limit tests");
  sht31.reset();
  float HighAlertLimitSetTemperature = 60, HighAlertLimitClearTemperature = 0, LowAlertLimitSetTemperature = -10,
        HighAlertLimitSetHumidity = 80, HighAlertLimitClearHumidity = 79, LowAlertLimitSetHumidity = 10;
  // check high and low alert temperatures - should be +60 and -10
  readHighAlertLimitSet(HighAlertLimitSetTemperature, HighAlertLimitSetHumidity);
  Serial.print("high alert limit set temperature (should be 60.00) = ");
  Serial.print(HighAlertLimitSetTemperature);
  Serial.print("  humidity (should be 80.00) = ");
  Serial.println(HighAlertLimitSetHumidity);
  readHighAlertLimitClear(HighAlertLimitClearTemperature, HighAlertLimitClearHumidity);
  Serial.print("high alert limit clear temperature (should be 58.00) = ");
  Serial.print(HighAlertLimitClearTemperature);
  Serial.print("  humidity (should be 79.00) = ");
  Serial.println(HighAlertLimitClearHumidity);
  readLowAlertLimitSet(LowAlertLimitSetTemperature, LowAlertLimitSetHumidity);
  Serial.print("low alert limit set(should be -10.00) = ");
  Serial.print(LowAlertLimitSetTemperature);
  Serial.print("  humidity (should be 22.00) = ");
  Serial.println(LowAlertLimitSetHumidity);

  // set the high temperature alert set and clear
  writeHighAlertLimitSet(highAlertTemperatureSet);  // set high alert temperature level and check it
  readHighAlertLimitSet(HighAlertLimitSetTemperature, HighAlertLimitSetHumidity);
  Serial.print("\nnew high alert limit set temperature  = ");
  Serial.print(highAlertTemperatureSet = HighAlertLimitSetTemperature);
  Serial.print("  humidity  ");
  Serial.println(HighAlertLimitSetHumidity);
  writeHighAlertLimitClear(highAlertTemperatureClear);
  readHighAlertLimitClear(HighAlertLimitClearTemperature, HighAlertLimitClearHumidity);
  Serial.print("new high alert limit clear temperature = ");
  Serial.print(highAlertTemperatureClear = HighAlertLimitClearTemperature);
  Serial.print("  humidity  = ");
  Serial.println(HighAlertLimitClearHumidity);
  uint16_t status = sht31.readStatus();
  Serial.print("status ");
  Serial.println(status, HEX);
  clearStatusRegister();                                               // clear status bits
  setPeriodicDataAcquisitionMode();                                    // set Periodic Data Acquisition Mode
  attachInterrupt(digitalPinToInterrupt(alertPin), alertISR, CHANGE);  // set up interrupt
}

// loop reading temperature and humidity and check for alerts
void loop() {
  uint16_t status = sht31.readStatus();  // read status bits
  Serial.print("status ");
  Serial.print(status, HEX);  // read temperature and humidity
  float temperature, humidity;
  readTempHum(temperature, humidity);
  if (!isnan(temperature)) {  // check if 'is not a number'
    Serial.print("  Temp *C = ");
    Serial.print(temperature);
    Serial.print("\t\t");
  } else {
    Serial.println("Failed to read temperature");
  }
  if (!isnan(humidity)) {  // check if 'is not a number'
    Serial.print("Hum. % = ");
    Serial.println(humidity);
  } else {
    Serial.println("Failed to read humidity");
  }
  // delay checking alertFlag
  long timer = millis();
  bool printedAlert = 0;
  while (millis() - timer < 3000) {
    if (alertFlag) {  // if alert occured
      alertFlag = 0;
      if (!printedAlert) {  // if not already printed
        printedAlert = 1;
        Serial.print("******* alert !! *******  status ");
        status = sht31.readStatus();
        Serial.print(status, HEX);
        Serial.print(" alert pin state  = ");
        int alertPinState = digitalRead(alertPin);
        Serial.println(alertPinState);
        if (alertPinState)
          Serial.println("   *** high temperature alert set !  **** ");
        else
          Serial.println("   *** high temperature alert cleared !  **** ");
        clearStatusRegister();
      }
    }
  }
}

// set up setPeriodic Data Acquisition Mode for alerts
void setPeriodicDataAcquisitionMode(void) {
  sht31.writeCommand(0x2126);  // set repeatability mode
  delay(20);
}

void clearStatusRegister(void) {
  sht31.writeCommand(0x3041);  // clear status register
  delay(20);
}

// read high alert set limit
bool readHighAlertLimitSet(float &temp, float &humidity) {
  Serial.println("reading high alert limit set ");
  uint8_t readbuffer[3] = { 0 };
  sht31.writeCommand(0xE11F);  // read high alert set
  delay(20);
  sht31.i2c_dev->read(readbuffer, sizeof(readbuffer));
  if (readbuffer[2] != crc8(readbuffer, 2)) {
    Serial.println("CRC error!");
    return false;
  }
  Serial.print("readHigAlertLimitSet 0x");
  Serial.print(readbuffer[0], HEX);
  Serial.print(readbuffer[1], HEX);
  Serial.println(readbuffer[2], HEX);
  // temperature convert 9 bit value to 16bit value
  int32_t stemp = ((int32_t)(((uint32_t)((readbuffer[0] & 0x01) << 8)) | readbuffer[1])) << 7;
  // convert from 16bit value
  stemp = 100.0f * ((stemp * 175.0f) / 65535.0f - 45.0f);
  temp = (float)stemp / 100.0f;
  // humidity convert 7 bit value to 16 bit value
  uint32_t shum = (uint32_t)((uint32_t)(readbuffer[0] & 0xFE) << 8);
  humidity = (shum * 100.0f) / 65535.0f;
  return true;
}

// read high alert clear limit
bool readHighAlertLimitClear(float &temp, float &humidity) {
  Serial.print("reading high alert limit clear ");
  uint8_t readbuffer[3] = { 0 };
  sht31.writeCommand(0xE114);  // read high alert set
  delay(20);
  sht31.i2c_dev->read(readbuffer, sizeof(readbuffer));
  if (readbuffer[2] != crc8(readbuffer, 2)) {
    Serial.println("CRC error!");
    return false;
  }
  Serial.print(" 0x");
  Serial.print(readbuffer[0], HEX);
  Serial.print(readbuffer[1], HEX);
  Serial.println(readbuffer[2], HEX);
  // temperature convert 9 bit value to 16bit value
  int32_t stemp = ((int32_t)(((uint32_t)((readbuffer[0] & 0x01) << 8)) | readbuffer[1])) << 7;
  // convert from 16bit value
  stemp = 100.0f * ((stemp * 175.0f) / 65535.0f - 45.0f);
  temp = (float)stemp / 100.0f;
  uint32_t shum = (uint32_t)((uint32_t)(readbuffer[0] & 0xFE) << 8);
  humidity = (shum * 100.0f) / 65535.0f;
  return true;
}

// write high alert set temperature
void writeHighAlertLimitSet(float temp) {
  Serial.print("write high alert limit set ");
  Serial.println(temp);
  // calculate 9 bit value for temperature
  int32_t stemp = (temp + 45.0f) * 65535.0f / 175.0f;
  stemp = stemp >> 7;
  //Serial.print("setHighAlertLimit 0x");
  //Serial.println(stemp,HEX);
  // buffer to set high alert - command is 0x611D
  uint8_t writebuffer[5] = { 0x61, 0x1D, ((stemp >> 8) & 0x01) | 0xCC, stemp & 0xff, 0 };
  writebuffer[4] = crc8(writebuffer + 2, 2);
  //Serial.print(writebuffer[2], HEX);
  //Serial.print(writebuffer[3], HEX);
  //Serial.println(writebuffer[4], HEX);
  sht31.i2c_dev->write(writebuffer, sizeof(writebuffer));  // write updated value
  delay(20);
  sht31.writeCommand(0x2126);  // set repeatability
  delay(20);
}


// write high alert clear temperature
void writeHighAlertLimitClear(float temp) {
  Serial.print("write high alert limit clear ");
  Serial.println(temp);
  // calculate 9 bit value for temperature
  int32_t stemp = (temp + 45.0f) * 65535.0f / 175.0f;
  stemp = stemp >> 7;
  //Serial.print("setHighAlertLimit 0x");
  //Serial.println(stemp,HEX);
  // buffer to set high alert - command is 0x611D
  uint8_t writebuffer[5] = { 0x61, 0x16, ((stemp >> 8) & 0x01) | 0xc8, stemp & 0xff, 0 };
  writebuffer[4] = crc8(writebuffer + 2, 2);
  //Serial.print(writebuffer[2], HEX);
  //Serial.print(writebuffer[3], HEX);
  //Serial.println(writebuffer[4], HEX);
  sht31.i2c_dev->write(writebuffer, sizeof(writebuffer));  // write updated value
  delay(20);
  sht31.writeCommand(0x2126);  // set repeatability
  delay(20);
}

// read low alert set temperature
bool readLowAlertLimitSet(float &temp, float &humidity) {
  Serial.println("reading low alert limit set ");
  uint8_t readbuffer[3] = { 0 };
  sht31.writeCommand(0xE102);  // read low alert set
  delay(20);
  sht31.i2c_dev->read(readbuffer, sizeof(readbuffer));
  if (readbuffer[2] != crc8(readbuffer, 2)) {
    Serial.println("CRC error!");
    return false;
  }
  Serial.print("readLowAlertLimitSet 0x");
  Serial.print(readbuffer[0], HEX);
  Serial.print(readbuffer[1], HEX);
  Serial.println(readbuffer[2], HEX);
  // temperature convert 9 bit value to 16bit value
  int32_t stemp = ((int32_t)(((uint32_t)((readbuffer[0] & 0x01) << 8)) | readbuffer[1])) << 7;
  // temperature convert from 16bit value
  stemp = 100.0f * ((stemp * 175.0f) / 65535.0f - 45.0f);
  temp = (float)stemp / 100.0f;
  // humidity convert 7 bit value to 16 bit value
  uint32_t shum = (uint32_t)((uint32_t)(readbuffer[0] & 0xFE) << 8);
  humidity = (shum * 100.0f) / 65535.0f;
  return true;
}

static uint8_t crc8(const uint8_t *data, int len) {
  /*
   *
   * CRC-8 formula from page 14 of SHT spec pdf
   *
   * Test data 0xBE, 0xEF should yield 0x92
   *
   * Initialization data 0xFF
   * Polynomial 0x31 (x8 + x5 +x4 +1)
   * Final XOR 0x00
   */

  const uint8_t POLYNOMIAL(0x31);
  uint8_t crc(0xFF);

  for (int j = len; j; --j) {
    crc ^= *data++;

    for (int i = 8; i; --i) {
      crc = (crc & 0x80) ? (crc << 1) ^ POLYNOMIAL : (crc << 1);
    }
  }
  return crc;
}

// read data in Periodic Data Acquisition Mode
bool readTempHum(float &temp, float &humidity) {
  uint8_t readbuffer[6];
  sht31.writeCommand(0xE000);  // Fetch Data command
  delay(20);
  sht31.i2c_dev->read(readbuffer, sizeof(readbuffer));
  if (readbuffer[2] != crc8(readbuffer, 2) || readbuffer[5] != crc8(readbuffer + 3, 2))
    return false;
  int32_t stemp = (int32_t)(((uint32_t)readbuffer[0] << 8) | readbuffer[1]);
  // simplified (65536 instead of 65535) integer version of:
  // temp = (stemp * 175.0f) / 65535.0f - 45.0f;
  stemp = ((4375 * stemp) >> 14) - 4500;
  temp = (float)stemp / 100.0f;
  uint32_t shum = ((uint32_t)readbuffer[3] << 8) | readbuffer[4];
  // simplified (65536 instead of 65535) integer version of:
  // humidity = (shum * 100.0f) / 65535.0f;
  shum = (625 * shum) >> 12;
  humidity = (float)shum / 100.0f;
  return true;
}
