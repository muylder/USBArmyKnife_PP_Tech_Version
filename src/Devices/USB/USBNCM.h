#pragma once

#include "../../USBArmyKnifeCapability.h"
#include "USBCore.h"

class USBNCM : USBArmyKnifeCapability {
public:
  USBNCM();

  virtual void loop(Preferences& prefs);
  virtual void begin(Preferences& prefs);
  virtual void end();

  void startPacketCollection();
  void stopPacketCollection();
  
  // Method to allow other modules to hook into RX stream
  typedef void (*PacketHandlerCb)(const uint8_t* buffer, uint32_t len);
  void setPacketHandler(PacketHandlerCb handler);
  
  // Method to send raw Ethernet frames to Host
  bool injectPacket(const uint8_t* buffer, uint32_t len);

  // Internal Rx dispatch
  void processIncomingPacket(const uint8_t* buffer, uint32_t len);

private:
  PacketHandlerCb externalhandler = nullptr;
};

namespace Devices::USB
{
    extern USBNCM NCM;
}