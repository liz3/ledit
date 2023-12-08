#ifndef LEDIT_VIM_ACTIONS_H
#define LEDIT_VIM_ACTIONS_H

#include "GLFW/glfw3.h"
#include "cursor.h"
#include "state.h"
#include "utf8String.h"
#include "utils.h"
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
      ActionResult r;
      r.allowCoords = false;
      return r;
    }
    vim->setSpecialCase(false);
    if (vim->isCommandBufferActive()) {
      cursor->unbind();
      vim->setIsCommandBufferActive(false);
    }
    vim->setInterceptor(nullptr);
    cursor->selection.stop();
    if(vim->activeAction()) {
      auto o = withType(ResultType::Cancel);
      o.allowCoords = false;
      return o;
    }
    if (mode == VimMode::VISUAL) {

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
      bool alt_pressed =
          glfwGetKey(vim->getState().window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS;
      if (alt_pressed)
        cursor->deleteWordBackwards();
      else
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
    if(content == ":ck") {
      state.killCommand();
      return;
    }
    if (content == ":co") {
      state.activateLastCommandBuffer();
      return;
    }
    if(content == ":lw"){
      state.toggleLineWrapping();
      return;
    }
    if (content == ":font") {
      state.changeFont();
      return;
    }
    if(content == ":config"){
      state.addCursor(state.provider.getConfigPath());
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
    } else if (mode == VimMode::NORMAL) {
      auto window = vim->getState().window;
      auto mods = vim->getKeyState().mods;
      bool ctrl_pressed =
          glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
          glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ||
          mods & GLFW_MOD_CONTROL;
      if (ctrl_pressed)
        vim->getState().fastSwitch();
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
      vim->iterate([cursor]() { 
        if(cursor->x > 0)
        cursor->moveLeft();
         });
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
      vim->iterate([cursor]() { 
        if(cursor->x < cursor->getCurrentLineLength())
        cursor->moveRight(); 
      });
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
      if (cursor->x < cursor->getCurrentLineLength())
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
    if (vim->activeAction())
      return withType(ResultType::Silent);
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
    if (vim->activeAction())
      return withType(ResultType::Silent);
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
      auto out = cursor->deleteLines(cursor->y, 1);
      vim->getState().tryCopyInput(out);
    } else {
      if (state.action.length() == 1) {
        for (auto &pair : PAIRS) {
          if (state.action[0] == pair.first || state.action[0] == pair.second) {
            Utf8String in(state.action);
            bool isClosing = state.action[0] == pair.second;
            auto result =
                cursor->findGlobal(!isClosing, in, cursor->x, cursor->y);
            if (result.first == -1 && result.second == -1) {
              return {};
            }
            cursor->x = result.first;

            cursor->y = result.second;

            if (state.replaceMode == ReplaceMode::ALL && !isClosing)
              cursor->selection.activate(cursor->x == 0 ? 0 : cursor->x - 1,
                                         cursor->y);
            else
              cursor->selection.activate(state.replaceMode == ReplaceMode::ALL
                                             ? cursor->x + 1
                                             : cursor->x,
                                         cursor->y);

            cursor->jumpMatching();
            if (state.replaceMode == ReplaceMode::ALL) {
              cursor->x++;
              cursor->selection.diffX(cursor->x);
            }
            if (state.replaceMode == ReplaceMode::INNER &&
                (cursor->selection.xEnd == cursor->selection.xStart + 1 ||
                 cursor->selection.xEnd == cursor->selection.xStart - 1) &&
                cursor->selection.yStart == cursor->selection.yEnd) {
              if (cursor->selection.xEnd == cursor->selection.xStart - 1)
                cursor->x++;
              cursor->selection.stop();
              return {};
            } else if (state.replaceMode == ReplaceMode::INNER) {
              cursor->selection.xStart++;
            }
            if (state.replaceMode == ReplaceMode::INNER && isClosing) {
              cursor->x++;
              cursor->selection.diffX(cursor->x);
            } else if (state.replaceMode == ReplaceMode::ALL && isClosing) {
              cursor->x--;
              cursor->selection.diffX(cursor->x);
            }
            Utf8String cc(cursor->getSelection());
            vim->getState().tryCopyInput(cc);
            cursor->deleteSelection();
            cursor->selection.stop();
            cursor->center(cursor->y);
            return {};
          }
        }
        if (state.action == "\"") {
          auto result =
              cursor->findGlobal(true, Utf8String("\""), cursor->x, cursor->y);
          auto resultRight = cursor->findGlobal(false, Utf8String("\""),
                                                cursor->x + 1, cursor->y);
          if (result.first == -1 || result.second == -1 ||
              resultRight.first == -1 || resultRight.second == -1)
            return {};
          cursor->x = result.first + 1;

          cursor->y = result.second;

          if (state.replaceMode == ReplaceMode::ALL)
            cursor->selection.activate(cursor->x == 0 ? 0 : cursor->x - 1,
                                       cursor->y);
          else
            cursor->selection.activate(cursor->x, cursor->y);

          cursor->selection.diff(state.replaceMode == ReplaceMode::ALL
                                     ? resultRight.first + 1
                                     : resultRight.first,
                                 resultRight.second);
          Utf8String cc(cursor->getSelection());
          vim->getState().tryCopyInput(cc);
          cursor->deleteSelection();
          cursor->selection.stop();
          return {};
        }
      }
      if (state.action == "w") {
        if (state.replaceMode == ReplaceMode::ALL) {
          vim->iterate([cursor]() { cursor->deleteWordVim(true); });
        } else {
          vim->iterate([cursor]() { cursor->deleteWordVim(false); });
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
      } else if (state.action == "$") {
        Utf8String &str = cursor->lines[cursor->y];
        auto length = str.size() - cursor->x;
        Utf8String w = str.substr(cursor->x, length);
        str.erase(cursor->x, length);
        cursor->historyPush(3, w.length(), w);
        vim->getState().tryCopyInput(w);
      } else if (state.direction == Direction::UP) {
        auto out =
            cursor->deleteLines(cursor->y - 1 - state.count, 1 + state.count);
        vim->getState().tryCopyInput(out);
      } else if (state.direction == Direction::DOWN) {
        auto out = cursor->deleteLines(cursor->y, 1 + state.count);
        vim->getState().tryCopyInput(out);
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
      cursor->selection.stop();
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
      if (cursor->x == cursor->getCurrentLineLength() - 1)
        cursor->x++;
      cursor->selection.activate(cursor->x, cursor->y);
    } else if (mode == VimMode::VISUAL) {
      auto a = withType(ResultType::Emit);
      a.action_name = "ESC";
      return a;
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
    } else {
      state.action = "$";
      return withType(ResultType::ExecuteSet);
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

    if(state.isInital){
      auto l = cursor->lines[cursor->y];
      l+= U"\n";
      vim->getState().tryCopyInput(l);
    }
    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {

    if (!vim->activeAction() && mode == VimMode::VISUAL) {
      vim->getState().tryCopy();
      cursor->selection.stop();
      vim->setMode(VimMode::NORMAL);
      ActionResult r;
      r.allowCoords = false;
      return r;
    }
    if(!vim->activeAction())
      return withType(ResultType::SetSelf);

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

    if (!vim->activeAction() && mode == VimMode::NORMAL ||
        mode == VimMode::VISUAL) {
      if (mode == VimMode::VISUAL) {
        if (cursor->selection.active) {
          cursor->deleteSelection();
        }
        vim->setMode(VimMode::NORMAL);
      }
      vim->getState().tryPaste();
      ActionResult r;
      r.allowCoords = false;
      return r;
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

class ParagraphAction : public Action {
private:
  std::string symbol;

public:
  ParagraphAction(std::string symbol) { this->symbol = symbol; }
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (vim->activeAction()) {
      state.action = symbol;
      return withType(ResultType::ExecuteSet);
    }
    if (symbol == "{") {
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
    } else {
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
    return {};
  }
};
class BracketAction : public Action {
private:
  std::string symbol;

public:
  BracketAction(std::string symbol) { this->symbol = symbol; }
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (vim->activeAction()) {
      state.action = symbol;
      return withType(ResultType::ExecuteSet);
    }
    return {};
  }
};
class ParenAction : public Action {
private:
  std::string symbol;

public:
  ParenAction(std::string symbol) { this->symbol = symbol; }
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (vim->activeAction()) {
      state.action = symbol;
      return withType(ResultType::ExecuteSet);
    }
    return {};
  }
};
class QuoteAction : public Action {
private:
  std::string symbol;

public:
  QuoteAction(std::string symbol) { this->symbol = symbol; }
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (vim->activeAction()) {
      state.action = symbol;
      return withType(ResultType::ExecuteSet);
    }
    return {};
  }
};

class SlashAction : public Action {
public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (vim->activeAction()) {
      return withType(ResultType::ExecuteSet);
    } else if (mode != VimMode::INSERT) {
      vim->getState().search();
      ActionResult r;
      r.allowCoords = false;
      return r;
    }
    return {};
  }
};

class FontSizeAction : public Action {
private:
  bool increase = false;

public:
  FontSizeAction(bool v) : increase(v) {}
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (vim->activeAction()) {
      return withType(ResultType::Silent);
    }
    auto window = vim->getState().window;
    auto mods = vim->getKeyState().mods;
    bool ctrl_pressed =
        glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ||
        mods & GLFW_MOD_CONTROL;
    if (ctrl_pressed) {
      if (increase) {
        vim->getState().increaseFontSize(0.05);
      } else {
        vim->getState().increaseFontSize(-0.05);
      }
    }
    return {};
  }
};
class MoveAction : public Action {
private:
  Direction direction;

public:
  MoveAction(Direction v) : direction(v) {}
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (vim->activeAction() || mode != VimMode::INSERT) {
      return withType(ResultType::Silent);
    }

    auto window = vim->getState().window;
    auto mods = vim->getKeyState().mods;
    bool ctrl_pressed =
        glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ||
        mods & GLFW_MOD_CONTROL;
    if (ctrl_pressed) {
      if (direction == Direction::UP)
        cursor->moveUp();
      else if (direction == Direction::RIGHT)
        cursor->moveRight();
      else if (direction == Direction::LEFT)
        cursor->moveLeft();
      else if (direction == Direction::DOWN)
        cursor->moveDown();
    }
    return {};
  }
};

class RAction : public Action {

public:
  class XInterceptor : public Interceptor {
    ActionResult intercept(char32_t in, Vim *vim, Cursor *cursor) override {
      cursor->setCurrent(in);
      vim->setInterceptor(nullptr);
      return {};
    }
  };
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (vim->getInterceptor() != &interceptor && !vim->activeAction() &&
        mode == VimMode::NORMAL)
      vim->setInterceptor(&interceptor);
    return withType(ResultType::Silent);
  }

private:
  XInterceptor interceptor;
};

