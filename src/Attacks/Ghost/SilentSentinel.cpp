#include "SilentSentinel.h"
#include "../../Debug/Logging.h"
#include "../../Devices/Storage/HardwareStorage.h"
#include <cstring>

#define LOG_GHOST "GHOST"

namespace Attacks {
    SilentSentinel Ghost;
}

// Struct definitions for parsing (simplified from standard 802.11)
typedef struct {
    int16_t fctl;
    int16_t duration;
    uint8_t da[6];
    uint8_t sa[6];
    uint8_t bssid[6];
    int16_t seqctl;
    unsigned char payload[];
} __attribute__((packed)) WifiMgmtHdr;

typedef struct {
    wifi_pkt_rx_ctrl_t rx_ctrl;
    uint8_t payload[];
} __attribute__((packed)) wifi_promiscuous_pkt_t;

SilentSentinel::SilentSentinel() {}

void SilentSentinel::begin(Preferences& prefs) {
    // Register settings if needed
}

void SilentSentinel::loop(Preferences& prefs) {
    if (running) {
        cycleChannel();
    }
}

void SilentSentinel::end() {
    stop();
}

void SilentSentinel::start() {
    if (running) return;
    
    Debug::Log.info(LOG_GHOST, "Starting Silent Sentinel...");
    
    // Ensure WiFi is initialized (assuming basic init was done by main or we do it)
    // We assume standard station mode init happened, or we force it.
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    esp_wifi_set_mode(WIFI_MODE_NULL);
    esp_wifi_start();
    
    // Set promiscuous
    wifi_promiscuous_filter_t filt = {.filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT};
    esp_wifi_set_promiscuous_filter(&filt);
    esp_wifi_set_promiscuous_rx_cb(&SilentSentinel::snifferCallback);
    esp_wifi_set_promiscuous(true);
    
    // Set channel
    esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
    
    running = true;
    capturedCount = 0;
    
    // Open log file to verify write access
    // We assume append in the callback or batched writing
    // Ideally we batch, but for simplicity we might just format string and call Storage logging
}

void SilentSentinel::stop() {
    if (!running) return;
    
    Debug::Log.info(LOG_GHOST, "Stopping Silent Sentinel...");
    esp_wifi_set_promiscuous(false);
    esp_wifi_set_promiscuous_rx_cb(nullptr);
    running = false;
}

void SilentSentinel::cycleChannel() {
    if (millis() - lastChannelHop > 500) { // Hop every 500ms
        lastChannelHop = millis();
        currentChannel++;
        if (currentChannel > 13) currentChannel = 1;
        esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
    }
}

void SilentSentinel::snifferCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
    Attacks::Ghost.processPacket(buf, type);
}

void SilentSentinel::processPacket(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (type != WIFI_PKT_MGMT) return;
    
    wifi_promiscuous_pkt_t *packet = (wifi_promiscuous_pkt_t*)buf;
    WifiMgmtHdr *hdr = (WifiMgmtHdr*)packet->payload;
    
    // Frame Control: Little Endian. 
    // Type is bits 2-3, Subtype is bits 4-7.
    // Management frames are Type 00 (0). 
    // Probe Request is Subtype 0100 (4).
    // fctl is 16 bits. 
    // Byte 0: Protocol(2) | Type(2) | Subtype(4)
    // Probe Req: Subtype 4 (0100), Type 0 (00).
    // So lower byte should be xxxx00xx -> wait.
    // Subtype is bits 4-7. 4 is 0100.
    // 0100 00 00 = 0x40.
    
    // Let's inspect the raw bytes of FCTL
    uint8_t frameType = packet->payload[0];
    
    if (frameType == 0x40) { // Probe Request
        int len = packet->rx_ctrl.sig_len;
        // Payload starts after header (24 bytes)
        int offset = sizeof(WifiMgmtHdr);
        
        // Safety check
        if (len < offset) return;
        
        uint8_t* data = packet->payload + offset;
        int remLen = len - offset;
        
        // Parse IEs
        // Tag 0 is SSID
        if (remLen > 2 && data[0] == 0) { // Tag 0
            uint8_t ssidLen = data[1];
            if (remLen >= 2 + ssidLen) {
                if (ssidLen > 0 && ssidLen <= 32) {
                    char ssid[33];
                    memcpy(ssid, data + 2, ssidLen);
                    ssid[ssidLen] = 0;
                    
                    // Filter empty SSIDs (wildcard probes)
                    if (ssidLen > 0) {
                        char macStr[18];
                        snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
                            hdr->sa[0], hdr->sa[1], hdr->sa[2], 
                            hdr->sa[3], hdr->sa[4], hdr->sa[5]);
                            
                        // Log it
                        std::string logEntry = std::string(macStr) + "," + std::string(ssid) + "," + std::to_string(packet->rx_ctrl.rssi);
                        
                        // We filter wildcards (empty SSID often sent by phones to check presence)
                        // Actually, directed probes (with SSID) are the valuable ones for Ghost.
                        
                        // Use Debug log for now, or append to file
                        // Ideally we encrypt this. EvilPortal encrypter is not easily accessible here without including Marauder headers.
                        // We will dump plaintext to a separate log "sentinel.csv" for now.
                        Devices::Storage.appendToFile("sentinel.csv", logEntry + "\n");
                        capturedCount++;
                    }
                }
            }
        }
    }
}
