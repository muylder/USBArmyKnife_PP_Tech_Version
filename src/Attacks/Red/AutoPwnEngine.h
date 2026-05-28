#pragma once

#include "../../USBArmyKnifeCapability.h"
#include <string>
#include <vector>

namespace Attacks::Red {

class AutoPwnEngine : public USBArmyKnifeCapability {
public:
  AutoPwnEngine();

  virtual void begin(Preferences& prefs);
  virtual void loop(Preferences& prefs);
  virtual void end();

  void executeScript(const std::string& filepath);

private:
  bool isEnabled = false;
  bool isRunning = false;
  std::vector<std::string> scriptLines;
  int currentLine = 0;

  void parseAndExecuteLine(const std::string& line);
};

extern AutoPwnEngine PwnEngine;

} // namespace Attacks::Red
