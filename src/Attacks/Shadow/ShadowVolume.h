#pragma once

#include "../../USBArmyKnifeCapability.h"
#include "../../Devices/USB/USBMSC.h"
#include <string>

class ShadowVolume : public USBArmyKnifeCapability {
public:
    ShadowVolume();
    
    virtual void begin(Preferences& prefs) override;
    virtual void loop(Preferences& prefs) override;
    virtual void end() override;

    void enableShadow(const std::string& secretKnock);
    void disableShadow();
    bool isShadowActive() { return shadowActive; }
    
    // Knock handling 
    void registerKnock(const std::string& key);

private:
    bool shadowActive = false;
    std::string knockSequence;
    std::string currentSequence;
};

namespace Attacks {
    extern ShadowVolume GhostStorage;
}
