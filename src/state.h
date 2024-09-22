#ifndef DEFS_H_H
#define DEFS_H_H

#include <vector>
#include <string>
#include "shader.h"
#include "GLFW/glfw3.h"
#include "cursor.h"
#include "highlighting.h"
#include "providers.h"
#include "utf8String.h"
#include "vim.h"
#include "accent.h"
void add_window(std::string p);
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
  Accent accentManager;
  std::string lastCmd;
  bool showLineNumbers = true;
  bool lineWrapping = false;
  bool isCommandRunning = false;
  bool openNewWindow;
  CursorEntry lastCommandOutCursor;
  int mode = 0;
  int round = 0;
  int fontSize;
  Vim *vim = nullptr;
  State();
  State(float w, float h, int fontSize);

  void invalidateCache();
  void freeVim();
  CursorEntry *hasEditedBuffer();
  void startReplace();
  void tryComment();
  void registerVim();
  bool checkCommandRun();
  void activateLastCommandBuffer();
  void killCommand();
  void checkChanged();
  void shellCommand();
  void switchMode();
  void increaseFontSize(int value);
  void toggleSelection();
  void fastSwitch();
  void switchBuffer();
  void tryPaste();
  void cut();
  void tryCopy();
  void tryCopyInput(Utf8String &in);
  void save();
  void saveNew();
  void switchLineHighlightMode();
  void changeFont();
  void open(bool inNewWindow = false);
  void setTheme();
  void command();
  bool execCommand(std::string command);
  void runCommand(std::string cmd);
  void reHighlight();
  void undo();
  void search();
  const Language *try_load_language(const std::string &name,
                                    const std::string &ext);
  void directlyEnableLanguage(std::string name);
  void tryEnableHighlighting();
  void inform(bool success, bool shift_pressed, bool ctrl_pressed = false);
  const std::vector<const Language *> getAllLanguages();
  void provideComplete(bool reverse);
  void fold();
  void renderCoords();
  void gotoLine();
  Utf8String getTabInfo();

  void deleteActive();
  void rotateBuffer();
  void toggleLineWrapping();
  void deleteCursor(size_t index);

  void activateCursor(size_t cursorIndex);
  void addCursor(std::string path);
  void addCursorWithExisting(std::string path);
  void init();

private:
  void activate_entries();
};

#endif
