#include <Arduino.h>
#include <Preferences.h>
#include <ArduinoOTA.h>

#include "FS.h"
#include <LittleFS.h>

#include <WiFi.h>
#include <WebServer.h>
// NOTE: add wifi_secrets.h with the following format:
// const char* ssid = "SSID";
// const char* password = "PASSWORD";
#include <wifi_secrets.h>

// set pin numbers
const int buttonPin = 12; // G12
const int buttonLedPin = 14; // G14
const int relay1Pin = 16;
const int relay2Pin = 17;

// the last time the relays were turned on
unsigned long relaysLastTurnedOnMillis = 0;

boolean buttonPressed = false;

// the state of both relays
uint8_t relayState = LOW;

boolean timerEnabled = true; // false = disabled, true = enabled

// the amount of time to wait before turning off the relay
long interval = 1000 * 60 * 5; // 5 mins
// const long interval = 1000 * 10 * 1; // 10 seconds

// see https://randomnerdtutorials.com/esp32-web-server-arduino-ide/ and https://lastminuteengineers.com/creating-esp32-web-server-arduino-ide/
WebServer server(80);

Preferences prefs;

void maybeSetButtonLed(uint8_t val) {
  if (digitalRead(buttonLedPin) != val) {
    digitalWrite(buttonLedPin, val);
  }
}

void maybeSetRelays(uint8_t val) {
  if (relayState == val) {
    return;
  }
  Serial.print("Setting relays: ");
  Serial.println(val);
  relayState = val;
  digitalWrite(relay1Pin, val);
  digitalWrite(relay2Pin, val);
}

void ICACHE_RAM_ATTR buttonPress() {
  buttonPressed = true;
}

void handleState() {
  Serial.println("#handleState");
  String str = "{";
  str += "\"relayState\":";
  if (relayState == LOW) { 
    str += "\"off\"";
  } else if (relayState == HIGH) {
    str += "\"on\"";
  } else {
    str += "\"UNKNOWN\"";
  }
  str += ", \"timerState\":";
  if (timerEnabled) {
    str += "\"enabled\"";
  } else {
    str += "\"disabled\"";
  }
  str += ", \"timerDuration\":";
  str += interval / 1000;
  str += "}";
  server.send(200, "application/json", str);
}

void maybeRedirect() {
  if (server.hasArg("source")) {
    server.sendHeader("Location", server.arg("source"), true);
    server.send(302, "text/plain", "");
  } else {
    server.send(200, "text/plain", "ok");
  }
}

void handleEnableTimer() {
  Serial.println("#handleEnableTimer");
  timerEnabled = true;
  maybeRedirect();
}

void handleDisableTimer() {
  Serial.println("#handleDisableTimer");
  timerEnabled = false;
  maybeRedirect();
}

void handleRelaysOn() {
  Serial.println("#handleRelaysOn");
  relaysLastTurnedOnMillis = millis();
  maybeSetRelays(HIGH);
  maybeRedirect();
}

void handleRelaysOff() {
  Serial.println("#handleRelaysOff");
  maybeSetRelays(LOW);
  maybeRedirect();
}

void handleUpdateTimerDuration() {
  Serial.println("#handleUpdateTimerDuration");
  if (server.hasArg("durationSeconds")) {
    interval = server.arg("durationSeconds").toInt() * 1000;
    Serial.println(interval);
    prefs.begin("prefs", false);
    prefs.putLong("interval", interval);
    prefs.end();
  }
    maybeRedirect();
}

void handleIndex(){
  String path = "/index.html";
  File file = LittleFS.open(path);
  if(!file || file.isDirectory()) {
    server.send(500, "text/html", "Could not load: " + path);
  }
  String page = file.readString();
  file.close();

  server.send(200, "text/html", page);
}

void setup() {
  Serial.begin(115200);

  // Prefs
  prefs.begin("prefs", true);
  if (prefs.isKey("interval")) {
    interval = prefs.getLong("interval");
  }
  prefs.end();

  // GPIOs
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(relay1Pin, OUTPUT);
  digitalWrite(relay1Pin, LOW);
  pinMode(relay2Pin, OUTPUT);
  digitalWrite(relay2Pin, LOW);
  pinMode(buttonLedPin, OUTPUT);
  digitalWrite(buttonLedPin, HIGH);
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonPress, FALLING);

  // Filesystem
  if (!LittleFS.begin(true)) {
    Serial.print("LittleFS Mount Failed\n");
    return;
  }

  // WIFI
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

  // OTA
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else {  // U_SPIFFS
        type = "filesystem";
      }

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
      }
    });
  ArduinoOTA.begin();

  // Web Server
  server.begin();
  server.on("/", handleIndex);
  server.on("/disable-timer", handleDisableTimer);
  server.on("/enable-timer", handleEnableTimer);
  server.on("/update-timer-duration", handleUpdateTimerDuration);
  server.on("/force-on", handleRelaysOn);
  server.on("/force-off", handleRelaysOff);
  server.on("/state", handleState);
}

void loop() {
  unsigned long currentMillis = millis();

  // handle button press
  if (buttonPressed) {
    relaysLastTurnedOnMillis = currentMillis;
    maybeSetRelays(HIGH);
    buttonPressed = false;
  }

  // handle timer elapsed
  if (timerEnabled && currentMillis - relaysLastTurnedOnMillis >= interval) {
    maybeSetRelays(LOW);
  }

  // handle LED
  maybeSetButtonLed(!relayState);

  server.handleClient();
  ArduinoOTA.handle();
}

