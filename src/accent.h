#ifndef LEDIT_ACCENT_H
#define LEDIT_ACCENT_H

#include "cursor.h"
#include "utf8String.h"
#include "../third-party/glfw/include/GLFW/glfw3.h"


struct LastEvent {
  int action, key;
};

class Accent {
public:
  Cursor *cursor = nullptr;

  void processEvent(GLFWwindow *window, int key, int scancode, int action,
                    int mods);
  void interrupt();
  bool checkBlock(int key, int action);
  bool shouldBlock();
  bool blockCp(uint32_t cp);
  Utf8String getStatus();
  bool needInterrupt = false;

private:
  bool wasShift = false;
  bool shift = false;
  int state = 0;
  int lastKey;
  uint64_t start;
  void provideComplete();
  void doInsert(int num);
  char getCharFromKey(int k);
};

#endif