#pragma once

#include "../../USBArmyKnifeCapability.h"
#include <esp_wifi.h>
#include <esp_wifi_types.h>
#include <vector>
#include <string>

class SilentSentinel : public USBArmyKnifeCapability {
public:
    SilentSentinel();
    
    virtual void begin(Preferences& prefs) override;
    virtual void loop(Preferences& prefs) override;
    virtual void end() override;

    void start();
    void stop();
    bool isRunning() { return running; }
    
    // Stats
    uint32_t getCapturedCount() { return capturedCount; }

private:
    bool running = false;
    uint32_t capturedCount = 0;
    uint32_t lastChannelHop = 0;
    uint8_t currentChannel = 1;
    
    void cycleChannel();
    static void snifferCallback(void* buf, wifi_promiscuous_pkt_type_t type);
    void processPacket(void* buf, wifi_promiscuous_pkt_type_t type);
};

namespace Attacks {
    extern SilentSentinel Ghost;
}
