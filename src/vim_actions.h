#ifndef LEDIT_VIM_ACTIONS_H
#define LEDIT_VIM_ACTIONS_H

#include "GLFW/glfw3.h"
#include "cursor.h"
#include "state.h"
#include "utf8String.h"
#include "vim.h"

ActionResult withType(ResultType type) {
  ActionResult r;
  r.type = type;
  return r;
}
class EscapeAction : public Action {
public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {
    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (vim->getState().mode != 0) {
      bool shift_pressed =
          glfwGetKey(vim->getState().window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
      vim->getState().inform(false, shift_pressed);
      return {};
    }
    vim->setSpecialCase(false);
    if (vim->isCommandBufferActive()) {
      cursor->unbind();
      vim->setIsCommandBufferActive(false);
    }
    if (mode == VimMode::VISUAL) {
      cursor->selection.stop();
      vim->setMode(VimMode::NORMAL);
    }
    if (mode == VimMode::INSERT)
      vim->setMode(VimMode::NORMAL);
    return {};
  }
};
class BackspaceAction : public Action {
public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {
    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (mode == VimMode::INSERT || cursor->bind) {
      cursor->removeOne();
    }
    return {};
  }
};
class EnterAction : public Action {
public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {
    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (vim->getState().mode != 0) {
      bool shift_pressed =
          glfwGetKey(vim->getState().window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
      vim->getState().inform(true, shift_pressed);
      auto out = withType(ResultType::Silent);
      out.allowCoords = false;
      return out;
    }
    if (mode == VimMode::INSERT) {
      cursor->append('\n');
    }
    if (vim->isCommandBufferActive()) {
      Utf8String content = vim->getState().miniBuf;
      cursor->unbind();
      vim->setIsCommandBufferActive(false);
      commandParser(content, vim, cursor);
      auto out = withType(ResultType::Silent);
      out.allowCoords = false;
      return out;
    }

    return {};
  }
  void commandParser(Utf8String &buffer, Vim *vim, Cursor *c) {
    std::string content = buffer.getStr();
    State &state = vim->getState();
    if (content == "/") {
      state.search();
      return;
    }
    if (content == ":b" || content == ":b ") {
      state.switchBuffer();
      return;
    }
    if (content == ":c" || content == ":c ") {
      state.command();
      return;
    }
    if (content == ":mode") {
      state.switchMode();
      return;
    }
    if (content == ":e" || content == ":e ") {
      state.open();
      return;
    }
    if (content == ":n" || content == ":new") {
      state.addCursor("");
      return;
    }
    if (content == ":bd") {
      state.deleteActive();
      return;
    }
    if (content.find(":w") == 0) {
      vim->getState().save();
      if (vim->getState().mode != 0 && content.find("!") == std::string::npos)
        return;
    }

    if (content.find("q") != std::string::npos) {
      bool forced = content.find("!") != std::string::npos;
      if (content.find("a") != std::string::npos) {
        if (forced) {
          state.exitFlag = true;
          glfwSetWindowShouldClose(state.window, true);
        } else {
          CursorEntry *edited = state.hasEditedBuffer();
          if (edited) {
            state.status =
                create(edited->path.length() ? edited->path : "New File") +
                U" edited, press ESC again to exit";
            vim->setSpecialCase(true);
          } else {
            state.exitFlag = true;
            glfwSetWindowShouldClose(state.window, true);
          }
        }
      } else {
        if (c->edited) {
          state.status = create(state.path.length() ? state.path : "New File") +
                         U" edited, press ESC again to exit";
          vim->setSpecialCase(true);
        } else {
          if (state.cursors.size() == 1) {
            state.exitFlag = true;
            glfwSetWindowShouldClose(state.window, true);
          } else {
            state.deleteActive();
          }
        }
      }
    }
  }
};
class TabAction : public Action {
public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {
    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {

    if (vim->getState().mode != 0) {
      bool shift_pressed =
          glfwGetKey(vim->getState().window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
      vim->getState().provideComplete(shift_pressed);
      auto out = withType(ResultType::Silent);
      out.allowCoords = false;
      return out;
    }
    if (mode == VimMode::INSERT) {
      State &st = vim->getState();
      bool useSpaces = st.provider.useSpaces;
      auto am = st.provider.tabWidth;
      if (useSpaces)
        cursor->append(std::string(am, ' '));
      else
        cursor->append('\t');
    }
    return {};
  }
};
class IAction : public Action {
public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {
    return {

    };
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (vim->activeAction()) {
      if (state.replaceMode == ReplaceMode::UNSET)
        state.replaceMode = ReplaceMode::INNER;
      return withType(ResultType::Silent);
    }
    vim->setMode(VimMode::INSERT);
    return {};
  }
};

class HAction : public Action {
public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {
    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (vim->activeAction()) {
      state.direction = Direction::LEFT;
      ActionResult r;
      r.type = ResultType::ExecuteSet;
      return r;
    }
    if (mode == VimMode::NORMAL || mode == VimMode::VISUAL) {
      vim->iterate([cursor]() { cursor->moveLeft(); });
    }
    return {};
  }
};

class JAction : public Action {
public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {
    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (vim->activeAction()) {
      state.direction = Direction::DOWN;
      ActionResult r;
      r.type = ResultType::ExecuteSet;
      return r;
    }
    if (mode == VimMode::NORMAL || mode == VimMode::VISUAL) {
      vim->iterate([cursor]() { cursor->moveDown(); });
    }
    return {};
  }
};
class KAction : public Action {
public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {
    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (vim->activeAction()) {
      state.direction = Direction::UP;
      ActionResult r;
      r.type = ResultType::ExecuteSet;
      return r;
    }
    if (mode == VimMode::NORMAL || mode == VimMode::VISUAL) {
      vim->iterate([cursor]() { cursor->moveUp(); });
    }
    return {};
  }
};
class LAction : public Action {
public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {
    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (vim->activeAction()) {
      state.direction = Direction::RIGHT;
      ActionResult r;
      r.type = ResultType::ExecuteSet;
      return r;
    }
    if (mode == VimMode::NORMAL || mode == VimMode::VISUAL) {
      vim->iterate([cursor]() { cursor->moveRight(); });
    }
    return {};
  }
};
class AAction : public Action {
public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {
    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (vim->activeAction()) {
      if (state.replaceMode == ReplaceMode::UNSET)
        state.replaceMode = ReplaceMode::ALL;
      return withType(ResultType::Silent);
    }
    if (mode == VimMode::NORMAL) {
      if (cursor->x < cursor->lines[cursor->y].size())
        cursor->moveRight();
      ActionResult r;
      r.type = ResultType::Emit;
      r.action_name = "i";
      return r;
    }
    return {};
  }
};

class AAAction : public Action {
public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {
    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    // if (vim->activeAction()) {
    //   state.direction = Direction::RIGHT;
    //   ActionResult r;
    //   r.type = ResultType::ExecuteSet;
    //   return r;
    // }
    if (mode == VimMode::NORMAL) {
      cursor->jumpEnd();
      ActionResult r;
      r.type = ResultType::Emit;
      r.action_name = "i";
      return r;
    }
    return {};
  }
};
class WAction : public Action {
public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {
    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (vim->activeAction()) {
      state.action = "w";
      return withType(ResultType::ExecuteSet);
    }
    if (mode == VimMode::NORMAL || mode == VimMode::VISUAL) {
      vim->iterate([cursor]() { cursor->advanceWord(); });
    }
    return {};
  }
};
class BAction : public Action {
public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {
    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (vim->activeAction()) {
      state.action = "b";
      return withType(ResultType::ExecuteSet);
    }
    if (mode == VimMode::NORMAL || mode == VimMode::VISUAL) {
      vim->iterate([cursor]() { cursor->advanceWordBackwards(); });
    }
    return {};
  }
};
class OAction : public Action {
public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {
    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    // if (vim->activeAction()) {
    //   state.direction = Direction::RIGHT;
    //   ActionResult r;
    //   r.type = ResultType::ExecuteSet;
    //   return r;
    // }
    if (mode == VimMode::NORMAL) {
      cursor->jumpEnd();
      cursor->append('\n');
      ActionResult r;
      r.type = ResultType::Emit;
      r.action_name = "i";
      return r;
    }
    return {};
  }
};
class OOAction : public Action {
public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {
    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    // if (vim->activeAction()) {
    //   state.direction = Direction::RIGHT;
    //   ActionResult r;
    //   r.type = ResultType::ExecuteSet;
    //   return r;
    // }
    if (mode == VimMode::NORMAL) {
      cursor->jumpStart();
      cursor->append('\n');
      cursor->moveUp();
      ActionResult r;
      r.type = ResultType::Emit;
      r.action_name = "i";
      return r;
    }
    return {};
  }
};
class DAction : public Action {
public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    if (state.isInital) {
      cursor->deleteLines(cursor->y, 1);
    } else {
      if (state.action == "w") {
        if (state.replaceMode == ReplaceMode::ALL) {
          vim->iterate([cursor]() {
            cursor->deleteWord();
            cursor->removeOne();
          });
        } else {
          vim->iterate([cursor]() { cursor->deleteWord(); });
        }
      } else if (state.action == "b") {
        if (state.replaceMode == ReplaceMode::ALL) {
          vim->iterate([cursor]() {
            cursor->deleteWordBackwards();
            cursor->removeOne();
          });
        } else {
          vim->iterate([cursor]() { cursor->deleteWordBackwards(); });
        }
      } else if (state.direction == Direction::UP) {
        cursor->deleteLines(cursor->y - 1 - state.count, 1 + state.count);
      } else if (state.direction == Direction::DOWN) {
        cursor->deleteLines(cursor->y, 1 + state.count);
      } else if (state.direction == Direction::LEFT) {
        vim->iterate([cursor, vim]() { cursor->removeOne(); });
      } else if (state.direction == Direction::RIGHT) {
        vim->iterate([cursor]() { cursor->removeBeforeCursor(); });
      }
    }
    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (mode == VimMode::VISUAL) {
      cursor->deleteSelection();
      ActionResult r;
      r.type = ResultType::Emit;
      r.action_name = "ESC";
    }
    if (vim->activeAction() == nullptr && mode == VimMode::NORMAL)
      return withType(ResultType::SetSelf);
    if (vim->activeAction() && vim->activeAction()->action_name == "d")
      return execute(mode, state, cursor, vim);
    return {};
  }
};

class UAction : public Action {
public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {

    if (!vim->activeAction() && mode == VimMode::NORMAL)
      cursor->undo();
    return {};
  }
};
class VAction : public Action {
public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {

    if (!vim->activeAction() && mode == VimMode::NORMAL) {
      vim->setMode(VimMode::VISUAL);
      cursor->selection.activate(cursor->x, cursor->y);
    }
    return {};
  }
};
class CAction : public Action {
public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {
    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (vim->activeAction() && vim->activeAction()->action_name == "d") {
      cursor->deleteLines(cursor->y, 1);
      cursor->append('\n');
      vim->setNext("i");
      return {};
    }
    if (!vim->activeAction()) {
      ActionResult r;
      r.type = ResultType::EmitAndSetFuture;
      r.action_name = "d";
      r.emit_next = "i";
      return r;
    }
    return {};
  }
};

class DollarAction : public Action {
public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {

    if (!vim->activeAction()) {
      cursor->jumpEnd();
    }
    return withType(ResultType::Silent);
  }
};

class ZeroAction : public Action {
public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {

    if (!vim->activeAction()) {
      cursor->jumpStart();
    }
    return withType(ResultType::Silent);
  }
};
class ColonAction : public Action {
public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {

    if (!vim->activeAction() && mode == VimMode::NORMAL) {
      State &st = vim->getState();

      st.miniBuf = U":";
      vim->getState().status = U"";
      cursor->bindTo(&st.miniBuf);
      vim->setIsCommandBufferActive(true);
      auto out = withType(ResultType::Silent);
      out.allowCoords = false;
      return out;
    }
    return {};
  }
};
class YAction : public Action {
public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {

    if (!vim->activeAction() && mode == VimMode::VISUAL) {
      vim->getState().tryCopy();
      ActionResult r;
      r.type = ResultType::Emit;
      r.allowCoords = false;
      r.action_name = "ESC";
      return r;
    }
    return {};
  }
};
class PAction : public Action {
public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {

    if (!vim->activeAction() && mode == VimMode::NORMAL) {
      vim->getState().tryPaste();
    }
    return {};
  }
};
class PercentAction : public Action {
public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {

    if (!vim->activeAction()) {
      cursor->jumpMatching();
    }
    return {};
  }
};
class IIAction : public Action {
public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (vim->activeAction())
      return withType(ResultType::Silent);
    Utf8String &l = cursor->lines[cursor->y];
    for (size_t i = 0; i < l.length(); i++) {
      char32_t current = l[i];
      if (current > ' ') {
        cursor->x = i;
        break;
      }
    }
    ActionResult r;
    r.type = ResultType::Emit;
    r.action_name = "i";
    return r;
  }
};
class GGAction : public Action {
public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (vim->activeAction()) {
      return withType(ResultType::Silent);
    }
    if (state.count) {
      cursor->gotoLine(state.count);
      return {};
    }

