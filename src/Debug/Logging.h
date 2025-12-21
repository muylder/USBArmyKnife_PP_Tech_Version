#pragma once

#ifdef __cplusplus
#include "../USBArmyKnifeCapability.h"

#include <string>
#include <vector>

#define MAX_LOG_ENTRY_IN_CHARS 350

class Logging : USBArmyKnifeCapability {
public:
  Logging();

  virtual void loop(Preferences& prefs);
  virtual void begin(Preferences& prefs);

  void info(const char* component, const std::string& msg);
  void warning(const char* component, const std::string& msg);
  void error(const char* component, const std::string& msg);
  
  // Returns a snapshot of current logs for thread safety
  std::vector<std::string> getLogs();
  void clear();
};

namespace Debug
{
    extern Logging Log;
}

extern "C"
{
#endif

    void log_compat(const char*);

#ifdef __cplusplus
 }
#endif