class Finder {
private:
  bool active = false, backwards = false, before = false;
  int foundIndex = -1;
  char32_t cc;
  Cursor *cursor = nullptr;
  Vim *vim = nullptr;

public:
  void find(char32_t w, bool backwards, bool before, Cursor *cursor, Vim *vim) {
    this->active = true;
    this->cc = w;
    this->backwards = backwards;
    this->before = before;
    this->cursor = cursor;
    this->vim = vim;
    foundIndex = -1;
    next();
  }
  void next(bool prev = false) {
    if (!vim || !cursor || cursor != vim->getState().cursor)
      return;
    auto x = cursor->x;
    auto y = cursor->y;
    if ((!backwards && prev) || (backwards && !prev)) {
      if (x == 0)
        return;
      for (int64_t i = x - 1; i >= 0; i--) {
        if (foundIndex != -1 && before && i == foundIndex)
          continue;
        char32_t current = cursor->lines[y][i];
        if (current == cc) {
          foundIndex = i;
          if (before)
            i++;
          cursor->x = i;
          cursor->selection.diffX(i);
          break;
        }
      }
    } else {
      x++;
      for (int64_t i = x + 1; i < cursor->lines[y].size(); i++) {

        if (foundIndex != -1 && before && i == foundIndex)
          continue;
        char32_t current = cursor->lines[y][i];
        if (current == cc) {
          foundIndex = i;
          if (before)
            i--;
          cursor->x = i;
          cursor->selection.diffX(i);
          break;
        }
      }
    }
  }
};

