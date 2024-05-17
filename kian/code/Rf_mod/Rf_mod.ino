const int ledPin = 2;   // LED pin
const int buttonPin = 4; // Button pin
const int outputres = 26; // Button pin

volatile bool ledState = LOW; // LED state variable (volatile for interrupt safety)

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP); // Set button pin with internal pull-up resistor
  pinMode(outputres, OUTPUT);

  // Attach interrupt to button pin on rising edge (button press)
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonPressISR, RISING);
  digitalWrite(outputres, HIGH);
}

void loop() {
  // No code needed in the loop here (interrupt handles LED toggling)
  // You can add other functionalities that don't rely on button presses
  // Serial.println("changed");
  // digitalWrite(outputres, !digitalRead(outputres));
  // delay(10000);
  
 

}

void buttonPressISR() {
  ledState = !ledState; // Toggle LED state
  digitalWrite(ledPin, ledState);
}


