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
#include "GLFW/glfw3.h"
#include "highlighting.h"
#include "la.h"
#include "utils.h"
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
  std::string highlightLine = "full";
  std::atomic_bool command_running;
  std::atomic_uint32_t command_pid = 0;
  std::string lastCommand = "";
  std::thread command_thread;
  Provider() {
    fs::path *homeDir = getHomeFolder();
    if (homeDir) {
      fs::path configDir = *homeDir / ".ledit";
      if (fs::exists(configDir)) {
        fs::path configFile = configDir / "config.json";
        configPath = configFile.generic_string();
        if (fs::exists(configFile)) {
          std::string contents = file_to_string(configPath);
          json parsed = json::parse(contents);
          parseConfig(&parsed);
        } else {
          json j;
          parseConfig(&j);
        }
        fs::path languages = configDir / "languages";
        if (fs::exists(languages)) {
          if (fs::is_directory(languages)) {
            for (const auto &entry : fs::directory_iterator(languages)) {
              std::string contents =
                  file_to_string(entry.path().generic_string());
              json parsed = json::parse(contents);
              loadExtraLanguage(parsed);
            }
          }
        }
        fs::path vimRemapPath = configDir / "vim_keys.json";
        if (fs::exists(vimRemapPath)) {
          std::string contents = file_to_string(vimRemapPath.generic_string());
          json parsed = json::parse(contents);
          setVimRemaps(parsed);
        }
      } else {
        fs::create_directory(configDir);
        json j;
        parseConfig(&j);
      }
      delete homeDir;
    } else {
      std::cerr << "Failed to load home env var\n";
    }
  }
  std::string getConfigPath() {
    fs::path *homeDir = getHomeFolder();
    if (!homeDir)
      return "";
    fs::path config = (*homeDir) / ".ledit" / "config.json";
    auto str = config.generic_string();
    delete homeDir;
    return str;
  }
  void reloadConfig() {
    std::string p = getConfigPath();
    if (!fs::exists(p))
      return;
    std::string contents = file_to_string(p);
    json parsed = json::parse(contents);
    parseConfig(&parsed);
  }
  bool killCommand() {
    if (!command_running)
      return false;
#ifdef _WIN32
    HANDLE processHandle = OpenProcess(PROCESS_TERMINATE, FALSE, command_pid);
    if (processHandle == NULL)
      return false;
    BOOL result = TerminateProcess(processHandle, 9);
    CloseHandle(processHandle);
#else
    auto out = kill(command_pid, SIGKILL);
#endif
    return true;
  }
  int runCommand(std::string command) {
    if (command_running)
      return 0;
    command_running = true;
    lastCommand = command;
    std::chrono::time_point<std::chrono::system_clock> now =
        std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    commandStartTime =
        std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    command_thread = std::thread([this, command]() {
#ifdef _WIN32
      HANDLE hStdoutRead, hStdoutWrite;
      SECURITY_ATTRIBUTES saAttr;
      saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
      saAttr.bInheritHandle = TRUE;
      saAttr.lpSecurityDescriptor = NULL;
      if (!CreatePipe(&hStdoutRead, &hStdoutWrite, &saAttr, 0)) {
        commandExitCode = 1;
        lastCommandOutput = "__LEDIT__: SPAWN ERROR: Couldn't pipe";
        command_running = false;
        command_pid = 0;
        glfwPostEmptyEvent();
        return 0;
      }
      SetHandleInformation(hStdoutRead, HANDLE_FLAG_INHERIT, 0);
      PROCESS_INFORMATION piProcInfo;
      STARTUPINFO siStartInfo;
      BOOL bSuccess = FALSE;
      ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
      ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
      siStartInfo.cb = sizeof(STARTUPINFO);
      siStartInfo.hStdError = hStdoutWrite;
      siStartInfo.hStdOutput = hStdoutWrite;
      siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
      bSuccess = CreateProcess(
          NULL,
          const_cast<char *>(command.c_str()), // command line
          NULL, NULL, TRUE, 0, NULL, NULL, &siStartInfo, &piProcInfo);
      if (!bSuccess) {
        CloseHandle(hStdoutWrite);
        CloseHandle(hStdoutRead);
        commandExitCode = 1;
        lastCommandOutput = "__LEDIT__: SPAWN ERROR: CreateProcess failed";
        command_running = false;
        command_pid = 0;
        glfwPostEmptyEvent();
        return 0;
      }
      command_pid = piProcInfo.dwProcessId;
      CloseHandle(hStdoutWrite);
      CloseHandle(piProcInfo.hThread);

      DWORD dwRead;
      CHAR chBuf[4096];
      std::string result;

      for (;;) {
        bSuccess = ReadFile(hStdoutRead, chBuf, 4096, &dwRead, NULL);
        if (!bSuccess || dwRead == 0)
          break;
        std::string str(chBuf, dwRead);
        result += str;
      }

      CloseHandle(hStdoutRead);

      WaitForSingleObject(piProcInfo.hProcess, INFINITE);

      DWORD exitCode;
      bool failedRetrieve = false;
      if (!GetExitCodeProcess(piProcInfo.hProcess, &exitCode)) {
        failedRetrieve = true;
        commandExitCode = 1;
      } else {
        commandExitCode = exitCode;
      }

      CloseHandle(piProcInfo.hProcess);
      result += "\n -- " + std::to_string(exitCode) +
                (failedRetrieve ? " (retrieve failed)" : "") + " ";
      {
        std::chrono::time_point<std::chrono::system_clock> now =
            std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto msNow =
            std::chrono::duration_cast<std::chrono::milliseconds>(duration)
                .count();
        auto runDuration = msNow - commandStartTime;
        double secs = (double)runDuration / 1000;
        result += toFixed(secs, 2) + "s";
      }
      result += " --";
      lastCommandOutput = result;
      command_running = false;
      command_pid = 0;
      glfwPostEmptyEvent();
#else
      int pipefd[2];
      if (pipe(pipefd) == -1) {
        commandExitCode = 1;
        lastCommandOutput = "__LEDIT__: SPAWN ERROR: Couldn't pipe";
        command_running = false;
        command_pid = 0;
        glfwPostEmptyEvent();
        return 0;
      }
      pid_t pid = fork();
      if (pid == -1) {
        commandExitCode = 1;
        lastCommandOutput = "__LEDIT__: SPAWN ERROR: fork() syscall: -1";
        command_running = false;
        command_pid = 0;
        glfwPostEmptyEvent();
        return 0;
      }
      if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        execlp("/bin/sh", "sh", "-c", command.c_str(), NULL);
        exit(EXIT_FAILURE);
      } else {
        command_pid = pid;
        close(pipefd[1]);
        int exitCode;
        std::string result;
        char buffer[1024];
        ssize_t count;
        while ((count = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
          buffer[count] = '\0';
          result += buffer;
        }
        close(pipefd[0]);
        waitpid(pid, &exitCode, 0);

        commandExitCode = exitCode;
        result += "\n -- " + std::to_string(exitCode) + " ";
        {
          std::chrono::time_point<std::chrono::system_clock> now =
              std::chrono::system_clock::now();
          auto duration = now.time_since_epoch();
          auto msNow =
              std::chrono::duration_cast<std::chrono::milliseconds>(duration)
                  .count();
          auto runDuration = msNow - commandStartTime;
          double secs = (double)runDuration / 1000;
          result += toFixed(secs, 2) + "s";
        }
        result += " --";
        lastCommandOutput = result;
        command_running = false;
        command_pid = 0;
        glfwPostEmptyEvent();
      }

#endif
      return 0;
    });

    return 0;
  }
  std::string getBranchName(std::string path) {
#ifdef LEDIT_DEBUG
    if (true)
      return "";
#endif
    std::string asPath = fs::path(path).parent_path().generic_string();
    const char *as_cstr = asPath.c_str();
    std::string branch = "";
#ifdef _WIN32
    std::string command = "cd " + asPath + " && git branch";
    FILE *pipe = _popen(command.c_str(), "r");
    if (pipe == NULL)
      return branch;
    char buffer[1024];
    while (fgets(buffer, sizeof buffer, pipe) != NULL) {
      branch += buffer;
    }
    _pclose(pipe);
#else
    int fd[2], pid;
    pipe(fd);
    pid = fork();
    if (pid == 0) {
      close(1);
      dup(fd[1]);
      close(0);
      close(2);
      close(fd[0]);
      close(fd[1]);
      const char *args[] = {"git", "branch", nullptr};
      chdir(as_cstr);
      execvp("git", static_cast<char *const *>((void *)args));
      exit(errno);
    } else {
      close(fd[1]);
      while (true) {
        char buffer[1024];
        int received = read(fd[0], buffer, 1024);
        if (received == 0)
          break;
        branch += std::string(buffer, received);
      }
      close(fd[0]);
      kill(pid, SIGTERM);
      wait(&pid);
    }
#endif
    if (!branch.length())
      return branch;
    std::string finalBranch = branch.substr(branch.find("* ") + 2);
    return finalBranch.substr(0, finalBranch.find("\n"));
  }
  std::string getCwdFormatted() {
    std::string path =
#ifdef _WIN32
        std::filesystem::current_path().generic_string();
#else
        std::filesystem::current_path();
#endif
    fs::path *homeDir = getHomeFolder();
    bool isHomeDirectory =
        homeDir != nullptr && path.find((*homeDir).generic_string()) == 0;
    std::string target = path;
    if (isHomeDirectory) {
      target = target.substr((*homeDir).generic_string().length());
      target = "~" + target;
    }
    if (homeDir != nullptr)
      delete homeDir;
    return target;
  }
  Vec4f getVecOrDefault(json o, const std::string entry, Vec4f def) {
    if (!o.contains(entry))
      return def;
    json e = o[entry];
    if (!e.is_array() || e.size() != 4)
      return def;
    for (auto &element : e) {
      int val = element;
      if (val < 0 || val > 255)
        return def;
    }
    return vec4f(((float)e[0] / (float)255), ((float)e[1] / (float)255),
                 ((float)e[2] / (float)255), ((float)e[3] / (float)255));
  }
  bool getBoolOrDefault(json o, const std::string entry, bool def) {
    if (!o.contains(entry))
      return def;
    json e = o[entry];
    if (!e.is_boolean())
      return def;

    return (bool)e;
  }
  int32_t getNumberOrDefault(json o, const std::string entry, int32_t def) {
    if (!o.contains(entry))
      return def;
    json e = o[entry];
    if (!e.is_number())
      return def;

    return (int32_t)e;
  }
  std::string getStringOrDefault(json o, const std::string entry,
                                 std::string def) {
    if (!o.contains(entry))
      return def;
    std::string e = o[entry];
    return e;
  }
  std::string getPathOrDefault(json o, const std::string entry,
                               std::string def) {
    if (!o.contains(entry))
      return def;
    std::string e = o[entry];
    if (!fs::exists(e))
      return def;
    return e;
  }
  const std::string getDefaultFontPath() {
#ifdef _WIN32
    return (getDefaultFontDir() / "consola.ttf").generic_string();
#endif
#ifdef __APPLE__
    return (getDefaultFontDir() / "Monaco.ttf").generic_string();
#endif
#ifdef __linux__
    FcConfig *config = FcInitLoadConfigAndFonts();
    FcPattern *pat = FcPatternCreate();
    FcObjectSet *objectSet = FcObjectSetBuild(FC_FAMILY, FC_STYLE, FC_LANG,
                                              FC_FILE, FC_SPACING, nullptr);
    FcFontSet *fSet = FcFontList(config, pat, objectSet);
    std::vector<FontEntry> results;
    for (int i = 0; fSet && i < fSet->nfont; i++) {
      FcPattern *font = fSet->fonts[i];
      FcChar8 *file, *family, *style;
      int spacing;
      if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch &&
          FcPatternGetInteger(font, FC_SPACING, 0, &spacing) == FcResultMatch &&
          FcPatternGetString(font, FC_FAMILY, 0, &family) == FcResultMatch &&
          FcPatternGetString(font, FC_STYLE, 0, &style) == FcResultMatch) {
        if (spacing == 100)
          results.push_back({std::string((const char *)file),
                             std::string((const char *)family),
                             std::string((const char *)style)});
      }
    }
    if (fSet)
      FcFontSetDestroy(fSet);
    for (auto &entry : results) {
      if (entry.name == "Hack" && entry.type == "Regular")
        return entry.path;
    }
    for (auto &entry : results) {
      if (entry.type == "Regular")
        return entry.path;
    }
    return results[0].path;
