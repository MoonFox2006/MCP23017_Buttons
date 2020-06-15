#ifndef __BUTTONS_H
#define __BUTTONS_H

#include "List.h"
#include "Queue.h"
#include "MCP23017.h"

struct button_t {
  uint8_t pin : 4;
  volatile bool pressed : 1;
  volatile bool dbl : 1;
  volatile uint16_t duration : 10;
};

class Buttons : public List<button_t, 16> {
public:
  enum eventkind_t : uint8_t { BTN_RELEASED, BTN_PRESSED, BTN_CLICK, BTN_DBLCLICK, BTN_LONGCLICK };

  struct event_t {
    eventkind_t kind : 3;
    uint8_t data : 4;
  };

  Buttons(MCP23017 &mcp, uint8_t intPin) : List<button_t, 16>(), _events(), _mcp(&mcp) {
    _mcp->attachInterrupt(intPin, std::bind(&Buttons::mcpCallback, this, std::placeholders::_1, std::placeholders::_2));
  }

  uint8_t addButton(uint8_t pin, bool level);

  uint8_t getEventCount() const {
    return _events.depth();
  }
  const event_t *peekEvent() {
    return _events.peek();
  }
  const event_t *getEvent() {
    return _events.get();
  }

protected:
  static const uint32_t CLICK_TIME = 20; // 20 ms.
  static const uint32_t DBLCLICK_TIME = 500; // 0.5 sec.
  static const uint32_t LONGCLICK_TIME = 1000; // 1 sec.

  bool putEvent(eventkind_t kind, uint8_t data);
  bool match(uint8_t index, const void *t) override;

  void mcpCallback(uint16_t pins, uint16_t values);

  Queue<event_t, 32> _events;
  MCP23017 *_mcp;
};

#endif
