#include <Arduino.h>

// set pin numbers
const int buttonPin = 12;     // the number of the pushbutton pin
const int relayPin =  5;       // the number of the relay pin

// the last time the button was pressed
unsigned long previousMillis = 0;

// the amount of time to wait before turning off the relay
const long interval = 1000 * 60 * 5; // 5 mins
// const long interval = 1000 * 10 * 1; // 10 seconds

void ICACHE_RAM_ATTR buttonPressed() {
  previousMillis = millis();
  if (digitalRead(relayPin) == LOW) {
    Serial.print("Turning on relay\n");
    digitalWrite(relayPin, HIGH);
  }
}

void setup() {
  Serial.begin(9600);
  // initialize the button pin as an input
  pinMode(buttonPin, INPUT_PULLUP);
  // initialize the relay pin as an output
  pinMode(relayPin, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonPressed, FALLING);

  Serial.print("\nhello\n");
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval && digitalRead(relayPin) == HIGH) {    
    Serial.print("turning off relay\n");
    digitalWrite(relayPin, LOW);
  }
}
