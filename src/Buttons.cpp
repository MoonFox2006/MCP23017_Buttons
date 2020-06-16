#include <Arduino.h>
#include "Buttons.h"

uint8_t Buttons::addButton(uint8_t pin, bool level) {
  if (pin < 16) {
    button_t btn;
    uint8_t result;

    btn.pin = pin;
    btn.pressed = false;
    btn.dbl = false;
    btn.duration = 0;
    result = add(btn);
    if (result != ERR_INDEX) {
      _mcp->pinMode(pin, level ? INPUT : INPUT_PULLUP);
      _mcp->setupInterrupt(pin, true);
      if (_mcp->digitalRead(pin)) {
        _items[result].pressed = true;
        putEvent(BTN_PRESSED, result);
        if (_lastISR) {
          uint32_t t = millis() - _lastISR;

          if (t) {
            for (uint8_t i = 0; i < result; ++i) {
              if (_items[i].pressed) {
                if (t + _items[i].duration >= 0x03FF) // 2^10
                  _items[i].duration = 0x03FF;
                else
                  _items[i].duration += t;
              } else if (_items[i].dbl) {
                if (t >= _items[i].duration) {
                  _items[i].dbl = false;
                  _items[i].duration = 0;
                } else
                  _items[i].duration -= t;
              }
            }
          }
        }
        _lastISR = millis();
      }
    }
    return result;
  }
  return ERR_INDEX;
}

bool Buttons::putEvent(eventkind_t kind, uint8_t data) {
  event_t e;

  e.kind = kind;
  e.data = data;
  return _events.put(&e);
}

bool Buttons::match(uint8_t index, const void *t) {
  return (_items[index].pin == ((button_t*)t)->pin);
}

void Buttons::mcpCallback(uint16_t pins, uint16_t values) {
  uint32_t time, t;

  time = millis();
  if (_lastISR)
    t = time - _lastISR;
  else
    t = 0;
  for (uint8_t i = 0; i < _count; ++i) {
    button_t &b = _items[i];

    if (t) {
      if (b.pressed) {
        if (t + b.duration >= 0x03FF) // 2^10
          b.duration = 0x03FF;
        else
          b.duration += t;
      } else if (b.dbl) {
        if (t >= b.duration) {
          b.dbl = false;
          b.duration = 0;
        } else
          b.duration -= t;
      }
    }

    if (values & (1 << b.pin)) {
      if (! b.pressed) {
        b.pressed = true;
        b.duration = 0;
        putEvent(BTN_PRESSED, i);
      }
    } else {
      if (b.pressed) {
        if (b.duration >= LONGCLICK_TIME)
          putEvent(BTN_LONGCLICK, i);
        else if (b.duration >= CLICK_TIME) {
          if (b.dbl)
            putEvent(BTN_DBLCLICK, i);
          else
            putEvent(BTN_CLICK, i);
        } else
          putEvent(BTN_RELEASED, i);
        b.pressed = false;
        b.dbl = true;
        b.duration = DBLCLICK_TIME;
      }
    }
  }
  _lastISR = time;
}
