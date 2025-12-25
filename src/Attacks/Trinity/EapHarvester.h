#pragma once

#include "../../USBArmyKnifeCapability.h"
#include <vector>
#include <string>

// EAPOL Constants
#define EAPOL_ETHERTYPE 0x888E
#define EAP_CODE_REQUEST 1
#define EAP_CODE_RESPONSE 2
#define EAP_TYPE_IDENTITY 1
#define EAP_TYPE_MD5_CHALLENGE 4

struct EapPromise {
    std::string username;
    std::string challenge; // The random challenge we sent
    std::string response; // The MD5 response
    std::string macAddress;
};

class EapHarvester : public USBArmyKnifeCapability {
public:
    EapHarvester();
    
    virtual void begin(Preferences& prefs) override;
    virtual void loop(Preferences& prefs) override;
    virtual void end() override;

    void start();
    void stop();
    bool isRunning() { return running; }
    
    // Packet handling
    void processPacket(const uint8_t* buffer, uint32_t len);
    
    // Stats
    size_t getCapturedCount() { return capturedCreds.size(); }
    std::vector<EapPromise> getCapturedCreds() { return capturedCreds; }

private:
    bool running = false;
    std::vector<EapPromise> capturedCreds;
    
    // State machine per MAC? For simplicity, we assume single host (The PC we are plugged into)
    // We just blast requests when we see START
    
    void sendIdentityRequest(const uint8_t* destMac);
    void sendMd5Challenge(const uint8_t* destMac, uint8_t id);
    void logCredentials(const std::string& user, const std::string& challenge, const std::string& response, const std::string& mac);
};

namespace Attacks {
    extern EapHarvester Eap;
}
