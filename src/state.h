#ifndef DEFS_H_H
#define DEFS_H_H

#include <iterator>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include "shader.h"
#include "cursor.h"
#include "highlighting.h"
#include "languages.h"
#include "providers.h"
#include "u8String.h"
#include "utf8String.h"
#include "utils.h"
#include "vim.h"
void register_vim_commands(Vim &vim, State &state);
struct CursorEntry {
  Cursor cursor;
  std::string path;
};
struct ReplaceBuffer {
  Utf8String search = U"";
  Utf8String replace = U"";
};
class State {
public:
  GLuint vao, vbo;
  bool focused = true;
  bool exitFlag = false;
  bool exitLoop = false;
  bool cacheValid = false;
  GLuint sel_vao, sel_vbo;
  GLuint highlight_vao, highlight_vbo;
  Cursor *cursor;
  std::vector<CursorEntry *> cursors;
  size_t activeIndex;
  Highlighter highlighter;
  Provider provider;
  FontAtlas *atlas = nullptr;
  GLFWwindow *window;
  ReplaceBuffer replaceBuffer;
  float WIDTH, HEIGHT;
  bool hasHighlighting;
  bool ctrlPressed = false;
  std::string path;
  Utf8String fileName;
  Utf8String status;
  Utf8String miniBuf;
  Utf8String dummyBuf;
  std::string lastCmd;
  bool showLineNumbers = true;
  bool highlightLine = true;
  bool lineWrapping = false;
  bool isCommandRunning = false;
  CursorEntry lastCommandOutCursor;
  int mode = 0;
  int round = 0;
  int fontSize;
  Vim *vim;
  State() {}

  void invalidateCache() { cacheValid = false; }

