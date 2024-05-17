// the nest

typedef enum {
  NORMAL_OPERATION,
  WIFI_OPERATION,
  ALARM_STATE
} SystemState;

// Usage in your code
SystemState currentState = NORMAL_OPERATION;

//sd card
#include "FS.h"
#include "SD.h"
#include "SPI.h"


#define MAX_BUFFER_SIZE 128           // Buffer size for formatted data
const char* DATA_FILE_NAME = "/test1.txt";

//for senser
#include <Wire.h>
#include <Adafruit_SHT31.h>
Adafruit_SHT31 sht31 = Adafruit_SHT31();

float highAlertTemperatureSet = 25.0f;    // high alert set
float highAlertTemperatureClear = 25.0f;  // high alert cleared

//#include <esp32-hal.h>

struct SHT3xData {
  float temp;
  float humidity;
};


// for sleep mode
#define BUTTON_PIN_BITMASK 0x300000000

#include "esp_sleep.h"

// setting up timer

#include <ESP32Time.h>

#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  10     /* Time ESP32 will go to sleep (in seconds) */

ESP32Time rtc;




//pin const
const int ledPin = 2;   // LED pin
const int SDcardpin = 17;

// function for wifi

// Import required libraries
#include "WiFi.h"
#include "ESPAsyncWebServer.h"


// Set your access point network credentials
const char* ssid = "ESP32-Access-Point";
const char* password = "123456789";

AsyncWebServer server(80);





//function to set up SD card

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if(!root){
    Serial.println("Failed to open directory");
    return;
  }
  if(!root.isDirectory()){
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if(levels){
        listDir(fs, file.name(), levels -1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void createDir(fs::FS &fs, const char * path){
  Serial.printf("Creating Dir: %s\n", path);
  if(fs.mkdir(path)){
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}

void removeDir(fs::FS &fs, const char * path){
  Serial.printf("Removing Dir: %s\n", path);
  if(fs.rmdir(path)){
    Serial.println("Dir removed");
  } else {
    Serial.println("rmdir failed");
  }
}

void readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while(file.available()){
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file){
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)){
      Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}

void deleteFile(fs::FS &fs, const char * path){
  Serial.printf("Deleting file: %s\n", path);
  if(fs.remove(path)){
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}
void writeDataToSD(float temp1, float humidity1, float temp2, float humidity2, const char * cause) {
  
  // Initialize RTC (assuming you're using a DS3231 module)
  

  // Get current time from RTC
  
  String formattedTime = rtc.getTime("%A, %B %d %Y %H:%M:%S");
  String dataLine = formattedTime + ", temp1 ," + String(temp1) + ", humd1 ," + String(humidity1) + ", temp2 ," + String(temp2) + ", humd2 ," + String(humidity2) +", wake up reason ," + cause;
  // Check if the file exists
  if (SD.exists(DATA_FILE_NAME)) {
    // File exists, append data with timestamp
    appendFile(SD, DATA_FILE_NAME, "\n");    
    appendFile(SD, DATA_FILE_NAME, dataLine.c_str());
  } else {
    // File doesn't exist, create it with header including time
    writeFile(SD, DATA_FILE_NAME, "Time,Temperature1,Humidity1,Temperature2,Humidity2\n");
    writeFile(SD, DATA_FILE_NAME, dataLine.c_str());
  }
}
//sleep mod function

void wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default :
      Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
      rtc.setTime(30, 24, 15, 17, 1, 2024);  // 17th Jan 2021 15:24:30
      //rtc.setTime(1609459200);  // 1st Jan 2021 00:00:00
      //rtc.offset = 7200; // change offset value

      break;
  }
}



void onwakeuptimer(){

  Serial.println("timer wake up");

  SHT3xData data = readSHT3(0x44);

  // Print the sensor readings to the serial monitor
    
  Serial.println("sence 1");
  Serial.print("Temperature: ");
  Serial.print(data.temp);
  Serial.print(" °C, Humidity: ");
  Serial.print(data.humidity);
  Serial.println("%");
  float temp1 = data.temp;
  float humd1 =data.humidity;

  

  data = readSHT3(0x45);

  // Print the sensor readings to the serial monitor
  Serial.println("sence 2");
  Serial.print("Temperature: ");
  Serial.print(data.temp);
  Serial.print(" °C, Humidity: ");
  Serial.print(data.humidity);
  Serial.println("%");

  float temp2 = data.temp;
  float humd2 =data.humidity;

  writeDataToSD(temp1,humd1,temp2,humd2, "timer");

}
void onwakeupallarm(){
Serial.println("allarm !!!");
}
bool wifi = false;
void onwakeuperf(){
  wifisetup();
  wifisend();
  int counter =0; 
  wifi = true;
}
//sensers

// Function to read data from SHT31 sensor
SHT3xData readSHT3(uint8_t address) {
  SHT3xData data;
  if (!sht31.begin(address)) {
    Serial.println("Couldn't find SHT31 sensor!");
    return data; // Return empty data if sensor not found
  }
  data.temp = sht31.readTemperature();
  data.humidity = sht31.readHumidity();
  return data;
}
//write high alert set temperature
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
// functions for wifi
void wifisetup(){
   // Setting the ESP as an access point
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  
}
void wifisend(){
  // Define filename without forward slash
  

  server.on("/DATA", HTTP_GET, [&](AsyncWebServerRequest *request){
    // Check if the file exists
    if (SD.exists(DATA_FILE_NAME)) {
      // Open the file for reading
      File dataFile = SD.open(DATA_FILE_NAME);
      if (dataFile) {
        String data = dataFile.readString();
        dataFile.close();

        // Send the data as plain text with appropriate content type
        request->send(200, "text/plain", data);
      } else {
        // File open failed, send error message
        request->send(500, "text/plain", "Failed to open data file.");
      }
    } else {
      // File doesn't exist, send error message
      request->send(404, "text/plain", "Data file not found.");
    }
  });

  server.begin();
  Serial.println("Web server started!");
}
//main functions
void setup() {
  //genral setup 
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);// internal led used to indicat that it is working

  //SD card set up
 
  pinMode(SDcardpin , OUTPUT); // enabling power
  gpio_pulldown_en((gpio_num_t)SDcardpin);
  digitalWrite(SDcardpin, HIGH); //terning on the lcd

  if(!SD.begin(5)){
    Serial.println("Card Mount Failed");
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
  }

  // set up temp senseres
  
  // writeHighAlertLimitSet(highAlertTemperatureSet);
  // writeHighAlertLimitClear(highAlertTemperatureSet);



  
 
  //sleep mode setup
   wakeup_reason();
  Serial.println(rtc.getTime("%A, %B %d %Y %H:%M:%S"));   // (String) returns time with specified format
  
  //esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,1); //1 = High, 0 = Low
  

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
 
 
}
void loop() 
  switch (currentState) {
    case NORMAL_OPERATION:
      onwakeuptimer();
      Serial.println("Going to sleep now");
      Serial.flush();
      esp_deep_sleep_start();
      break;
    case WIFI_OPERATION:
      onwakeuperf();
      delay(40000);
      break;
    case ALARM_STATE:
      onwakeupallarm();
      delay(4000)
    break;
  }  

}
