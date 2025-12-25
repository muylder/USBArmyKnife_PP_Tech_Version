#pragma once

#include "../../USBArmyKnifeCapability.h"
#include "../../Devices/USB/USBCore.h"
#include <vector>
#include <string>

struct USBIdentity {
    uint16_t vid;
    uint16_t pid;
    std::string manufacturer;
    std::string product;
};

class HydraHID : public USBArmyKnifeCapability {
public:
    HydraHID();
    
    virtual void begin(Preferences& prefs) override;
    virtual void loop(Preferences& prefs) override;
    virtual void end() override;

    void start();
    void stop();
    void cycleIdentity();
    bool isRunning() { return running; }
    
    USBIdentity getCurrentIdentity();

private:
    bool running = false;
    std::vector<USBIdentity> identities;
    size_t currentIndex = 0;
    unsigned long lastCycleTime = 0;
    
    void populateIdentities();
};

namespace Attacks {
    extern HydraHID Trinity;
}
