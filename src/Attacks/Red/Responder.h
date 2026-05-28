#pragma once

#include "../../USBArmyKnifeCapability.h"

#ifdef ARDUINO_ARCH_ESP32
#include <AsyncUDP.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#endif

namespace Attacks::Red {

class Responder : public USBArmyKnifeCapability {
public:
  Responder();

  virtual void begin(Preferences& prefs);
  virtual void loop(Preferences& prefs);
  virtual void end();

private:
  bool isEnabled = false;

#ifdef ARDUINO_ARCH_ESP32
  AsyncUDP udpLLMNR;
  WiFiServer* smbServer;
  
  void handleLLMNR(AsyncUDPPacket packet);
  void handleSMB();
#endif
};

extern Responder ResponderC2;

} // namespace Attacks::Red
