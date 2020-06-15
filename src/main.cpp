#include <Arduino.h>
#include "MCP23017.h"
#include "Buttons.h"

void halt(const __FlashStringHelper *msg) {
  Serial.println(msg);
  Serial.flush();
  ESP.deepSleep(0);
}

MCP23017 mcp;
Buttons btns(mcp, D5);

void setup() {
  Serial.begin(115200);
  Serial.println();

  mcp.init();
  if (! mcp.begin())
    halt(F("MCP23017 not found!"));
  mcp.pinMode(4, OUTPUT);
  mcp.digitalWrite(4, LOW);
  btns.addButton(0, LOW);
  btns.addButton(1, LOW);
  btns.addButton(2, LOW);
  btns.addButton(3, LOW);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  while (Buttons::event_t *evt = (Buttons::event_t*)btns.getEvent()) {
    digitalWrite(LED_BUILTIN, LOW);
    Serial.print(F("Button #"));
    Serial.print(evt->data);
    Serial.print(' ');
    switch (evt->kind) {
      case Buttons::BTN_RELEASED:
        Serial.println(F("released"));
        break;
      case Buttons::BTN_PRESSED:
        Serial.println(F("pressed"));
        break;
      case Buttons::BTN_CLICK:
        Serial.println(F("clicked"));
        break;
      case Buttons::BTN_DBLCLICK:
        Serial.println(F("double clicked"));
        break;
      case Buttons::BTN_LONGCLICK:
        Serial.println(F("long clicked"));
        break;
    }
    digitalWrite(LED_BUILTIN, HIGH);
  }
  delay(1);
}
