#ifndef PROVIDERS_H
#define PROVIDERS_H
#ifdef __APPLE__
#include <sys/signal.h>
#endif
#include <unordered_map>
#include <atomic>
#include <vector>
#include <thread>
#include <chrono>
#include <filesystem>
#include "highlighting.h"
#include "la.h"
#include "../third-party/json/json.hpp"
#ifndef _WIN32
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#else
#include <windows.h>
#endif
#ifdef __linux__
#include <fontconfig/fontconfig.h>
struct FontEntry {
  std::string path;
  std::string name;
  std::string type;
};
#endif

namespace fs = std::filesystem;
using json = nlohmann::json;

class Provider {
public:
  std::string lastProvidedFolder;
  std::vector<std::string> folderEntries;
  std::unordered_map<std::string, std::string> commands;
  int offset = 0;
  EditorColors colors;
  std::string fontPath = getDefaultFontPath();
  std::vector<std::string> extraFonts;
  std::unordered_map<char32_t, std::string> vimRemaps;
  std::vector<Language> extraLanguages;
  std::string configPath;
  std::string lastCommandOutput;
  int commandExitCode = 0;
  int fontSize = 25;
#ifdef __APPLE__
  bool titleBarColorSet = false;
  Vec4f titleBarColor;
#endif
  int32_t tabWidth = 2;
  bool useSpaces = true;
  bool autoReload = false;
  bool vim_emulation = false;
  uint64_t commandStartTime = 0;
  bool saveBeforeCommand = false;
  bool allowTransparency = false;
  bool lineNumbers = true;
  bool lineWrapping = false;
  bool autoOpenCommandOut = false;
  bool commandHadOutput = false;
  bool relativeLineNumbers = false;
  bool useCapsAsEscape = false;
  bool enableAccents = true;
  std::string theme = "default";
  std::string highlightLine = "full";
  std::atomic_bool command_running = false;
  std::atomic_int32_t command_pid = 0;
  std::string lastCommand = "";
  std::thread command_thread;
  Provider();
  fs::path getConfigDir(fs::path& base);
  std::string getConfigPath();
  void reloadConfig();
  bool loadTheme(std::string name);
  bool killCommand();
  int runCommand(std::string command);
  std::pair<int, std::string> runDirect(std::string command,
                                        std::string folder = "");
  std::string getBranchName(std::string path);
  std::string getCwdFormatted();
  Vec4f getVecOrDefault(json o, const std::string entry, Vec4f def);
  bool getBoolOrDefault(json o, const std::string entry, bool def);
  int32_t getNumberOrDefault(json o, const std::string entry, int32_t def);
  std::string getStringOrDefault(json o, const std::string entry,
                                 std::string def);
  std::string getPathOrDefault(json o, const std::string entry,
                               std::string def);
  const std::string getDefaultFontPath();
  const fs::path getDefaultFontDir();
  void setVimRemaps(json &in);
  void loadExtraLanguage(json &entry);
  void parseConfig(json *configRoot);
  json vecToJson(Vec4f value);
  void writeConfig();
  std::string getLast();
  std::string getFileToOpen(std::string inp, bool reverse);

private:
  fs::path *getHomeFolder();
};

#endif
