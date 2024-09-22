#include "vim.h"
#include "GLFW/glfw3.h"

ActionTrie::ActionTrie(Action *action, std::string action_name, int glfwKey) {
  this->action = action;
  this->action_name = action_name;
  this->glfwKey = key;
}
ActionTrie::ActionTrie(Action *action, std::string action_name, char32_t key) {
  this->action = action;
  this->action_name = action_name;
  this->key = key;
  useKey = false;
}
Vim::Vim(State *state) : gState(state) {}
Vim::~Vim() {
  for (auto entry : tries) {
    delete entry.second->action;
    delete entry.second;
  }
  for (auto *ref : refs) {
    delete ref;
  }
}
void Vim::resetMotionState() {
  state.count = 0;
  state.isInital = true;
  state.direction = Direction::UNSET;
  state.action = "";
  state.replaceMode = ReplaceMode::UNSET;
}
void Vim::reset() {
  resetMotionState();
  setMode(VimMode::NORMAL);
  activeTrie = nullptr;
  next = "";
  last = "";
}
void Vim::setCursor(Cursor *cursor) {
  reset();
  if (this->cursor && commandBufferActive) {
    setIsCommandBufferActive(false);
    this->cursor->unbind();
  }
  this->cursor = cursor;
}
VimMode Vim::getMode() { return mode; }
bool Vim::processCharacter(char32_t c) {
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
std::string Vim::getModeName() {
  if (mode == VimMode::INSERT)
    return "-- INSERT --";
  if (mode == VimMode::VISUAL)
    return "-- VISUAL --";
  return "-- NORMAL --";
}

void Vim::setMode(VimMode mode) { this->mode = mode; }
bool Vim::processKey(int key, int scancode, int action, int mods) {
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

void Vim::addRef(Action *action) { refs.push_back(action); }
void Vim::registerTrie(Action *action, std::string action_name, int glfwKey) {
  ActionTrie *trie = new ActionTrie(action, action_name, glfwKey);
  keyTries[glfwKey] = trie;
  tries[action_name] = trie;
}
void Vim::iterate(std::function<void()> func) {
  size_t count = state.count == 0 ? 1 : state.count;
  for (size_t i = 0; i < count; i++)
    func();
}
void Vim::registerTrieChar(Action *action, std::string action_name,
                           char32_t t) {
  ActionTrie *trie = new ActionTrie(action, action_name, t);
  charTries[t] = trie;
  tries[action_name] = trie;
}
void Vim::remapTrie(int glfwKey, std::string action) {
  if (!tries.count(action))
    return;
  keyTries[glfwKey] = tries[action];
}
void Vim::remapCharTrie(char32_t key, std::string action) {
  if (!tries.count(action))
    return;
  charTries[key] = tries[action];
}
State &Vim::getState() { return *gState; }
ActionTrie *Vim::activeAction() { return activeTrie; }

bool Vim::isCommandBufferActive() { return commandBufferActive; }
void Vim::setIsCommandBufferActive(bool v) { commandBufferActive = v; }

std::string &Vim::getLast() { return last; }
void Vim::setNext(std::string n) { next = n; }

Utf8String &Vim::cmdBuffer() { return commandBuffer; }

void Vim::setSpecialCase(bool v) { specialCase = v; }

bool Vim::shouldRenderCoords() {
  return !specialCase && (!cursor || cursor->bind == nullptr);
}
size_t Vim::getCount() { return state.count; }

LastKeyState &Vim::getKeyState() { return lastKeyState; }

void Vim::setInterceptor(Interceptor *v) { interceptor = v; }

Interceptor *Vim::getInterceptor() { return interceptor; }

bool Vim::exec(ActionTrie *trie) {

  auto result = trie->action->peek(mode, state, cursor, this);
  if (activeTrie && trie != activeTrie && result.type != ResultType::Silent)
    state.isInital = false;
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