#endif
  }
  const fs::path getDefaultFontDir() {
// no idea how to do this differently
#ifdef _WIN32
    return "C:\\Windows\\Fonts";
#endif
#ifdef __APPLE__
    return "/System/Library/Fonts";
#else
    // fontconfig used
    return "";
#endif
  }
  void setVimRemaps(json &in) {
    for (auto &entry : in.items()) {
      std::string k = entry.key();
      vimRemaps[k[0]] = entry.value();
    }
  }
  void loadExtraLanguage(json &entry) {
    if (!entry.is_object())
      return;
    Language language;
    language.modeName = entry["mode_name"];
    if (entry.contains("key_words") && entry["key_words"].is_array()) {
      for (auto &word : entry["key_words"])
        language.keyWords.push_back(word);
    }
    if (entry.contains("special_words") && entry["special_words"].is_array()) {
      for (auto &word : entry["special_words"])
        language.specialWords.push_back(word);
    }
    language.singleLineComment = entry.contains("single_line_comment")
                                     ? entry["single_line_comment"]
                                     : "";
    if (entry.contains("multi_line_comment") &&
        entry["multi_line_comment"].is_array()) {
      language.multiLineComment = std::pair(entry["multi_line_comment"][0],
                                            entry["multi_line_comment"][1]);
    }
    language.stringCharacters =
        entry.contains("string_characters") ? entry["string_characters"] : "";
    if (entry.contains("escape_character")) {
      std::string content = entry["escape_character"];
      language.escapeChar = content[0];
    }
    if (entry.contains("file_extensions") &&
        entry["file_extensions"].is_array()) {
      for (auto &word : entry["file_extensions"])
        language.fileExtensions.push_back(word);
    }
    if (language.modeName.length() && language.fileExtensions.size()) {
      extraLanguages.push_back(language);
    }
  }
  void parseConfig(json *configRoot) {
    if (configRoot->contains("colors")) {
      json configColors = (*configRoot)["colors"];
      colors.string_color =
          getVecOrDefault(configColors, "string_color", colors.string_color);
      colors.default_color =
          getVecOrDefault(configColors, "default_color", colors.default_color);
      colors.keyword_color =
          getVecOrDefault(configColors, "keyword_color", colors.keyword_color);
      colors.special_color =
          getVecOrDefault(configColors, "special_color", colors.special_color);
      colors.comment_color =
          getVecOrDefault(configColors, "comment_color", colors.comment_color);
      colors.background_color = getVecOrDefault(
          configColors, "background_color", colors.background_color);
      colors.highlight_color = getVecOrDefault(configColors, "highlight_color",
                                               colors.highlight_color);
      colors.selection_color = getVecOrDefault(configColors, "selection_color",
                                               colors.selection_color);
      colors.number_color =
          getVecOrDefault(configColors, "number_color", colors.number_color);
      colors.status_color =
          getVecOrDefault(configColors, "status_color", colors.status_color);
      colors.line_number_color = getVecOrDefault(
          configColors, "line_number_color", colors.line_number_color);
      colors.minibuffer_color = getVecOrDefault(
          configColors, "minibuffer_color", colors.minibuffer_color);
      colors.cursor_color_standard = getVecOrDefault(
          configColors, "cursor_color", colors.cursor_color_standard);
      colors.cursor_color_vim = getVecOrDefault(
          configColors, "vim_cursor_color", colors.cursor_color_vim);
    }
    if (configRoot->contains("commands")) {
      for (const auto &entry : (*configRoot)["commands"].items()) {
        commands[entry.key()] = entry.value();
      }
    }
    fontPath = getPathOrDefault(*configRoot, "font_face", fontPath);
    if (configRoot->contains("extra_fonts")) {
      json &extra_fonts = (*configRoot)["extra_fonts"];
      for (auto &item : extra_fonts) {
        std::string str = item;
        extraFonts.push_back(str);
      }
    }
    useSpaces = getBoolOrDefault(*configRoot, "use_spaces", useSpaces);
    saveBeforeCommand =
        getBoolOrDefault(*configRoot, "save_before_command", saveBeforeCommand);
      autoOpenCommandOut =
        getBoolOrDefault(*configRoot, "auto_open_cmd_output", autoOpenCommandOut);
    autoReload = getBoolOrDefault(*configRoot, "auto_reload", autoReload);
    vim_emulation = getBoolOrDefault(*configRoot, "vim_mode", vim_emulation);
    tabWidth = getNumberOrDefault(*configRoot, "tab_width", tabWidth);
    allowTransparency =
        getBoolOrDefault(*configRoot, "window_transparency", allowTransparency);
    lineNumbers = getBoolOrDefault(*configRoot, "line_numbers", lineNumbers);
    lineWrapping = getBoolOrDefault(*configRoot, "line_wrapping", lineWrapping);
    highlightLine =
        getStringOrDefault(*configRoot, "highlight_active_line", highlightLine);
  }
  json vecToJson(Vec4f value) {
    json j;
    j.push_back((int)(255 * value.x));
    j.push_back((int)(255 * value.y));
    j.push_back((int)(255 * value.z));
    j.push_back((int)(255 * value.w));
    return j;
  }
  void writeConfig() {
    if (!configPath.length())
      return;
    json config;
    json cColors;
    cColors["string_color"] = vecToJson(colors.string_color);
    cColors["cursor_color"] = vecToJson(colors.cursor_color_standard);
    cColors["vim_cursor_color"] = vecToJson(colors.cursor_color_vim);
    cColors["default_color"] = vecToJson(colors.default_color);
    cColors["keyword_color"] = vecToJson(colors.keyword_color);
    cColors["special_color"] = vecToJson(colors.special_color);
    cColors["comment_color"] = vecToJson(colors.comment_color);
    cColors["background_color"] = vecToJson(colors.background_color);
    cColors["highlight_color"] = vecToJson(colors.highlight_color);
    cColors["selection_color"] = vecToJson(colors.selection_color);
    cColors["number_color"] = vecToJson(colors.number_color);
    cColors["status_color"] = vecToJson(colors.status_color);
    cColors["line_number_color"] = vecToJson(colors.line_number_color);
    cColors["minibuffer_color"] = vecToJson(colors.minibuffer_color);
    config["font_face"] = fontPath;
    config["window_transparency"] = allowTransparency;
    config["use_spaces"] = useSpaces;
    config["line_wrapping"] = lineWrapping;
    config["line_numbers"] = lineNumbers;
    config["highlight_active_line"] = highlightLine;
    config["auto_open_cmd_output"] = autoOpenCommandOut;
    config["tab_width"] = tabWidth;
    config["colors"] = cColors;
    if (extraFonts.size()) {
      json extra_fonts;
      for (auto &f : extraFonts) {
        extra_fonts.push_back(f);
      }
      config["extra_fonts"] = extra_fonts;
    }
    const std::string contents = config.dump(2);
    string_to_file(configPath, contents);
  }
  std::string getLast() {
    if (!folderEntries.size())
      return "";
    return folderEntries[offset];
  }
  std::string getFileToOpen(std::string path, bool reverse) {
    if (!fs::exists(path) || !fs::is_directory(path))
      return "";
    if (lastProvidedFolder == path) {
      offset += reverse ? -1 : 1;

      if (offset == folderEntries.size())
        offset = 0;
      else if (offset == -1)
        offset = folderEntries.size() - 1;
      return folderEntries[offset];
    }
    folderEntries.clear();
    for (auto const &dir_entry : fs::directory_iterator{path}) {
      folderEntries.push_back(dir_entry.path().string());
    }
    offset = 0;
    lastProvidedFolder = path;
    return folderEntries[offset];
  }

private:
  fs::path *getHomeFolder() {
#ifdef _WIN32
    const char *home = getenv("USERPROFILE");
#else
    const char *home = getenv("HOME");
#endif
    if (home)
      return new fs::path(home);
    return nullptr;
  }
};

#endif