  CursorEntry *hasEditedBuffer() {
    for (CursorEntry *cur : cursors) {
      if (cur->cursor.edited)
        return cur;
    }
    return nullptr;
  }
  void startReplace() {
    if (mode != 0)
      return;
    mode = 30;
    status = U"Search: ";
    miniBuf = replaceBuffer.search;
    cursor->bindTo(&miniBuf);
  }
  void tryComment() {
    if (!hasHighlighting)
      return;
    cursor->comment(highlighter.language.singleLineComment);
  }
  void registerVim() {
    this->vim = new Vim(this);
    register_vim_commands(*vim, *this);
  }
  bool checkCommandRun() {
    if (!provider.command_running && isCommandRunning) {
      std::chrono::time_point<std::chrono::system_clock> now =
          std::chrono::system_clock::now();
      auto duration = now.time_since_epoch();
      auto msNow =
          std::chrono::duration_cast<std::chrono::milliseconds>(duration)
              .count();
      auto runDuration = msNow - provider.commandStartTime;
      double secs = (double)runDuration / 1000;

      provider.command_thread.join();
      isCommandRunning = false;
      checkChanged();
      status = U"cmd: [" + Utf8String(lastCmd) + U" | " +
               Utf8String(toFixed(secs, 2)) +
               U"s"
               U"]: " +
               Utf8String(std::to_string(provider.commandExitCode));
      return true;
    }
    return false;
  }
  void activateLastCommandBuffer() {
    if (isCommandRunning)
      return;
    if (provider.lastCommandOutput.size() == 0)
      return;
    lastCommandOutCursor.cursor.reset();
    lastCommandOutCursor.path = "";
    lastCommandOutCursor.cursor.appendWithLines(
        Utf8String(provider.lastCommandOutput));
    auto offset =
        std::find(cursors.begin(), cursors.end(), &lastCommandOutCursor);
    if (offset == cursors.end()) {
      cursors.push_back(&lastCommandOutCursor);
      activateCursor(cursors.size() - 1);
    } else {
      activateCursor(offset - cursors.begin());
    }
  }
  void killCommand() { provider.killCommand(); }
  void checkChanged() {
    if (!path.length())
      return;
    cursor->branch = provider.getBranchName(path);
    auto changed = cursor->didChange(path);
    if (changed) {
      if (provider.autoReload) {
        cursor->reloadFile(path);
        reHighlight();
        status = U"Reloaded";
        return;
      }
      miniBuf = U"";
      mode = 36;
      status = U"[" + create(path) + U"]: Changed on disk, reload?";
      cursor->bindTo(&dummyBuf);
    }
  }
  void switchMode() {
    if (mode != 0)
      return;
    round = 0;
    miniBuf = U"Text";
    status = U"Mode: ";
    cursor->bindTo(&dummyBuf);
    mode = 25;
  }
  void increaseFontSize(float value) {
    if (mode != 0)
      return;

    atlas->changeScale(value);
    status = U"resize: [" + numberToString(atlas->fs * atlas->scale) + U"]";
  }
  void toggleSelection() {
    if (mode != 0)
      return;
    if (cursor->selection.active)
      cursor->selection.stop();
    else
      cursor->selection.activate(cursor->x, cursor->y);
    renderCoords();
  }
  void fastSwitch() {
    if (mode != 0)
      return;
    if (cursors.size() == 1) {
      status = U"No other buffers in cache";
      return;
    }
    size_t index = 0;
    for (size_t i = 0; i < cursors.size(); i++) {
      if (&(cursors[i]->cursor) == cursor) {
        index = i;
        break;
      }
    }
    if (index == cursors.size() - 1)
      index = 0;
    else
      index++;
    activateCursor(index);
    std::string &path = cursors[index]->path;
    status = U"Switched to: " + create(path.length() ? path : "New File");
  }
  void switchBuffer() {
    if (mode != 0 && mode != 5)
      return;
    if (mode == 0) {
      if (cursors.size() == 1) {
        status = U"No other buffers in cache";
        return;
      }
      round = 0;
      miniBuf = create(cursors[0]->path);
      cursor->bindTo(&miniBuf);
      mode = 5;
      status = U"Switch to: ";
    } else {
      round++;
      if (round == cursors.size())
        round = 0;
      miniBuf = create(cursors[round]->path);
    }
  }
  void tryPaste() {
    const char *contents = glfwGetClipboardString(NULL);
    if (contents) {
      Utf8String str = create(std::string(contents));
      cursor->appendWithLines(str, vim != nullptr);
      if (mode != 0)
        return;
      if (hasHighlighting)
        highlighter.highlight(cursor->lines, &provider.colors, cursor->skip,
                              cursor->maxLines, cursor->y,
                              cursor->history.size());
      status = U"Pasted " + numberToString(str.length()) + U" Characters";
    }
  }
  void cut() {
    if (!cursor->selection.active) {
      status = U"Aborted: No selection";
      return;
    }
    std::string content = cursor->getSelection();
    glfwSetClipboardString(NULL, content.c_str());
    cursor->deleteSelection();
    cursor->selection.stop();
    status = U"Cut " + numberToString(content.length()) + U" Characters";
  }
  void tryCopy() {
    if (!cursor->selection.active) {
      status = U"Aborted: No selection";
      return;
    }
    std::string content = cursor->getSelection();
    glfwSetClipboardString(NULL, content.c_str());
    cursor->selection.stop();
    status = U"Copied " + numberToString(content.length()) + U" Characters";
  }
  void tryCopyInput(Utf8String &in) {
    const std::string &content = in.getStrRef();
    glfwSetClipboardString(NULL, content.c_str());
  }
  void save() {
    if (mode != 0 && mode != 40)
      return;
    if (!path.length()) {
      saveNew();
      return;
    }
    cursor->saveTo(path);
    status = U"Saved: " + create(path);
  }
  void saveNew() {
    if (mode != 0)
      return;
    miniBuf = U"";
    cursor->bindTo(&miniBuf);
    mode = 1;
    status = U"Save to[" + create(provider.getCwdFormatted()) + U"]: ";
  }
  void changeFont() {
    if (mode != 0)
      return;
    miniBuf = create(provider.fontPath);
    cursor->bindTo(&miniBuf);
    mode = 15;
    status = U"Set font: ";
  }
  void open() {
    if (mode != 0)
      return;
    miniBuf = U"./";
    provider.lastProvidedFolder = "";
    cursor->bindTo(&miniBuf);
    mode = 4;
    status = U"Open [" + create(provider.getCwdFormatted()) + U"]: ";
  }
  void command() {
    if (mode != 0)
      return;
    if (lastCmd.size())
      miniBuf = Utf8String(lastCmd);
    else
      miniBuf = U"";
    cursor->bindTo(&miniBuf);
    mode = 40;
    round = 0;
    status = U"cmd: ";
  }
  void runCommand(std::string cmd) {
    if (!cmd.size())
      return;
    if (!provider.commands.count(cmd)) {
      status = U"Unregistered Command";
      return;
    }
    if (provider.command_running) {
      status = U"cmd: [" + Utf8String(lastCmd) + U"]: is running";
      return;
    }
    std::map<std::string, std::string> replaces;

    std::string command = provider.commands[cmd];
    if (path.length()) {
      fs::path file(path);
      std::string extension = file.extension().generic_string();
      std::string filename = file.filename().generic_string();
      std::string basename =
          filename.substr(0, filename.length() - extension.length());
      replaces["$file_path"] = path;
      replaces["$file_name"] = filename;
      replaces["$file_basename"] = basename;
      replaces["$file_extension"] = extension;
    }
    replaces["$cwd"] = provider.getCwdFormatted();
    if (cursor->selection.active) {
      replaces["$selection_content"] = cursor->getSelection();
    } else {
      replaces["$selection_content"] = "";
    }

    for (auto &entry : replaces) {
      auto index = command.find(entry.first);
      size_t offset = 0;
      while (index != std::string::npos) {
        if (index == 0 || command[index - 1] != '\\') {
          command.replace(index, entry.first.size(), entry.second);
        } else if (index > 0 || command[index - 1] == '\\') {
          offset = index + 1;
        }
        index = command.find(entry.first, offset);
      }
    }
    if (provider.saveBeforeCommand && path.length())
      save();
    lastCmd = cmd;
    isCommandRunning = true;
    provider.runCommand(command);
    status = U"cmd: [" + Utf8String(cmd) + U"]: executing";
  }
  void reHighlight() {
    if (hasHighlighting)
      highlighter.highlight(cursor->lines, &provider.colors, cursor->skip,
                            cursor->maxLines, cursor->y,
                            cursor->history.size());
  }
  void undo() {
    bool result = cursor->undo();
    status = result ? U"Undo" : U"Undo failed";
    if (result)
      reHighlight();
  }
  void search() {
    if (mode != 0)
      return;
    miniBuf = U"";
    cursor->bindTo(&miniBuf, true);
    mode = 2;
    status = U"Search: ";
  }
  const Language *try_load_language(const std::string &name,
                                    const std::string &ext) {
    for (const auto &language : provider.extraLanguages) {
      for (const auto &extension : language.fileExtensions) {
        if (extension == ext || extension == name)
          return &language;
      }
    }
    return has_language(name, ext);
  }
  void directlyEnableLanguage(std::string name) {
    const Language *lang =
        try_load_language(name, name.find_last_of(".") != std::string::npos
                                    ? name.substr(name.find_last_of(".") + 1)
                                    : name);
    if (lang) {
      highlighter.setLanguage(*lang, lang->modeName);
      highlighter.highlight(cursor->lines, &provider.colors, cursor->skip,
                            cursor->maxLines, cursor->y,
                            cursor->history.size());
      hasHighlighting = true;
    }
  }
  void tryEnableHighlighting() {
    fs::path path = fs::path(fileName.getStrRef());
    auto name = path.filename();
    auto extension = path.extension();
    std::string extension_str = extension.generic_string();
    const Language *lang = try_load_language(
        name.generic_string(),
        extension_str.length() ? extension_str.substr(1) : "");
    if (lang) {
      highlighter.setLanguage(*lang, lang->modeName);
      highlighter.highlight(cursor->lines, &provider.colors, cursor->skip,
                            cursor->maxLines, cursor->y,
                            cursor->history.size());
      hasHighlighting = true;
    } else {
      hasHighlighting = false;
    }
  }
  void inform(bool success, bool shift_pressed) {
    if (success) {
      if (mode == 1) { // save to
        bool result = cursor->saveTo(convert_str(miniBuf));
        if (result) {
          status = U"Saved to: " + miniBuf;
          if (!path.length()) {
            path = convert_str(miniBuf);
            cursors[activeIndex]->path = path;
            auto split = cursor->split(path, "/");
            std::string fName = split[split.size() - 1];
            fileName = create(fName);
            std::string window_name = "ledit: " + path;
            glfwSetWindowTitle(window, window_name.c_str());
            tryEnableHighlighting();
          }
        } else {
          status = U"Failed to save to: " + miniBuf;
        }
      } else if (mode == 2 || mode == 7) { // search
        status = cursor->search(miniBuf, false, mode != 7);
        if (mode == 7)
          mode = 2;
        // hacky shit
        if (status != U"[Not found]: ")
          mode = 6;
        return;
      } else if (mode == 6) { // search
        status = cursor->search(miniBuf, true);
        if (status == U"[No further matches]: ") {
          mode = 7;
        }
        return;
      } else if (mode == 3) { // gotoline
        auto line_str = convert_str(miniBuf);
        if (isSafeNumber(line_str)) {
          cursor->gotoLine(std::stoi(line_str));
          status = U"Jump to: " + miniBuf;
        } else {
          status = U"Invalid line: " + miniBuf;
        }
      } else if (mode == 4 || mode == 5) {
        cursor->unbind();
        if (mode == 5) {
          if (round != activeIndex) {
            activateCursor(round);
            status =
                U"Switched to: " + create(path.length() ? path : "New File");
          } else {
            status = U"Canceled";
          }
        } else {
          bool found = false;
          size_t fIndex = 0;
          auto converted = convert_str(miniBuf);
          for (size_t i = 0; i < cursors.size(); i++) {
            if (cursors[i]->path == converted) {
              found = true;
              fIndex = i;
              break;
            }
          }
          if (found && activeIndex != fIndex)
            activateCursor(fIndex);
          else if (!found)
            addCursor(converted);
        }

      } else if (mode == 15) {
        atlas->readFont(convert_str(miniBuf), fontSize, true);
        for (auto entry : provider.extraFonts) {
          atlas->readFont(path, fontSize);
        }
        provider.fontPath = convert_str(miniBuf);
        provider.writeConfig();
        status = U"Loaded font: " + miniBuf;
      } else if (mode == 25) {
        if (round == 0) {
          status = U"Mode: Text";
          hasHighlighting = false;
        } else {
          auto lang = getAllLanguages()[round - 1];
          highlighter.setLanguage(*lang, lang->modeName);
          hasHighlighting = true;
          status = U"Mode: " + miniBuf;
        }
      } else if (mode == 30) {
        replaceBuffer.search = miniBuf;
        miniBuf = replaceBuffer.replace;
        cursor->unbind();
        cursor->bindTo(&miniBuf);
        status = U"Replace: ";
        mode = 31;
        return;
      } else if (mode == 31) {
        mode = 32;
        replaceBuffer.replace = miniBuf;
        status = replaceBuffer.search + U" => " + replaceBuffer.replace;
        cursor->unbind();
        return;
      } else if (mode == 32) {
        if (shift_pressed) {
          auto count =
              cursor->replaceAll(replaceBuffer.search, replaceBuffer.replace);
          if (count)
            status = U"Replaced " + numberToString(count) + U" matches";
          else
            status = U"[No match]: " + replaceBuffer.search + U" => " +
                     replaceBuffer.replace;
        } else {
          auto result = cursor->replaceOne(replaceBuffer.search,
                                           replaceBuffer.replace, true);
          status =
              result + replaceBuffer.search + U" => " + replaceBuffer.replace;
          return;
        }
      } else if (mode == 36) {
        cursor->reloadFile(path);
        status = U"Reloaded";
      } else if (mode == 40) {
        runCommand(miniBuf.getStr());
      }
    } else {
      status = U"Aborted";
    }
    cursor->unbind();
    mode = 0;
  }
  const std::vector<const Language *> getAllLanguages() {
    std::vector<const Language *> l;
    for (const auto &lang : LANGUAGES) {
      l.push_back(&lang);
    }

    for (const auto &lang : provider.extraLanguages) {
      l.push_back(&lang);
    }
    return l;
  }
  void provideComplete(bool reverse) {
    if (mode == 4 || mode == 15 || mode == 1) {
      std::string convert = convert_str(miniBuf);
      std::string e = provider.getFileToOpen(
          convert == provider.getLast() ? provider.lastProvidedFolder : convert,
          reverse);
      if (!e.length())
        return;
      std::string p = provider.lastProvidedFolder;
      miniBuf = create(e);
    } else if (mode == 25) {
      const auto langs = getAllLanguages();
      if (reverse) {
        if (round == 0)
          round = langs.size();
        else
          round--;
      } else {
        if (round == langs.size())
          round = 0;
        else
          round++;
      }
      if (round == 0)
        miniBuf = U"Text";
      else
        miniBuf = create(langs[round - 1]->modeName);
    } else if (mode == 40) {
      int next = round + (reverse ? -1 : 1);
      if (next < 0)
        next = provider.commands.size() - 1;
      else if (next == provider.commands.size())
        next = 0;
      auto cursor = provider.commands.begin();
      std::advance(cursor, next);
      miniBuf = Utf8String(cursor->first);
      round = next;
    }
  }
  void renderCoords() {
    if (mode != 0)
      return;
    // if(hasHighlighting)
    // highlighter.highlight(cursor->lines, &provider.colors, cursor->skip,
    // cursor->maxLines, cursor->y);
    Utf8String branch;
    if (cursor->branch.length()) {
      branch = U" [git: " + create(cursor->branch) + U"]";
    }
    auto x = cursor->x + 1;
    if (vim && vim->getMode() != VimMode::INSERT &&
        x == cursor->getCurrentLineLength() + 1 && x > 1)
      x--;
    status = (vim ? Utf8String(vim->getModeName()) + U" " : U"") +
             numberToString(cursor->y + 1) + U":" + numberToString(x) + branch +
             U" [" + fileName + U": " +
             (hasHighlighting ? highlighter.languageName : U"Text") + U"]";
    if (cursor->selection.active)
      status +=
          U" Selected: [" + numberToString(cursor->getSelectionSize()) + U"]";
    if (vim && vim->getCount() > 0) {
      status += U" " + Utf8String(std::to_string(vim->getCount()));
    }
    if (atlas && atlas->errors.size())
      status += U" " + atlas->errors[0];
    if (isCommandRunning)
      status += U" cmd[" + Utf8String(lastCmd) + U"]: executing";
  }
  void gotoLine() {
    if (mode != 0)
      return;
    miniBuf = U"";
    cursor->bindTo(&miniBuf);
    mode = 3;
    status = U"Line: ";
  }
  Utf8String getTabInfo() {
    if (activeIndex == 0 && cursors.size() == 1)
      return U"[ 1 ]";
    Utf8String text = U"[ " + numberToString(activeIndex + 1) + U":" +
                      numberToString(cursors.size()) + U" ]";

    return text;
  }

