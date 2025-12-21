#include "Logging.h"
#include "../Devices/USB/USBCDC.h"
#include "../Attacks/Agent/Agent.h"

#include <deque>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Increase log depth to 100
#define MAX_LOG_DEPTH 100

static std::deque<std::string> logMsg;
static SemaphoreHandle_t logMutex = NULL;

#define LOG_DELAY_UNTIL_DEBUGGER                    "loggerdelay"
#define LOG_DELAY_UNTIL_DEBUGGER_DEFAULT            true

static bool loggerDelayUntilDebuggerConnected = LOG_DELAY_UNTIL_DEBUGGER_DEFAULT;
static bool writtenPendingEntries = false;

namespace Debug
{
    Logging Log;
}

Logging::Logging()
{
}

std::vector<std::string> Logging::getLogs()
{
    std::vector<std::string> copy;
    if (logMutex != NULL)
    {
        if (xSemaphoreTake(logMutex, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            copy.reserve(logMsg.size());
            for(const auto& s : logMsg) {
                copy.push_back(s);
            }
            xSemaphoreGive(logMutex);
        }
    }
    return copy;
}

void Logging::clear()
{
    if (logMutex != NULL)
    {
        if (xSemaphoreTake(logMutex, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            logMsg.clear();
            xSemaphoreGive(logMutex);
        }
    }
}

static void writeDebugOut(const std::string& component, const std::string& msg)
{
    if (logMutex != NULL)
    {
        // Try to take the mutex, wait max 10ms. If we can't get it, we drop the log to avoid deadlocks in ISRs
        if (xSemaphoreTake(logMutex, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            logMsg.emplace_back(msg);
            
            // Limit size
            while (logMsg.size() > MAX_LOG_DEPTH)
            {
                logMsg.pop_front();
            }
            
            xSemaphoreGive(logMutex);
        }
    }
    else
    {
        // Fallback before mutex init (rare)
        logMsg.emplace_back(msg);
         while (logMsg.size() > MAX_LOG_DEPTH)
        {
            logMsg.pop_front();
        }
    }

    if (loggerDelayUntilDebuggerConnected == false || (loggerDelayUntilDebuggerConnected && Attacks::Agent.isAgentConnected()))
    {
        Devices::USB::CDC.writeDebugString(std::string(component) + "|" + msg);
    } 
}

void Logging::info(const char *component, const std::string &msg)
{
    std::string formattedMsg = "[INFO] " + msg;
    
    // Ensure we can't overflow the debug log with large amounts of data
    if (formattedMsg.length() > MAX_LOG_ENTRY_IN_CHARS)
    {
        const std::string err = "[INFO] ERROR LOG MESSAGE TOO LARGE!";
        writeDebugOut(component, err);
    }
    else
    {
        writeDebugOut(component, formattedMsg);
    }
}

void Logging::warning(const char *component, const std::string &msg)
{
    std::string formattedMsg = "[WARN] " + msg;
    if (formattedMsg.length() > MAX_LOG_ENTRY_IN_CHARS) formattedMsg = formattedMsg.substr(0, MAX_LOG_ENTRY_IN_CHARS);
    writeDebugOut(component, formattedMsg);
}

void Logging::error(const char *component, const std::string &msg)
{
    std::string formattedMsg = "[ERR]  " + msg;
    if (formattedMsg.length() > MAX_LOG_ENTRY_IN_CHARS) formattedMsg = formattedMsg.substr(0, MAX_LOG_ENTRY_IN_CHARS);
    writeDebugOut(component, formattedMsg);
}

extern "C" void log_compat(const char* msg)
{
    std::string logMsg(msg);
    Debug::Log.info("C", logMsg);
}

void Logging::loop(Preferences &prefs)
{
    // No need to prune here, we do it on write now.

    if (loggerDelayUntilDebuggerConnected && !writtenPendingEntries && Attacks::Agent.isAgentConnected())
    {
        // Snapshot logs to avoid holding mutex while writing to slow USB
        std::vector<std::string> pending;
        
        if (logMutex != NULL && xSemaphoreTake(logMutex, pdMS_TO_TICKS(100)) == pdTRUE)
        {
             for (const auto& msg : logMsg)
             {
                 pending.push_back(msg);
             }
             xSemaphoreGive(logMutex);
        }

        for (const auto& msg : pending)
        {
            Devices::USB::CDC.writeDebugString("OLD|" + msg);
        }
        writtenPendingEntries = true;
    }
}

void Logging::begin(Preferences &prefs)
{
    if (logMutex == NULL)
    {
        logMutex = xSemaphoreCreateMutex();
    }

    registerUserConfigurableSetting(CATEGORY_USB, LOG_DELAY_UNTIL_DEBUGGER, USBArmyKnifeCapability::SettingType::Bool, LOG_DELAY_UNTIL_DEBUGGER_DEFAULT);
    loggerDelayUntilDebuggerConnected = prefs.getBool(LOG_DELAY_UNTIL_DEBUGGER, LOG_DELAY_UNTIL_DEBUGGER_DEFAULT);
}
