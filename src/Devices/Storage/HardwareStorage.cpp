#include "HardwareStorage.h"
#include "../../Debug/Logging.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#ifdef NO_SD
    #ifdef ARDUINO_ARCH_RP2040
        #include <VFS.h>
        #include <LittleFS.h>
        #define FILE_INTERFACE LittleFS
    #else
        #include <SPIFFS.h>
        #define FILE_INTERFACE SPIFFS
    #endif
#elif ARDUINO_ARCH_RP2040
    #include "RP2040/SDClassWrapper.h"
    using namespace fs;
    #define FILE_INTERFACE SDCard
#elif USE_SD_INTERFACE
    // generic esp32 SD interface
    #include "SD.h"
    using namespace fs;
    #define FILE_INTERFACE SD
#else
    #include "ESP32/SDMMCFS2.h"
    #include "driver/sdmmc_host.h"
    #include "driver/sdspi_host.h"
    #include "sdmmc_cmd.h"
    using namespace fs;
    #define FILE_INTERFACE SD_MMC_2

    fs::SDMMCFS getFileInterface()
    {
        return FILE_INTERFACE;
    }

#endif

#define LOG_MMC "MMC"

static uint8_t cachedCapacity = 0;
static std::vector<std::string> filesCache;
static SemaphoreHandle_t fsMutex = NULL;

namespace Devices
{
    HardwareStorage Storage;
}

std::string HardwareStorage::readFile(fs::FS &fs, const char *path)
{
    if (fsMutex != NULL) xSemaphoreTake(fsMutex, portMAX_DELAY);
    File file = fs.open(path, "r");
    if (!file)
    {
        if (fsMutex != NULL) xSemaphoreGive(fsMutex);
        return "";
    }
    std::string line;
    while (file.available())
    {
        line = line + (char)file.read();
    }
    file.close(); // Ensure close before give
    if (fsMutex != NULL) xSemaphoreGive(fsMutex);
    return line;
}

void HardwareStorage::writeFileData(const std::string& filename, const uint8_t *buffer, const size_t size)
{
    if (fsMutex != NULL) xSemaphoreTake(fsMutex, portMAX_DELAY);
    File file = FILE_INTERFACE.open(filename.c_str(), "w");
    if (!file)
    {
        Debug::Log.info(LOG_MMC, "Could not open file");
        if (fsMutex != NULL) xSemaphoreGive(fsMutex);
        return;
    }
    
    if (file.write(buffer, size) != size)
    {
        Debug::Log.info(LOG_MMC, "write err");
    }

    file.close();
    refreshCache();
    if (fsMutex != NULL) xSemaphoreGive(fsMutex);
}

void HardwareStorage::appendToFile(const std::string& filename, const std::string& content)
{
    if (fsMutex != NULL) xSemaphoreTake(fsMutex, portMAX_DELAY);
    File file = FILE_INTERFACE.open(filename.c_str(), "a");
    if (!file)
    {
        Debug::Log.info(LOG_MMC, "Could not open file for append: " + filename);
        if (fsMutex != NULL) xSemaphoreGive(fsMutex);
        return;
    }
    file.print(content.c_str());
    file.close();
    refreshCache();
    if (fsMutex != NULL) xSemaphoreGive(fsMutex);
}

// WARNING: Returning a File object is inherently unsafe if used across threads
// without holding the lock for the duration of the File object's lifetime.
// However, since we can't easily lock for the lifetime of the object returned,
// callers must be careful or we need a wrapper. 
// For now, we protect the open call.
File HardwareStorage::openFile(const std::string& filename, const char* mode)
{
    if (fsMutex != NULL) xSemaphoreTake(fsMutex, portMAX_DELAY);
    File file = FILE_INTERFACE.open(filename.c_str(), mode);
    if (!file)
    {
        Debug::Log.error(LOG_MMC, "Could not open file");
        if (fsMutex != NULL) xSemaphoreGive(fsMutex);
        return File();
    }

    if (fsMutex != NULL) xSemaphoreGive(fsMutex);
    return file;
}

std::size_t HardwareStorage::getFileSize(const std::string& filename)
{
    if (fsMutex != NULL) xSemaphoreTake(fsMutex, portMAX_DELAY);
    File file = FILE_INTERFACE.open(filename.c_str(), "r");
    if (!file)
    {
        Debug::Log.info(LOG_MMC, "Could not open file: " + filename);
        if (fsMutex != NULL) xSemaphoreGive(fsMutex);
        return 0;
    }

    // Get the file size
    int ret = file.size();
    file.close();
    if (fsMutex != NULL) xSemaphoreGive(fsMutex);
    return ret;
}

bool HardwareStorage::doesFileExist(const std::string& filename)
{
    bool ret = false;
    if (fsMutex != NULL) xSemaphoreTake(fsMutex, portMAX_DELAY);
    ret = FILE_INTERFACE.exists(filename.c_str());
    if (fsMutex != NULL) xSemaphoreGive(fsMutex);
    return ret;
}

