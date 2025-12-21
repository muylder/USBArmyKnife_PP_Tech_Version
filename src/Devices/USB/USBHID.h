#pragma once

#include "USBCore.h"
#include "../../USBArmyKnifeCapability.h"

class USBHID : USBArmyKnifeCapability {
public:
  USBHID();

  virtual void loop(Preferences& prefs);
  virtual void begin(Preferences& prefs);

  bool IsQueueEmpty();
  void keyboard_press(const uint8_t modifiers, const uint8_t key1, const uint8_t key2, const uint8_t key3, const uint8_t key4, const uint8_t key5, const uint8_t key6);
  void keyboard_release();
  void consumer_device_keypress(const uint16_t keyCode);
  void mouseMove(int8_t xDelta, int8_t yDelta);

  uint8_t getLEDs(); // Returns bitmask: 0x01 NumLock, 0x02 CapsLock, 0x04 ScrollLock
};

namespace Devices::USB
{
    extern USBHID HID;
}