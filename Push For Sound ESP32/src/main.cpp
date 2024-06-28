#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
// NOTE: add wifi_secrets.h with the following format:
// const char* ssid = "SSID";
// const char* password = "PASSWORD";
#include <wifi_secrets.h>

// set pin numbers
const int buttonPin = 12; // G12
const int relay1Pin = 16;
const int relay2Pin = 17;

// the last time the button was pressed
unsigned long previousMillis = 0;

// the state of both relays
uint8_t relayState = LOW;

boolean timerState = true; // false = disabled, true = enabled

// the amount of time to wait before turning off the relay
const long interval = 1000 * 60 * 5; // 5 mins
// const long interval = 1000 * 10 * 1; // 10 seconds

// see https://randomnerdtutorials.com/esp32-web-server-arduino-ide/ and https://lastminuteengineers.com/creating-esp32-web-server-arduino-ide/
WebServer server(80);

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

void handleIndex() {
  Serial.println("#handleIndex");

  String str = "<!DOCTYPE html> <html>\n";
  str +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  str +="<title>Push to play</title>\n";
  str +="<style>";
  str +="</style>\n";
  str +="</head>\n";
  str +="<body>\n";
  str +="<h1>Push to play config</h1>\n";
  str += "<p>";
  str += "Relay state: ";
  if (relayState == LOW) { 
    str += "off";
  } else if (relayState == HIGH) {
    str += "on";
  } else {
    str += "UNKNOWN";
  }
  str += "</p>";
  str += "<p>";
  str += "Timer state: ";
  if (timerState) {
    str += "enabled";
  } else {
    str += "disabled";
  }
  str += "</p>";
  str += "<p><a href=\"/enable-timer\">Enable timer mode</a>";
  str += "</br>";
  str += "<p><a href=\"/disable-timer\">Disable timer mode</a>";
  str += "</br>";
  str += "<p><a href=\"/force-on\">Force relays on</a>";
  str += "</br>";
  str += "<p><a href=\"/force-off\">Force relays off</a>";
  str +="</body>\n";
  str +="</html>\n";

  server.send(200, "text/html", str);  
}

void handleEnableTimer() {
  Serial.println("#handleEnableTimer");
  timerState = true;
  server.sendHeader("Location", "/", true);  
  server.send(302, "text/plain", "");
}

void handleDisableTimer() {
  Serial.println("#handleDisableTimer");
  timerState = false;
  server.sendHeader("Location", "/", true);  
  server.send(302, "text/plain", "");
}

void handleRelaysOn() {
  Serial.println("#handleRelaysOn");
  timerState = false;
  setRelays(HIGH);
  server.sendHeader("Location", "/", true);  
  server.send(302, "text/plain", "");
}

void handleRelaysOff() {
  Serial.println("#handleRelaysOff");
  timerState = false;
  setRelays(LOW);
  server.sendHeader("Location", "/", true);  
  server.send(302, "text/plain", "");
}

void setup() {
  Serial.begin(115200);

  pinMode(buttonPin, INPUT_PULLUP);

  pinMode(relay1Pin, OUTPUT);
  pinMode(relay2Pin, OUTPUT);
  setRelays(LOW);

  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonPressed, FALLING);

  Serial.print("\nHardware setup Complete!\n");

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();

  server.on("/", handleIndex);
  server.on("/disable-timer", handleDisableTimer);
  server.on("/enable-timer", handleEnableTimer);
  server.on("/force-on", handleRelaysOn);
  server.on("/force-off", handleRelaysOff);
}

void loop() {
  unsigned long currentMillis = millis();

  if (timerState && currentMillis - previousMillis >= interval && relayState == HIGH) {    
    setRelays(LOW);
  }

  server.handleClient();
}

