#include "HydraHID.h"
#include "../../Debug/Logging.h"

#define LOG_TRINITY "TRINITY"

namespace Attacks {
    HydraHID Trinity;
}

HydraHID::HydraHID() {}

void HydraHID::begin(Preferences& prefs) {
    populateIdentities();
}

void HydraHID::loop(Preferences& prefs) {
    if (running) {
        // Auto-cycle every 10 seconds if running? 
        // Or just wait for user command?
        // Let's implement auto-cycle for "Fuzzing" mode
        if (millis() - lastCycleTime > 10000) {
            cycleIdentity();
        }
    }
}

void HydraHID::end() {
    stop();
}

void HydraHID::start() {
    running = true;
    Debug::Log.info(LOG_TRINITY, "Hydra-HID Activated: Cycling Identities...");
    cycleIdentity();
}

void HydraHID::stop() {
    running = false;
    Debug::Log.info(LOG_TRINITY, "Hydra-HID Deactivated");
}

void HydraHID::populateIdentities() {
    // Common trusted keyboards/mice
    identities.push_back({0x046D, 0xC31C, "Logitech", "USB Keyboard"}); // Logitech
    identities.push_back({0x413C, 0x2107, "Dell", "KB216 Wired Keyboard"}); // Dell
    identities.push_back({0x03F0, 0x034A, "HP", "Elite USB Keyboard"}); // HP
    identities.push_back({0x045E, 0x07F8, "Microsoft", "Wired Keyboard 600"}); // Microsoft
    identities.push_back({0x17EF, 0x608D, "Lenovo", "Calliope Keyboard"}); // Lenovo
    identities.push_back({0x05AC, 0x024F, "Apple", "Magic Keyboard"}); // Apple
}

void HydraHID::cycleIdentity() {
    lastCycleTime = millis();
    currentIndex = (currentIndex + 1) % identities.size();
    
    USBIdentity id = identities[currentIndex];
    Debug::Log.info(LOG_TRINITY, "Switching Identity to: " + id.manufacturer + " - " + id.product);
    
    // We need to form the correct USB Mode mask. 
    // HydraHID assumes we want to maintain the current capabilities but just swap VID/PID
    // However, USBCore::changeUSBMode takes a specific mode. 
    // Let's assume we want HID + Serial (Standard Ducky Mode)
    // Note: DuckyInterpreter::USB_MODE is an enum.
    
    DuckyInterpreter::USB_MODE mode = (DuckyInterpreter::USB_MODE)(DuckyInterpreter::USB_MODE::HID); 
    // We might want Storage too if it was enabled, but let's stick to HID for evasion.
    
    std::string serial = "1337"; // Could randomize this too
    
    Devices::USB::Core.changeUSBMode(mode, id.vid, id.pid, id.manufacturer, id.product, serial);
}

USBIdentity HydraHID::getCurrentIdentity() {
    if (identities.empty()) return {0,0,"",""};
    return identities[currentIndex];
}