  void deleteActive() { deleteCursor(activeIndex); }
  void rotateBuffer() {
    if (cursors.size() == 1)
      return;
    size_t next = activeIndex + 1;
    if (next == cursors.size())
      next = 0;
    activateCursor(next);
  }
  void toggleLineWrapping() {
    lineWrapping = !lineWrapping;
    status = U"(Experimental) Linewrapping: ";
    status += (lineWrapping ? U"True" : U"Off");
  }
  void deleteCursor(size_t index) {
    if (mode != 0 || cursors.size() == 1 || index >= cursors.size())
      return;
    CursorEntry *entry = cursors[index];
    if (entry != &lastCommandOutCursor) {
      delete entry;

      if (activeIndex != index)
        return;
    }
    size_t targetIndex = index == 0 ? 1 : index - 1;
    cursors.erase(cursors.begin() + index);
    activateCursor(targetIndex);
  }
  void activateCursor(size_t cursorIndex) {
    CursorEntry *entry = cursors[cursorIndex];
    activeIndex = cursorIndex;
    std::string path = entry->path;
    this->cursor = &(entry->cursor);
    if (vim)
      vim->setCursor(this->cursor);
    this->path = path;
    status = create(path);
    if (path.length()) {
      if (path == "-") {
        fileName = U"-(STDIN/OUT)";
        hasHighlighting = false;
        renderCoords();
      } else {
        auto split = entry->cursor.split(path, "/");
        fileName = create(split[split.size() - 1]);
        tryEnableHighlighting();
        checkChanged();
      }
    } else if (entry == &lastCommandOutCursor) {
      fileName = U"Output [" + Utf8String(lastCmd) + U"]";
      hasHighlighting = false;
      renderCoords();
    } else {
      fileName = U"New File";
      hasHighlighting = false;
      renderCoords();
    }
    std::string window_name = "ledit: " + (path.length() ? path : "New File");
    glfwSetWindowTitle(window, window_name.c_str());
  }
  void addCursor(std::string path) {
    if (path.length() && std::filesystem::is_directory(path))
      path = "";

    if (path.length())
      for (size_t i = 0; i < cursors.size(); i++) {
        auto *entry = cursors[i];
        if (entry->path == path) {
          activateCursor(i);
          return;
        }
      }

    Cursor newCursor = path.length() ? Cursor(path) : Cursor();
    if (path.length()) {
      newCursor.branch = provider.getBranchName(path);
    }
    CursorEntry *entry = new CursorEntry{newCursor, path};
    cursors.push_back(entry);
    activateCursor(cursors.size() - 1);
  }
  State(float w, float h, int fontSize) {

    this->fontSize = fontSize;
    WIDTH = w;
    HEIGHT = h;
  }
  void init() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // TODO set size of vbo base on buffer size?