class FindAction : public Action {

public:
  class FindInterceptor : public Interceptor {
  private:
    FindAction *action = nullptr;

  public:
    FindInterceptor(FindAction *action) { this->action = action; }
    ActionResult intercept(char32_t in, Vim *vim, Cursor *cursor) override {
      action->finder->find(in, action->backwards, action->offset, cursor, vim);
      vim->setInterceptor(nullptr);
      return {};
    }
  };
  FindAction(Finder *finder_, bool backwards_, bool offset_)
      : finder(finder_), backwards(backwards_), offset(offset_) {
    interceptor = new FindInterceptor(this);
  }
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (vim->getInterceptor() != interceptor && !vim->activeAction())
      vim->setInterceptor(interceptor);
    return withType(ResultType::Silent);
  }

private:
  Finder *finder;
  bool backwards;
  bool offset;
  FindInterceptor *interceptor;
};

class SemicolonAction : public Action {

public:
  SemicolonAction(Finder *finder_) : finder(finder_) {}
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (!vim->activeAction()) {
      finder->next(false);
    }
    return {

    };
  }

private:
  Finder *finder;
};
class CommaAction : public Action {

public:
  CommaAction(Finder *finder_) : finder(finder_) {}
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (!vim->activeAction()) {
      finder->next(true);
    }
    return {};
  }

