#pragma once

#include "../../USBArmyKnifeCapability.h"
#include <lwip/sockets.h>
#include <lwip/netdb.h>

class EtherHarvest : public USBArmyKnifeCapability {
public:
    EtherHarvest();
    
    virtual void begin(Preferences& prefs) override;
    virtual void loop(Preferences& prefs) override;
    virtual void end() override;

    void start();
    void stop();
    bool isRunning() { return running; }
    
    // Stats
    uint32_t getPoisonedCount() { return poisonedCount; }

private:
    bool running = false;
    uint32_t poisonedCount = 0;
    int sock_llmnr = -1;
    
    static void responderTask(void* pvParameters);
    void runResponder();
};

namespace Attacks {
    extern EtherHarvest Neo;
}
