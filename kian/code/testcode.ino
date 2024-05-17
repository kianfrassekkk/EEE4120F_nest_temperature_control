#include <Wire.h>
#include <Adafruit_SHT31.h>
Adafruit_SHT31 sht31 = Adafruit_SHT31();

struct Button {
  const uint8_t PIN;
  bool pressed;
  unsigned long lastDebounceTime;
  unsigned long debounceDelay; // Adjust the debounce delay as needed
};

#include <esp32-hal.h>



Button button1 = {12, false, 0, 300};

struct SHT3xData {
  float temp;
  float humidity;
};



void IRAM_ATTR isr(void) {
  
  if ((millis() - button1.lastDebounceTime) > button1.debounceDelay) {
    button1.pressed = true;
   }
  button1.lastDebounceTime = millis();
 

  
}

void setup() {
  // put your setup code here, to run once:
  
  
  Serial.begin(115200);
  pinMode(button1.PIN, INPUT_PULLUP);
  attachInterrupt(button1.PIN, isr, CHANGE);
 

  loop(); // Add this line to call the loop function
}

void loop() {
  // put your main code here, to run repeatedly:


  if (button1.pressed) {
    SHT3xData data = readSHT3(0x44);

    // Print the sensor readings to the serial monitor
    

    Serial.println("sence 1");
    Serial.print("Temperature: ");
    Serial.print(data.temp);
    Serial.print(" °C, Humidity: ");
    Serial.print(data.humidity);
    Serial.println("%");

    data = readSHT3(0x45);

    // Print the sensor readings to the serial monitor
    Serial.println("sence 2");
    Serial.print("Temperature: ");
    Serial.print(data.temp);
    Serial.print(" °C, Humidity: ");
    Serial.print(data.humidity);
    Serial.println("%");
    button1.pressed = false;
  }
}


SHT3xData readSHT3(int address) {
  SHT3xData data;
  if (!sht31.begin(address)) {
    Serial.println("Couldn't find SHT31");
    data.temp = 0.0;
    data.humidity = 0.0;
  } else {
    float temp = sht31.readTemperature();
    float humidity = sht31.readHumidity();
    if (isnan(temp)) {
      data.temp = 0.0;
    } else {
      data.temp = temp;
    }
    if (isnan(temp)) {
      data.humidity = 0.0;
    } else {
      data.humidity = humidity;
    }

  }
  return data;
}