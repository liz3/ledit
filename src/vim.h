#ifndef LEDIT_VIM_H
#define LEDIT_VIM_H

#include <iostream>
#include "la.h"
#include "cursor.h"
#include "utf8String.h"
#include <unordered_map>
#include <vector>
#include <functional>
class State;
class Vim;
enum class ResultType {
  Done,
  SetSelf,
  SetSelfIntercept,
  Emit,
  ExecuteSet,
  EmitAndSetFuture,
  Silent,
  Cancel
};

struct ActionResult {
  ResultType type = ResultType::Done;
  bool useKey = false;
  std::string action_name;
  std::string emit_next;
  bool allowCoords = true;
};

enum class Direction { UNSET, UP, DOWN, LEFT, RIGHT };
enum class ReplaceMode { UNSET, INNER, ALL };
struct MotionState {
  size_t count = 0;
  bool isInital = true;
  Direction direction = Direction::UNSET;
  ReplaceMode replaceMode = ReplaceMode::UNSET;
  std::string action;
  std::string future;
};
enum class VimMode { NORMAL, INSERT, VISUAL };
class Action {
public:
  virtual ~Action() = default;
  virtual ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                               Vim *vim) = 0;
  virtual ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                            Vim *vim) = 0;
};
struct Interceptor {
  virtual ActionResult intercept(char32_t in, Vim *vim, Cursor *cursor) = 0;
};
struct ActionTrie {
public:
  ActionTrie(Action *action, std::string action_name, int glfwKey);
  ActionTrie(Action *action, std::string action_name, char32_t key);
  int glfwKey;
  std::string action_name;
  char32_t key;
  bool useKey = false;
  Action *action;
};
struct LastKeyState {
  int action = 0, scancode = 0, mods = 0;
};
class Vim {
public:
  Vim(State *state);
  ~Vim();
  void resetMotionState();
  void reset();
  void setCursor(Cursor *cursor);
  VimMode getMode();
  bool processCharacter(char32_t c);
  std::string getModeName();
  void setMode(VimMode mode);
  bool processKey(int key, int scancode, int action, int mods);
  void addRef(Action *action);
  void registerTrie(Action *action, std::string action_name, int glfwKey);
  void iterate(std::function<void()> func);
  void registerTrieChar(Action *action, std::string action_name, char32_t t);

  void remapTrie(int glfwKey, std::string action);
  void remapCharTrie(char32_t key, std::string action);
  State &getState();
  ActionTrie *activeAction();
  bool isCommandBufferActive();
  void setIsCommandBufferActive(bool v);
  std::string &getLast();
  void setNext(std::string n);
  Utf8String &cmdBuffer();
  void setSpecialCase(bool v);
  bool shouldRenderCoords();
  size_t getCount();
  LastKeyState &getKeyState();
  void setInterceptor(Interceptor *v);
  Interceptor *getInterceptor();

private:
  bool exec(ActionTrie *trie);

  Utf8String commandBuffer;
  VimMode mode = VimMode::NORMAL;
  Cursor *cursor = nullptr;
  ActionTrie *activeTrie = nullptr;
  Interceptor *interceptor = nullptr;
  std::unordered_map<char32_t, ActionTrie *> charTries;
  std::unordered_map<int, ActionTrie *> keyTries;
  std::unordered_map<std::string, ActionTrie *> tries;
  bool commandBufferActive = false;
  bool specialCase = false;
  std::vector<Action *> refs;
  std::string last;
  std::string next;
  MotionState state;
  LastKeyState lastKeyState;
  State *gState;
};

#endif