#include "EtherHarvest.h"
#include "../../Debug/Logging.h"
#include "../../Devices/Storage/HardwareStorage.h"
#include <lwip/inet.h>
#include <cstring>

#define LOG_NEO "NEO"
#define LLMNR_PORT 5355

namespace Attacks {
    EtherHarvest Neo;
}

EtherHarvest::EtherHarvest() {}

void EtherHarvest::begin(Preferences& prefs) {
    // Register settings
}

void EtherHarvest::loop(Preferences& prefs) {
    // Task handles the work
}

void EtherHarvest::end() {
    stop();
}

void EtherHarvest::start() {
    if (running) return;
    
    Debug::Log.info(LOG_NEO, "Starting Ether-Harvest (Responder-Lite)...");
    
    // We assume NCM is active and IP is assigned (192.168.4.1 usually by default in NCM libs)
    // We start the listener task
    xTaskCreate(EtherHarvest::responderTask, "responderTask", 4096, this, 5, NULL);
    
    running = true;
    poisonedCount = 0;
}

void EtherHarvest::stop() {
    if (!running) return;
    
    Debug::Log.info(LOG_NEO, "Stopping Ether-Harvest...");
    running = false;
    // Task should self-terminate or we kill it effectively by closing socket
    if (sock_llmnr >= 0) {
        close(sock_llmnr);
        sock_llmnr = -1;
    }
}

void EtherHarvest::responderTask(void* pvParameters) {
    EtherHarvest* instance = (EtherHarvest*)pvParameters;
    instance->runResponder();
    vTaskDelete(NULL);
}

void EtherHarvest::runResponder() {
    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(LLMNR_PORT);
    
    sock_llmnr = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock_llmnr < 0) {
        Debug::Log.error(LOG_NEO, "Unable to create socket");
        running = false;
        return;
    }
    
    int err = bind(sock_llmnr, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    if (err < 0) {
        Debug::Log.error(LOG_NEO, "Socket unable to bind");
        close(sock_llmnr);
        running = false;
        return;
    }
    
    // Join LLMNR Multicast group
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr("224.0.0.252");
    mreq.imr_interface.s_addr = htonl(INADDR_ANY); // Default interface
    if (setsockopt(sock_llmnr, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        Debug::Log.error(LOG_NEO, "Failed to join LLMNR multicast group");
        // Continue anyway, might just work for unicast or braodcast
    }

    Debug::Log.info(LOG_NEO, "Listening for LLMNR queries...");
    
    char rx_buffer[512];
    struct sockaddr_in source_addr;
    socklen_t socklen = sizeof(source_addr);
    
    while (running) {
        int len = recvfrom(sock_llmnr, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr*)&source_addr, &socklen);
        
        if (len < 0) {
            Debug::Log.error(LOG_NEO, "recvfrom failed");
            break;
        }
        
        // Basic parsing of DNS packet
        // Header is 12 bytes
        // ID (2), Flags (2), QDCOUNT (2), ANCOUNT (2), NSCOUNT (2), ARCOUNT (2)
        if (len > 12) {
            uint16_t flags = (rx_buffer[2] << 8) | rx_buffer[3];
            uint16_t qdcount = (rx_buffer[4] << 8) | rx_buffer[5];
            
            // Interest only in Standard Query (Opcode 0) and Question (QR 0)
            if ((flags & 0xF800) == 0 && qdcount > 0) {
                // Parse Query Name
                // Format: [len]label[len]label...0
                int idx = 12;
                std::string name = "";
                while (idx < len) {
                    uint8_t lblLen = rx_buffer[idx++];
                    if (lblLen == 0) break;
                    if (idx + lblLen > len) break;
                    
                    if (name.length() > 0) name += ".";
                    for (int i=0; i<lblLen; i++) {
                        name += (char)rx_buffer[idx++];
                    }
                }
                
                char senderIp[16];
                inet_ntoa_r(source_addr.sin_addr, senderIp, sizeof(senderIp));
                
                std::string logMsg = "Poisoning LLMNR query for: " + name + " from " + std::string(senderIp);
                Debug::Log.info(LOG_NEO, logMsg);
                Devices::Storage.appendToFile("etherhost_log.csv", logMsg + "\n");
                
                poisonedCount++;
                
                // TODO: Send spoofed response
                // Construct DNS response
                // ID matches
                // Flags: QR=1, AA=1, RA=0, RCODE=0 -> 0x8400
                // Queue Question
                // Answer: Name pointer (C00C), Type A (0001), Class IN (0001), TTL (30s), Len 4, IP
                
                // For now, we just log detection (Passive Mode)
                // Active mode would involve sendto()
            }
        }
    }
    
    close(sock_llmnr);
    sock_llmnr = -1;
    running = false;
}
