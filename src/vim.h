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
  ActionTrie(Action *action, std::string action_name, int glfwKey) {
    this->action = action;
    this->action_name = action_name;
    this->glfwKey = key;
  }
  ActionTrie(Action *action, std::string action_name, char32_t key) {
    this->action = action;
    this->action_name = action_name;
    this->key = key;
    useKey = false;
  }

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
  Vim(State *state) : gState(state) {}
  ~Vim() {
    for (auto entry : tries) {
      delete entry.second->action;
      delete entry.second;
    }
    for (auto *ref : refs) {
      delete ref;
    }
  }
  void resetMotionState() {
    state.count = 0;
    state.isInital = true;
    state.direction = Direction::UNSET;
    state.action = "";
    state.replaceMode = ReplaceMode::UNSET;
  }
  void reset() {
    resetMotionState();
    setMode(VimMode::NORMAL);
    activeTrie = nullptr;
    next = "";
    last = "";
  }
  void setCursor(Cursor *cursor) {
    reset();
    if (this->cursor && commandBufferActive) {
      setIsCommandBufferActive(false);
      this->cursor->unbind();
    }
    this->cursor = cursor;
  }
  VimMode getMode() { return mode; }
  bool processCharacter(char32_t c) {
    if (!cursor)
      return true;
    if (mode == VimMode::INSERT || cursor->bind) {
      cursor->append(c);
      return true;
    }
    if (interceptor) {
      auto res = interceptor->intercept(c, this, cursor);
      return res.allowCoords;
    }
    if (c >= '0' && c <= '9') {
      if (c != '0' || state.count > 0) {
        state.count = state.count * 10 + ((int)(c - 48));
        return true;
      }
    }
    if (charTries.count(c)) {
      auto *trie = charTries[c];
      return exec(trie);
    }
    return true;
  }
  std::string getModeName() {
    if (mode == VimMode::INSERT)
      return "-- INSERT --";
    if (mode == VimMode::VISUAL)
      return "-- VISUAL --";
    return "-- NORMAL --";
  }
  void setMode(VimMode mode) { this->mode = mode; }
  bool processKey(int key, int scancode, int action, int mods) {
    lastKeyState.action = action;
    lastKeyState.mods = mods;
    lastKeyState.scancode = scancode;
    bool isPress = action == GLFW_PRESS || action == GLFW_REPEAT;
    if (isPress && keyTries.count(key)) {
      auto *trie = keyTries[key];
      return exec(trie);
    }
    return false;
  }
  void addRef(Action *action) { refs.push_back(action); }
  void registerTrie(Action *action, std::string action_name, int glfwKey) {
    ActionTrie *trie = new ActionTrie(action, action_name, glfwKey);
    keyTries[glfwKey] = trie;
    tries[action_name] = trie;
  }
  void iterate(std::function<void()> func) {
    size_t count = state.count == 0 ? 1 : state.count;
    for (size_t i = 0; i < count; i++)
      func();
  }
  void registerTrieChar(Action *action, std::string action_name, char32_t t) {
    ActionTrie *trie = new ActionTrie(action, action_name, t);
    charTries[t] = trie;
    tries[action_name] = trie;
  }

  void remapTrie(int glfwKey, std::string action) {
    if (!tries.count(action))
      return;
    keyTries[glfwKey] = tries[action];
  }
  void remapCharTrie(char32_t key, std::string action) {
    if (!tries.count(action))
      return;
    charTries[key] = tries[action];
  }
  State &getState() { return *gState; }
  ActionTrie *activeAction() { return activeTrie; }
  bool isCommandBufferActive() { return commandBufferActive; }
  void setIsCommandBufferActive(bool v) { commandBufferActive = v; }
  std::string &getLast() { return last; }
  void setNext(std::string n) { next = n; }
  Utf8String &cmdBuffer() { return commandBuffer; }
  void setSpecialCase(bool v) { specialCase = v; }
  bool shouldRenderCoords() {
    return !specialCase && (!cursor || cursor->bind == nullptr);
  }
  size_t getCount() { return state.count; }
  LastKeyState &getKeyState() { return lastKeyState; }
  void setInterceptor(Interceptor *v) { interceptor = v; }
  Interceptor *getInterceptor() { return interceptor; }

private:
  bool exec(ActionTrie *trie) {
    if (activeTrie && trie != activeTrie)
      state.isInital = false;
    auto result = trie->action->peek(mode, state, cursor, this);
    last = trie->action_name;
    if (result.type == ResultType::ExecuteSet) {
      if (activeTrie) {
        result = activeTrie->action->execute(mode, state, cursor, this);
      }
    }
    if (result.type == ResultType::Emit) {
      if (tries.count(result.action_name))
        return exec(tries[result.action_name]);
      return result.allowCoords;
    }
    if (result.type == ResultType::SetSelf) {
      activeTrie = trie;
      return result.allowCoords;
    }
    if (result.type == ResultType::EmitAndSetFuture) {
      next = result.emit_next;
      if (tries.count(result.action_name))
        return exec(tries[result.action_name]);
      return result.allowCoords;
    }
    if (result.type == ResultType::Done) {
      activeTrie = nullptr;
      resetMotionState();
      if (next.length() && tries.count(next)) {
        std::string v = next;
        next = "";
        return exec(tries[v]);
      }
    }
    if (result.type == ResultType::Cancel) {
      activeTrie = nullptr;
      next = "";
    }
    return result.allowCoords;
  }

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