#include "AutoPwnEngine.h"
#include "../../Debug/Logging.h"
#include "../../Devices/Storage/HardwareStorage.h"
#include "../Ducky/DuckyPayload.h"
#include "Responder.h"

#define TAG_PWN "AutoPwn"

namespace Attacks::Red {
  AutoPwnEngine PwnEngine;
}

using namespace Attacks::Red;

AutoPwnEngine::AutoPwnEngine() {}

void AutoPwnEngine::begin(Preferences& prefs) {
  isEnabled = true;
  isRunning = false;
  
  // Auto-start default pwn script if it exists
  if (Devices::Storage.isRunning()) {
      auto file = Devices::Storage.openFile("/autopwn.txt", "r");
      if (file) {
          file.close();
          executeScript("/autopwn.txt");
      }
  }
}

void AutoPwnEngine::executeScript(const std::string& filepath) {
  if (!Devices::Storage.isRunning()) return;

  auto file = Devices::Storage.openFile(filepath.c_str(), "r");
  if (!file) return;

  scriptLines.clear();
  while (file.available()) {
      String line = file.readStringUntil('\n');
      line.trim();
      if (line.length() > 0) {
          scriptLines.push_back(line.c_str());
      }
  }
  file.close();

  currentLine = 0;
  isRunning = true;
  Debug::Log.info(TAG_PWN, "AutoPwn Engine Started.");
}

void AutoPwnEngine::loop(Preferences& prefs) {
  if (!isEnabled || !isRunning) return;

  if (currentLine < scriptLines.size()) {
      parseAndExecuteLine(scriptLines[currentLine]);
      currentLine++;
      delay(10); // Small delay between script commands
  } else {
      isRunning = false;
      Debug::Log.info(TAG_PWN, "AutoPwn Engine Finished.");
  }
}

void AutoPwnEngine::parseAndExecuteLine(const std::string& line) {
  Debug::Log.info(TAG_PWN, "Executing: " + line);

  if (line.rfind("INJECT ", 0) == 0) {
      std::string payload = line.substr(7);
      Attacks::Ducky.setPayloadCmdLine(payload.c_str());
  } 
  else if (line == "START_RESPONDER") {
      // Responder is likely already started by main loop, but this is an example integration point
      Debug::Log.info(TAG_PWN, "Responder activated by script.");
  }
  else if (line.rfind("WAIT ", 0) == 0) {
      int waitTime = std::stoi(line.substr(5));
      delay(waitTime);
  }
}

void AutoPwnEngine::end() {
  isEnabled = false;
  isRunning = false;
}
