#ifdef ARDUINO_ARCH_ESP32
#include "USBNCM.h"
#include "../../Debug/Logging.h"

#include <Adafruit_TinyUSB.h>
#include <usb_ncm.h>
#include <tcp_server.h>

#ifdef NO_SD
    #include <SPIFFS.h>
    #define FILE_INTERFACE SPIFFS
#elif USE_SD_INTERFACE
    // generic esp32 SD interface
    #include "SD.h"
    using namespace fs;
    #define FILE_INTERFACE SD
#else
    #include "../../Devices/Storage/ESP32/SDMMCFS2.h"
    using namespace fs;
    #define FILE_INTERFACE SD_MMC_2
#endif

#include "../../Utilities/PcapBuffer.h"

static Adafruit_USBD_NET usbNetworking;
static bool readyForPackets = false;
static PcapBuffer* buf = nullptr;

#define USBNCM_PCAPONSTART  "pcapNcmOnStart"
#define USBNCM_PCAPONSTART_DEFAULT  false

USBNCM::USBNCM()
{
}

extern "C" void createPcap(uint8_t *buffer, uint32_t len, bool received)
{
    Devices::USB::NCM.processIncomingPacket(buffer, len);

    if (buf != nullptr)
    {
        buf->append(buffer, len);
    }
}

void USBNCM::setPacketHandler(PacketHandlerCb handler)
{
    externalhandler = handler;
}

void USBNCM::processIncomingPacket(const uint8_t* buffer, uint32_t len)
{
    if (externalhandler)
    {
        externalhandler(buffer, len);
    }
}

bool USBNCM::injectPacket(const uint8_t* buffer, uint32_t len)
{
    if (readyForPackets)
    {
        // Try to send via TinyUSB networking
        // Note: writePacket signature usually (void* buf, uint32_t len)
        return usbNetworking.writePacket((void*)buffer, len);
    }
    return false;
}

void USBNCM::startPacketCollection()
{
    if (Devices::USB::Core.currentDeviceType() == USBDeviceType::NCM && buf == nullptr)
    {
        buf = new PcapBuffer(std::string("usbncm"), &FILE_INTERFACE);
        set_packet_handler(&createPcap);
        Debug::Log.info("NCM", "Started PCAP");
    }
    else
    {
        Debug::Log.error("NCM", "Error starting PCAP, incorrect mode or missing SD");
    }
}

void USBNCM::stopPacketCollection()
{
    // We don't null listner anymore to keep EAPHarvester working?
    // Actually, if we stop pcap, we shouldn't kill the handler if EAP is running.
    // However, the design assumes PacketHandler is global.
    // Let's ensure begin() sets it, and start/stop just toggle 'buf' existence?
    // No, startPacketCollection sets it.
    // Let's rely on EAPHarvester enabling it, OR change architecture.
    // For now: Leave as is, but EAPHarvester can call set_packet_handler(&createPcap) too if needed?
    // Better: Ensure begin() sets it.
    
    // set_packet_handler(NULL); // Don't disable complete handler
    Debug::Log.info("NCM", "Stopping PCAP");
    if (buf != nullptr)
    {
        delete buf;
        buf = nullptr;
    }
}

void USBNCM::begin(Preferences &prefs)
{
    registerUserConfigurableSetting(CATEGORY_USB, USBNCM_PCAPONSTART, USBArmyKnifeCapability::SettingType::Bool, USBNCM_PCAPONSTART_DEFAULT);

    if (Devices::USB::Core.currentDeviceType() == USBDeviceType::NCM)
    {
        if (readyForPackets == false && usb_ncm_init(&usbnet_hasNewPacket, &usbnet_getPacket, &usbnet_releasePacket, &usbnet_transmitPacket) == ESP_OK)
        {
            bool pcapOnStart = prefs.getBool(USBNCM_PCAPONSTART, USBNCM_PCAPONSTART_DEFAULT);
            if (pcapOnStart)
            {
                startPacketCollection();
            }

            //Starting this task seems to take resources away from NimBLE meaning its tasks can't start
            //xTaskCreate(tcp_server_task, "tcp_server", 4096, (void *)1234, 4, NULL);
            Debug::Log.info("NCM", "NCM Init OK");
            
            // Always set handler to allow EAP harvesting even if PCAP is off
            set_packet_handler(&createPcap);
            
            readyForPackets = true;
            TinyUSB_Device_Init(0, true);
        }
        else
        {
            Debug::Log.error("NCM", "NCM Init error");
        }
    }
}

void USBNCM::loop(Preferences &prefs)
{
    if (Devices::USB::Core.currentDeviceType() == USBDeviceType::NCM && readyForPackets)
    {
        poll_for_packets();
    }

    if (buf != nullptr)
    {
        // access to files 
        // Most applications need to use the protocol layer only in one task.
        // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/sdmmc.html
        buf->flushToDisk();
    }
}

void USBNCM::end()
{
    if (buf != nullptr)
    {
        stopPacketCollection();
    }
}
#endif