uint8_t* HardwareStorage::readFileAsBinary(const std::string& filename)
{
    if (fsMutex != NULL) xSemaphoreTake(fsMutex, portMAX_DELAY);
    File file = FILE_INTERFACE.open(filename.c_str(), "r");
    if (!file)
    {
        Debug::Log.info(LOG_MMC, "Could not open file: " + filename);
        if (fsMutex != NULL) xSemaphoreGive(fsMutex);
        return 0;
    }

    // Get the file size
    size_t fileSize = file.size();

    // Allocate memory for the file content
    uint8_t *fileContent = (uint8_t *)malloc(fileSize + 1); // +1 for null terminator
    if (!fileContent)
    {
        Debug::Log.info(LOG_MMC, "Could not alloc");
        file.close();
        if (fsMutex != NULL) xSemaphoreGive(fsMutex);
        return 0;
    }

    // Read the entire file into the buffer
    if (file.read((uint8_t *)fileContent, fileSize) != fileSize)
    {
        Debug::Log.info(LOG_MMC, "Could not read");
        free(fileContent);
        file.close();
        if (fsMutex != NULL) xSemaphoreGive(fsMutex);
        return 0;
    }

    // Null-terminate the buffer
    fileContent[fileSize] = '\0';
    file.close();

    if (fsMutex != NULL) xSemaphoreGive(fsMutex);
    return fileContent;
}

std::string HardwareStorage::readLineFromFile(const std::string &filename, const int lineNumber)
{
    if (fsMutex != NULL) xSemaphoreTake(fsMutex, portMAX_DELAY);
    File file = FILE_INTERFACE.open(filename.c_str(), "r");
    if (!file)
    {
        Debug::Log.info(LOG_MMC, "Could not open file: " + filename);
        if (fsMutex != NULL) xSemaphoreGive(fsMutex);
        return "";
    }

    int currentLine = 0;
    String line;
    while (file.available())
    {
        line = file.readStringUntil('\n'); // todo read up to EOF
        if (currentLine == lineNumber)
        {
            break;
        }
        line = "";
        currentLine++;
    }
    file.close();

    std::string ret = std::string(line.c_str());
    if (fsMutex != NULL) xSemaphoreGive(fsMutex);

    if (ret.length() == 1 && ret[0] == '\n')
    {
        return "";
    }
    if (ret.length() == 2 && ret[0] == '\r' && ret[1] == '\n')
    {
        return "";
    }

    return ret;
}

static void listDir(std::vector<std::string> &files, fs::FS &fs, const char *dirname)
{
    File root = fs.open(dirname, "r");
    if (!root)
    {
        return;
    }
    if (!root.isDirectory())
    {
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            const char* path =
#ifdef ARDUINO_ARCH_RP2040
            file.fullName();
#else
            file.path();
#endif
            listDir(files, fs, path);
        }
        else
        {
            std::string filename = std::string(file.name());
            files.emplace_back("/" + filename);
        }
        file = root.openNextFile();
    }
}

std::vector<std::string> HardwareStorage::listFiles()
{
    if (fsMutex != NULL) xSemaphoreTake(fsMutex, portMAX_DELAY);
    if (filesCache.size() != 0)
    {
        if (fsMutex != NULL) xSemaphoreGive(fsMutex);
        return filesCache;
    }

    if (running)
    {
        listDir(filesCache, FILE_INTERFACE, "/");
    }

    if (fsMutex != NULL) xSemaphoreGive(fsMutex);
    return filesCache; // Returns copy
}

uint8_t HardwareStorage::usedPercentage()
{
    if (fsMutex != NULL) xSemaphoreTake(fsMutex, portMAX_DELAY);
    if (cachedCapacity != 0)
    {
        if (fsMutex != NULL) xSemaphoreGive(fsMutex);
        return cachedCapacity;
    }
#if defined(NO_SD) && defined(ARDUINO_ARCH_RP2040)
    fs::FSInfo info;
    if (FILE_INTERFACE.info(info))
    {
        cachedCapacity = (info.usedBytes / info.totalBytes)* 100;
    }
    else
    {
        cachedCapacity = 0;
    }
#else
    cachedCapacity = (FILE_INTERFACE.usedBytes() / FILE_INTERFACE.totalBytes()) * 100;
#endif
    if (fsMutex != NULL) xSemaphoreGive(fsMutex);
    return cachedCapacity;
}

bool HardwareStorage::createEmptyFile(const std::string &filename)
{
    if (fsMutex != NULL) xSemaphoreTake(fsMutex, portMAX_DELAY);
    File file = FILE_INTERFACE.open(filename.c_str(), "w");
    if (!file)
    {
        if (fsMutex != NULL) xSemaphoreGive(fsMutex);
        return false;
    }
    file.close();
    refreshCache();
    if (fsMutex != NULL) xSemaphoreGive(fsMutex);
    return true;
}