private:
  Finder *finder;
};

void register_vim_commands(Vim &vim, State &state) {
  Finder *finder = new Finder();
  vim.registerTrie(new EscapeAction(), "ESC", GLFW_KEY_ESCAPE);
  vim.registerTrie(new BackspaceAction(), "BACKSPACE", GLFW_KEY_BACKSPACE);
  vim.registerTrie(new EnterAction(), "ENTER", GLFW_KEY_ENTER);
  vim.registerTrie(new TabAction(), "TAB", GLFW_KEY_TAB);
  vim.registerTrie(new FontSizeAction(true), "F_INCREASE", GLFW_KEY_EQUAL);
  vim.registerTrie(new FontSizeAction(false), "F_DECREASE", GLFW_KEY_MINUS);
  vim.registerTrie(new TabAction(), "TAB", GLFW_KEY_TAB);
  vim.registerTrie(new MoveAction(Direction::UP), "M_UP", GLFW_KEY_P);
  vim.registerTrie(new MoveAction(Direction::RIGHT), "M_RIGHT", GLFW_KEY_F);
  vim.registerTrie(new MoveAction(Direction::DOWN), "M_DOWN", GLFW_KEY_N);
  vim.registerTrie(new MoveAction(Direction::LEFT), "M_LEFT", GLFW_KEY_B);
  vim.registerTrieChar(new IAction(), "i", 'i');
  vim.registerTrieChar(new HAction(), "h", 'h');
  vim.registerTrieChar(new JAction(), "j", 'j');
  vim.registerTrieChar(new KAction(), "k", 'k');
  vim.registerTrieChar(new LAction(), "l", 'l');
  vim.registerTrieChar(new AAction(), "a", 'a');
  vim.registerTrieChar(new AAAction(), "A", 'A');
  vim.registerTrieChar(new WAction(), "w", 'w');
  vim.registerTrieChar(new BAction(), "b", 'b');
  vim.registerTrieChar(new BAction(), "b", 'b');
  vim.registerTrieChar(new OAction(), "o", 'o');
  vim.registerTrieChar(new OOAction(), "O", 'O');
  vim.registerTrieChar(new DAction(), "d", 'd');
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
  vim.registerTrieChar(new GAction(), "g", 'g');
  vim.registerTrieChar(new GGAction(), "G", 'G');
  vim.registerTrieChar(new ParagraphAction("{"), "{", '{');
  vim.registerTrieChar(new ParagraphAction("}"), "}", '}');
  vim.registerTrieChar(new BracketAction("["), "[", '[');
  vim.registerTrieChar(new BracketAction("]"), "]", ']');
  vim.registerTrieChar(new ParenAction("("), "(", '(');
  vim.registerTrieChar(new ParenAction(")"), ")", ')');
  vim.registerTrieChar(new QuoteAction("\""), "\"", '\"');
  vim.registerTrieChar(new SlashAction(), "/", '/');
  vim.registerTrieChar(new RAction(), "r", 'r');
  vim.registerTrieChar(new FindAction(finder, false, false), "f", 'f');
  vim.registerTrieChar(new FindAction(finder, false, true), "t", 't');
  vim.registerTrieChar(new FindAction(finder, true, false), "F", 'F');
  vim.registerTrieChar(new FindAction(finder, true, true), "T", 'T');
  vim.registerTrieChar(new SemicolonAction(finder), ";", ';');
  vim.registerTrieChar(new CommaAction(finder), ",", ',');

  for(auto& entry: state.provider.vimRemaps)
    vim.remapCharTrie(entry.first, entry.second);
}

#endif