    glBufferData(GL_ARRAY_BUFFER, sizeof(RenderChar) * 600 * 1000, nullptr,
                 GL_DYNAMIC_DRAW);
    activate_entries();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // //selection buffer;
    glGenVertexArrays(1, &sel_vao);
    glGenBuffers(1, &sel_vbo);
    glBindVertexArray(sel_vao);
    glBindBuffer(GL_ARRAY_BUFFER, sel_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SelectionEntry) * 16, nullptr,
                 GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(SelectionEntry),
                          (void *)offsetof(SelectionEntry, pos));
    glVertexAttribDivisor(0, 1);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SelectionEntry),
                          (void *)offsetof(SelectionEntry, size));
    glVertexAttribDivisor(1, 1);

    // //selection buffer;
    glGenVertexArrays(1, &highlight_vao);
    glGenBuffers(1, &highlight_vbo);
    glBindVertexArray(highlight_vao);
    glBindBuffer(GL_ARRAY_BUFFER, highlight_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SelectionEntry), nullptr,
                 GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(SelectionEntry),
                          (void *)offsetof(SelectionEntry, pos));
    glVertexAttribDivisor(0, 1);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SelectionEntry),
                          (void *)offsetof(SelectionEntry, size));
    glVertexAttribDivisor(1, 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

private:
  void activate_entries() {
    // pos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(RenderChar),
                          (void *)offsetof(RenderChar, pos));
    glVertexAttribDivisor(0, 1);

    // size
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(RenderChar),
                          (void *)offsetof(RenderChar, size));
    glVertexAttribDivisor(1, 1);

    // uv_pos
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(RenderChar),
                          (void *)offsetof(RenderChar, uv_pos));
    glVertexAttribDivisor(2, 1);
    // uv_size
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(RenderChar),
                          (void *)offsetof(RenderChar, uv_size));
    glVertexAttribDivisor(3, 1);
    // fg_color
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(RenderChar),
                          (void *)offsetof(RenderChar, fg_color));
    glVertexAttribDivisor(4, 1);
    // bg_color
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(RenderChar),
                          (void *)offsetof(RenderChar, bg_color));
    glVertexAttribDivisor(5, 1);

    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, sizeof(RenderChar),
                          (void *)offsetof(RenderChar, hasColor));
    glVertexAttribDivisor(6, 1);
  }
};

#endif