bool HardwareStorage::deleteFile(const std::string& filename)
{
    if (fsMutex != NULL) xSemaphoreTake(fsMutex, portMAX_DELAY);
    refreshCache();
    bool res = FILE_INTERFACE.remove(filename.c_str());
    if (fsMutex != NULL) xSemaphoreGive(fsMutex);
    return res;
}

HardwareStorage::HardwareStorage()
{
#ifndef NO_SD
#if !defined(ARDUINO_ARCH_RP2040) && !defined(USE_SD_INTERFACE)
    SD_MMC = FILE_INTERFACE;
#endif
#endif
    fsMutex = xSemaphoreCreateMutex();
}

bool HardwareStorage::isRawAccessSupported()
{
#if defined(USE_SD_INTERFACE) or defined(USE_SD_MMC_INTERFACE)
    return true;
#else
    return false;
#endif
}

size_t HardwareStorage::sectorSize()
{
#if defined(USE_SD_INTERFACE)
    return FILE_INTERFACE.sectorSize();
#elif defined (USE_SD_MMC_INTERFACE)
    return FILE_INTERFACE.getCard()->csd.sector_size;
#else
    return 0;
#endif
}

size_t HardwareStorage::deviceCapacity()
{
#if defined(USE_SD_INTERFACE)
    return FILE_INTERFACE.cardSize();
#elif defined (USE_SD_MMC_INTERFACE)
    return FILE_INTERFACE.getCard()->csd.capacity;
#else
    return 0;
#endif
}

int32_t HardwareStorage::readRawSectors(uint8_t* buffer, uint32_t lba, uint32_t sectors)
{
#if defined(USE_SD_INTERFACE)
    return FILE_INTERFACE.readRAW((uint8_t*) buffer, lba);
#elif defined (USE_SD_MMC_INTERFACE)
    if (sdmmc_read_sectors(FILE_INTERFACE.getCard(), buffer, lba, sectors) == ESP_OK)
    {
        return sectors * FILE_INTERFACE.getCard()->csd.sector_size;
    }
    return -1;
#else
    return -1;
#endif
}

int32_t HardwareStorage::writeRawSectors(uint8_t* buffer, uint32_t lba, uint32_t sectors)
{
#if defined(USE_SD_INTERFACE)
    return FILE_INTERFACE.writeRAW(buffer, lba);
#elif defined (USE_SD_MMC_INTERFACE)
    if (sdmmc_write_sectors(FILE_INTERFACE.getCard(), buffer, lba, sectors) == ESP_OK)
    {
        return sectors * FILE_INTERFACE.getCard()->csd.sector_size;
    }
    return -1;
#else
    return -1;
#endif
}

void HardwareStorage::flush()
{
    // not currently supported
}

void HardwareStorage::refreshCache()
{
    filesCache.clear();
}

void HardwareStorage::loop(Preferences &prefs)
{
}

void HardwareStorage::begin(Preferences &prefs)
{
    this->begin(prefs, false);
}

void HardwareStorage::begin(Preferences &prefs, bool format)
{
#ifdef NO_SD
    #ifdef ARDUINO_ARCH_RP2040
    if (!FILE_INTERFACE.begin())
    {
        Debug::Log.info(LOG_MMC, "FILE_INTERFACE could not be started");
    }
    else
    {
        running = true;
        VFS.root(FILE_INTERFACE);
    }
    
    #else
    if (!FILE_INTERFACE.begin(format, "/sdcard"))
    {
        Debug::Log.info(LOG_MMC, "FILE_INTERFACE could not be started");
    }
    else
    {
        running = true;
    }
    #endif
#elif defined (SD_MMC_D1_PIN)
    FILE_INTERFACE.setPins(SD_MMC_CLK_PIN, SD_MMC_CMD_PIN, SD_MMC_D0_PIN, SD_MMC_D1_PIN, SD_MMC_D2_PIN, SD_MMC_D3_PIN);
    if (!FILE_INTERFACE.begin("/sdcard", false, format, SDMMC_FREQ_52M))
    {
        Debug::Log.info(LOG_MMC, "FILE_INTERFACE could not be started");
    }
    else
    {
        uint8_t cardType = FILE_INTERFACE.cardType();
        if (cardType == CARD_NONE)
        {
            Debug::Log.info(LOG_MMC, "Could not find SD Card");
        }
        else
        {
            running = true;
        }
    }
#elif defined(ARDUINO_ARCH_RP2040)
    if (!FILE_INTERFACE.begin(format))
    {
        Debug::Log.info(LOG_MMC, "FILE_INTERFACE could not be started");
    }
    else
    {
        running = true;
    }
#elif defined(USE_SD_INTERFACE)
    pinMode(SD_MISO_PIN, INPUT_PULLUP);
    SPI.begin(SD_SCLK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);

    if (!FILE_INTERFACE.begin(SD_CS_PIN))
    {
        Debug::Log.info(LOG_MMC, "FILE_INTERFACE could not be started");
    }
    else
    {
        running = true;
    }
#endif
}