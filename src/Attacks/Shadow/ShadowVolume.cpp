#include "ShadowVolume.h"
#include "../../Debug/Logging.h"
#include "../../Devices/USB/USBCore.h"

#define LOG_SHADOW "SHADOW"
#define DEFAULT_KNOCK "1337"

namespace Attacks {
    ShadowVolume GhostStorage;
}

ShadowVolume::ShadowVolume() {}

void ShadowVolume::begin(Preferences& prefs) {
    knockSequence = DEFAULT_KNOCK; // In reality this would be loaded from prefs
}

void ShadowVolume::loop(Preferences& prefs) {
    // Check for timeout of sequence if needed
}

void ShadowVolume::end() {
    disableShadow();
}

void ShadowVolume::enableShadow(const std::string& secretKnock) {
    Debug::Log.info(LOG_SHADOW, "Attempting to unlock Shadow Volume...");
    
    // In a real implementation, this would mount an encrypted .img file as the Mass Storage Device
    // For this tech demo, we will mount a specific "shadow.img" file instead of the raw SD
    // This requires USBCore/USBMSC to support re-hosting
    
    // We simulate this by checking the knock
    if (secretKnock == knockSequence) {
        Debug::Log.info(LOG_SHADOW, "Knock Accepted. Mounting Shadow Volume.");
        
        // Unmount current (soft reset)
        // Mount "shadow.img"
        bool result = Devices::USB::MSC.mountDiskImage("/shadow.img", false);
        
        if (result) {
            shadowActive = true;
            Debug::Log.info(LOG_SHADOW, "Shadow Volume Mounted Successfully.");
        } else {
            Debug::Log.error(LOG_SHADOW, "Failed to mount shadow.img. Does it exist?");
        }
    } else {
        Debug::Log.warning(LOG_SHADOW, "Invalid Knock.");
    }
}

void ShadowVolume::disableShadow() {
    if (shadowActive) {
        Debug::Log.info(LOG_SHADOW, "Locking Shadow Volume.");
        // Remount default SD
        Devices::USB::MSC.mountSD();
        shadowActive = false;
    }
}

void ShadowVolume::registerKnock(const std::string& key) {
    // This could be fed by DuckyScript "READ_KEYS" or WebUI
    // For now, webUI checks
    enableShadow(key);
}