    cursor->gotoLine(cursor->lines.size());
    return {};
  }
};
class GAction : public Action {
public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (vim->activeAction()) {
      if (vim->activeAction()->action_name == "g") {
        cursor->gotoLine(1);
        return {};
      }
      return withType(ResultType::Silent);
    }
    return withType(ResultType::SetSelf);
  }
};

class ParagraphUpAction : public Action {
public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (vim->activeAction()) {
      return withType(ResultType::Silent);
    }
    if (cursor->y == 0)
      return withType(ResultType::Silent);
    for (int64_t i = cursor->y - 1; i >= 0; i--) {
      bool allws = true;
      auto ref = cursor->lines[i].getStrRef();
      for (const char t : ref) {
        if (t > ' ') {
          allws = false;
          break;
        }
      }
      if (allws) {
        cursor->gotoLine(i + 1);
        return {};
      }
    }
    cursor->gotoLine(1);
    return {};
  }
};
class ParagraphDownAction : public Action {
public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (vim->activeAction()) {
      return withType(ResultType::Silent);
    }
    for (int64_t i = cursor->y + 1; i < cursor->lines.size(); i++) {
      bool allws = true;
      auto ref = cursor->lines[i].getStrRef();
      for (const char t : ref) {
        if (t > ' ') {
          allws = false;
          break;
        }
      }
      if (allws) {
        cursor->gotoLine(i + 1);
        return {};
      }
    }
    cursor->gotoLine(cursor->lines.size());
    return {};
  }
};
void register_vim_commands(Vim &vim, State &state) {
  vim.registerTrie(new EscapeAction(), "ESC", GLFW_KEY_ESCAPE);
  vim.registerTrie(new BackspaceAction(), "BACKSPACE", GLFW_KEY_BACKSPACE);
  vim.registerTrie(new EnterAction(), "ENTER", GLFW_KEY_ENTER);
  vim.registerTrie(new TabAction(), "TAB", GLFW_KEY_TAB);
  vim.registerTrieChar(new IAction(), "i", 'i');
  vim.registerTrieChar(new HAction(), "h", 's');
  vim.registerTrieChar(new JAction(), "j", 'd');
  vim.registerTrieChar(new KAction(), "k", 'f');
  vim.registerTrieChar(new LAction(), "l", 'g');
  vim.registerTrieChar(new AAction(), "a", 'a');
  vim.registerTrieChar(new AAAction(), "A", 'A');
  vim.registerTrieChar(new WAction(), "w", 'w');
  vim.registerTrieChar(new BAction(), "b", 'b');
  vim.registerTrieChar(new BAction(), "b", 'b');
  vim.registerTrieChar(new OAction(), "o", 'o');
  vim.registerTrieChar(new OOAction(), "O", 'O');
  vim.registerTrieChar(new DAction(), "d", 'h');
  vim.registerTrieChar(new UAction(), "u", 'u');
  vim.registerTrieChar(new VAction(), "v", 'v');
  vim.registerTrieChar(new CAction(), "c", 'c');
  vim.registerTrieChar(new DollarAction(), "$", '$');
  vim.registerTrieChar(new ZeroAction(), "0", '0');
  vim.registerTrieChar(new ColonAction(), ":", ':');
  vim.registerTrieChar(new YAction(), "y", 'y');
  vim.registerTrieChar(new PAction(), "p", 'p');
  vim.registerTrieChar(new PercentAction(), "%", '%');
  vim.registerTrieChar(new IIAction(), "I", 'I');
  vim.registerTrieChar(new GAction(), "g", 'j');
  vim.registerTrieChar(new GGAction(), "G", 'J');
  vim.registerTrieChar(new ParagraphUpAction(), "{", '{');
  vim.registerTrieChar(new ParagraphDownAction(), "}", '}');
}

#endif