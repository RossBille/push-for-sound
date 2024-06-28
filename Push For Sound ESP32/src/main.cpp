#include <Arduino.h>

// set pin numbers
const int buttonPin = 12; // G12
const int relay1Pin = 16;
const int relay2Pin = 17;

// the last time the button was pressed
unsigned long previousMillis = 0;

// the state of both relays
uint8_t relayState = LOW;

// the amount of time to wait before turning off the relay
const long interval = 1000 * 60 * 5; // 5 mins
// const long interval = 1000 * 10 * 1; // 10 seconds

void setRelays(uint8_t val) {
    Serial.print("Setting relays: ");
    Serial.println(val);
    relayState = val;
    digitalWrite(relay1Pin, val);
    digitalWrite(relay2Pin, val);
}

void ICACHE_RAM_ATTR buttonPressed() {
  previousMillis = millis();
  if (relayState == LOW) {
    setRelays(HIGH);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(buttonPin, INPUT_PULLUP);

  pinMode(relay1Pin, OUTPUT);
  pinMode(relay2Pin, OUTPUT);
  setRelays(LOW);

  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonPressed, FALLING);

  Serial.print("\nSetup Complete!\n");
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval && relayState == HIGH) {    
    setRelays(LOW);
  }
}

