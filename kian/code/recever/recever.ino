// the hand held device

// set up wifi 
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "ESP32-Access-Point";
const char* password = "123456789";

//Your IP address or domain name with URL path
const char* serverNameTemp = "http://192.168.4.1/DATA";

// for sleep mode
#define BUTTON_PIN_BITMASK 0x200000000 // 2^33 in hex set up the eternal pin trigger

#include "esp_sleep.h"

//sd card
#include "FS.h"
#include "SD.h"
#include "SPI.h"

// For the LCD
#include <LiquidCrystal_I2C.h>   


const int ledPin = 2;   // LED pin
const int RFsendpin = 27; 
const int SDcardpin = 15;
const int LCDPIN =4;

// Debouncing variables
volatile unsigned long lastDebounceTime = 0;  // millisecond timestamp for debounce
const int debounceDelay = 50;                 // debounce delay in milliseconds


LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x3F for a 16 chars and 2 line display     // Make sure backlight is on



/*
Method to print the reason by which ESP32
has been awaken from sleep
*/

// Function to write a string to the LCD
bool printToLCD(const String& messageTop, const String& messageBottom = "") {
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print(messageTop);

  if (!messageBottom.isEmpty()) {
    lcd.setCursor(2, 1);
    lcd.print(messageBottom);
  }
  return true;  // You can return a value to indicate success/failure (optional)
}

void drawProgressBar(int progressLevel, const String& messageTop) {
  lcd.clear();  // Clear the entire LCD instead of just the first line
  lcd.setCursor(2, 0);
  lcd.print(messageTop);
  // Move cursor to the beginning of the second line
  lcd.setCursor(0, 1);

  lcd.print("Progress: [");


  for (int i = 0; i < progressLevel; i++) {
    lcd.setCursor(11+i, 1);
    lcd.print(">");
  }

  // for (int i = progressLevel; i < 5; i++) {
  //   lcd.print(' ');
  // }
  lcd.setCursor(15, 1);
  lcd.print("]");
}

// function to send a transmition paluse
bool RFsend() {
  // Simulate signal transmission 20 times
  int prog;
  for (int i = 0; i < 20; i++) {
    digitalWrite(RFsendpin, HIGH); // Turn on signal (replace with actual transmission)
    delay(100); // Delay for 1 millisecond (adjust as needed)
    digitalWrite(RFsendpin, LOW); // Turn off signal
    delay(100); // Delay for 1 millisecond (adjust as needed)
    
    //for loding bar
    if (i % 5 == 0) {
      prog = i / 5;
    } else {
      prog = (i / 5) + 1;
    }
    Serial.print(prog);
    drawProgressBar(prog, "RF send");
   
  }
  return true; // Simulate successful transmission (replace with actual success check)
}

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

void testFileIO(fs::FS &fs, const char * path){
  File file = fs.open(path);
  static uint8_t buf[512];
  size_t len = 0;
  uint32_t start = millis();
  uint32_t end = start;
  if(file){
    len = file.size();
    size_t flen = len;
    start = millis();
    while(len){
      size_t toRead = len;
      if(toRead > 512){
        toRead = 512;
      }
      file.read(buf, toRead);
      len -= toRead;
    }
    end = millis() - start;
    Serial.printf("%u bytes read for %u ms\n", flen, end);
    file.close();
  } else {
    Serial.println("Failed to open file for reading");
  }


  file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for writing");
    return;
  }

  size_t i;
  start = millis();
  for(i=0; i<2048; i++){
    file.write(buf, 512);
  }
  end = millis() - start;
  Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
  file.close();
}


//on wake up 
void on_wacke_up(){
  //step on
  printToLCD("woken up");
  wifisetup();
  fetchDataToSave(serverNameTemp);
  delay(3333);

  

  // trigger rf
   RFsend();

  


  digitalWrite(ledPin, HIGH);
  delay(3333);
  digitalWrite(ledPin, LOW);
}

// wifi resever
void wifisetup(){
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  
  printToLCD("Connect to wifi");
  while(WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

// Function to fetch data from web server and save to SD card
bool fetchDataToSave(const char* serverUrl) {
  HTTPClient http;

  // Set server address
  http.begin(serverUrl);

  // Send HTTP GET request
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    // Check for successful response code (e.g., 200)
    if(httpResponseCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println("Payload: ");
      Serial.println(payload);

      // Open file for writing on SD card
      File dataFile = SD.open("/DATA.txt", FILE_WRITE);
      if (dataFile) {
        Serial.println("Writing data to SD card...");
        dataFile.print(payload);
        dataFile.close();
        Serial.println("Data saved successfully!");
        return true;
      } else {
        Serial.println("Failed to open data file for writing");
      }
    } else {
      Serial.printf("Error code %d from server\n", httpResponseCode);
    }
  } else {
    Serial.println("Server connection failed");
  }

  http.end();
  return false;
}


void setup(){

  //genral setup 
  Serial.begin(115200);
  delay(1000); //Take some time to open up the Serial Monitor
  pinMode(ledPin, OUTPUT);// internal led used to indicat that it is working

  // lCD set up
  lcd.init();
  lcd.backlight();
  lcd.clear();
  pinMode(LCDPIN , OUTPUT); // enabling power
  digitalWrite(LCDPIN, HIGH); //terning on the lcd
  gpio_pulldown_en((gpio_num_t)LCDPIN);// for the power switch

  //SD card set up
  pinMode(SDcardpin , OUTPUT); // enabling power
  digitalWrite(SDcardpin, HIGH); //terning on the lcd
  gpio_pulldown_en((gpio_num_t)SDcardpin);// for the power switch

  if(!SD.begin(5)){
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);

  // set up rf send
  pinMode(RFsendpin, OUTPUT);

  listDir(SD, "/", 0);
  writeFile(SD, "/DATA.txt", ""); // rewright the data file incase it wat deleted
  
 
  //sleep mode setup
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,1); //1 = High, 0 = Low
  

  
}

void loop(){
  
  
 

  on_wacke_up();
  esp_deep_sleep_start(); 
  // digitalWrite(ledPin, HIGH);
  // printToLCD("hello");
  // delay(3333);
  // digitalWrite(ledPin, LOW);
  // wrighttodatafile("temp, ,humidity , ,");
  // readFile(SD, "/DATA.txt");
  // delay(3333);
  
  
}

