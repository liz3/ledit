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
    if (vim->activeAction()) {
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
    if (vim->getState().mode != 0 ||
        (!vim->isCommandBufferActive() && cursor->isFolder)) {
      auto window = vim->getState().window;
      auto mods = vim->getKeyState().mods;

      bool ctrl_pressed =
          glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
          glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ||
          mods & GLFW_MOD_CONTROL;
      bool shift_pressed =
          glfwGetKey(vim->getState().window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

      vim->getState().inform(true, shift_pressed, ctrl_pressed);
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
    if (content == ":b") {
      state.switchBuffer();
      return;
    }
    if (content == ":c") {
      state.command();
      return;
    } else if (content.find(":c ") == 0 && content.length() > 3) {
      state.runCommand(content.substr(3));
      return;
    }
    if (content == ":ck") {
      state.killCommand();
      return;
    }
    if (content == ":co") {
      state.activateLastCommandBuffer();
      return;
    }
    if (content == ":%s" || content == ":replace") {
      state.startReplace();
      return;
    }
    if (content == ":lw") {
      state.toggleLineWrapping();
      return;
    }
    if(content.find(":tw ") == 0 && content.length() >4){
      state.provider.tabWidth = std::stoi(content.substr(4));
      state.status = U"Tab Width: " + Utf8String(content.substr(4));
      return;
    }
    if (content == ":hl") {
      state.switchLineHighlightMode();
      return;
    }
    if (content == ":rconfig" || content == ":rc") {
      state.provider.reloadConfig();
      return;
    }
    if (content == ":ln") {
      state.showLineNumbers = !state.showLineNumbers;
      return;
    }
    if (content == ":font") {
      state.changeFont();
      return;
    }
    if (content == ":config") {
      state.addCursor(state.provider.getConfigPath());
      return;
    }
    if (content == ":mode") {
      state.switchMode();
      return;
    } else if (content.find(":mode ") == 0 && content.length() > 6) {
      state.directlyEnableLanguage(content.substr(6));
      return;
    }
    if (content == ":e") {
      state.open();
      return;
    } else if (content.find(":e ") == 0 && content.length() > 3) {
      state.addCursor(content.substr(3));
      return;
    }
    if (content == ":win") {
      state.open(true);
      return;
    } else if (content.find(":win ") == 0 && content.length() > 3) {
      add_window(content.substr(5));
      return;
    }
    if (content == ":sh") {
      state.shellCommand();
      return;
    }
    if (content.find(":sh ") == 0) {
      if (state.execCommand(content.substr(4))) {
        state.lastCmd = content.substr(4);
      }
      return;
    }

    if (content == ":theme") {
      state.setTheme();
      return;
    } else if (content.find(":theme ") == 0 && content.length() > 3) {
      if (state.provider.loadTheme(content.substr(7)))
        state.status = U"Theme: " + Utf8String(content.substr(7));
      return;
    }
    if (content == ":n" || content == ":new") {
      state.addCursor("");
      return;
    }
    if (content == ":bd") {
      if (c->edited) {
        state.status =
            create(state.path.length() ? state.path : "New File") + U" edited";
        vim->setSpecialCase(true);
        return;
      }
      state.deleteActive();
      return;
    }
    if (content == ":bd!") {
      state.deleteActive();
      return;
    }
    if (content.find(":w") == 0) {
      state.save();
      if (vim->getState().mode != 0 || content == ":w")
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
                U" edited, use :qa! again to exit";
            vim->setSpecialCase(true);
          } else {
            state.exitFlag = true;
            glfwSetWindowShouldClose(state.window, true);
          }
        }
      } else {
        if (c->edited && !forced) {
          state.status = create(state.path.length() ? state.path : "New File") +
                         U" edited, use :q! again to exit";
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
      return;
    }
    state.status = U"Unknown commmand " + Utf8String(content);
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
    auto window = vim->getState().window;
    auto mods = vim->getKeyState().mods;

    bool ctrl_pressed =
        glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ||
        mods & GLFW_MOD_CONTROL;
    bool shift_pressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
    State &st = vim->getState();

    if (mode == VimMode::INSERT && !ctrl_pressed) {
      bool useSpaces = st.provider.useSpaces;
      auto am = st.provider.tabWidth;
      if (useSpaces)
        cursor->append(std::string(am, ' '));
      else
        cursor->append('\t');
    } else if (mode == VimMode::NORMAL) {

      if (ctrl_pressed)
        vim->getState().fastSwitch();
      else if (shift_pressed) {
        st.fold();
        ActionResult r;
        r.allowCoords = false;
        return r;
      }
    } else if (shift_pressed && mode == VimMode::VISUAL) {
      st.fold();
      ActionResult r;
      r.type = ResultType::Emit;
      r.action_name = "ESC";
      r.allowCoords = false;
      return r;
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
    if (vim->activeAction() || mode == VimMode::VISUAL) {
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
        if (cursor->x > 0)
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
        if (cursor->x < cursor->getCurrentLineLength())
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
    if (vim->activeAction() || mode == VimMode::VISUAL) {
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
private:
  bool onlyCopy = false;
  bool onlyMark = false;

public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    if (state.isInital && mode != VimMode::VISUAL) {
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
              cursor->selection.activate(cursor->x == 0 ? 0 : cursor->x,
                                         cursor->y);
            else
              cursor->selection.activate(state.replaceMode == ReplaceMode::ALL
                                             ? cursor->x + 1
                                             : cursor->x,
                                         cursor->y);
            {
              auto &state = vim->getState();
              if (state.hasHighlighting)
                cursor->jumpMatching(
                    state.highlighter.language.stringCharacters,
                    state.highlighter.language.escapeChar);
              else {
                cursor->jumpMatching(U"\"", '\\');
              }
            }
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
            if(onlyMark)
              return withType(ResultType::Silent);
            Utf8String cc(cursor->getSelection());
            vim->getState().tryCopyInput(cc);
            if (!onlyCopy) {

              cursor->deleteSelection();
              cursor->center(cursor->y);
            }
            cursor->selection.stop();
            return {};
          }
        }
        if (state.action == "\"" || state.action == "'" ||
            state.action == "`") {

          auto result = cursor->findGlobal(true, Utf8String(state.action),
                                           cursor->x, cursor->y);
          auto resultRight = cursor->findGlobal(false, Utf8String(state.action),
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
          if(onlyMark)
            return withType(ResultType::Silent);
          Utf8String cc(cursor->getSelection());
          vim->getState().tryCopyInput(cc);
          if (!onlyCopy)
            cursor->deleteSelection();
          cursor->selection.stop();
          return {};
        }
      }
      Utf8String outBuffer;
      auto co = onlyCopy;
      Utf8String *ptr = &outBuffer;
      if (state.action == "w") {
        if (state.replaceMode == ReplaceMode::ALL) {
          vim->iterate([cursor, ptr, co]() {
            *ptr += cursor->deleteWordVim(true, !co);
          });
        } else {
          vim->iterate([cursor, ptr, co]() {
            *ptr += cursor->deleteWordVim(false, !co);
          });
        }
      } else if (state.action == "b") {
        if (state.replaceMode == ReplaceMode::ALL) {
          vim->iterate([cursor, ptr, co]() {
            *ptr += cursor->deleteWordBackwards(co);
            auto ch = cursor->removeOne(co);
            if (ch != 0)
              *ptr += ch;
          });
        } else {
          vim->iterate(
              [cursor, ptr, co]() { *ptr += cursor->deleteWordBackwards(co); });
        }
      } else if (state.action == "$") {
        Utf8String &str = cursor->lines[cursor->y];
        auto length = str.size() - cursor->x;
        Utf8String w = str.substr(cursor->x, length);
        if (!co) {

          str.erase(cursor->x, length);
          cursor->historyPush(3, w.length(), w);
        }
        *ptr += w;
      } else if (state.direction == Direction::UP) {
        auto out =
            cursor->deleteLines(cursor->y - state.count, 1 + state.count, !co);
        *ptr += out;
      } else if (state.direction == Direction::DOWN) {
        auto out = cursor->deleteLines(cursor->y, 1 + state.count, !co);
        *ptr += out;
      } else if (state.direction == Direction::LEFT) {
        vim->iterate([cursor, ptr, co]() {
          auto out = cursor->removeOne(co);
          if (out != 0)
            *ptr += out;
        });
      } else if (state.direction == Direction::RIGHT) {
        int offset = 0;
        int *off = &offset;
        vim->iterate([cursor, co, ptr, off]() {
          if (!co) {

            *ptr += cursor->removeBeforeCursor();
          } else {
            auto target = cursor->x + *off;
            if (target >= cursor->getCurrentLineLength())
              return;
            *ptr += cursor->lines[cursor->y][target];
            (*off)++;
          }
        });
      }
      if (outBuffer.length())
        vim->getState().tryCopyInput(outBuffer);
    }
    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (mode == VimMode::VISUAL) {
      vim->getState().tryCopy();
      cursor->deleteSelection();
      cursor->selection.stop();
      ActionResult r;
      r.type = ResultType::Emit;
      r.action_name = "ESC";
      return r;
    }
    if (vim->activeAction() == nullptr && mode == VimMode::NORMAL)
      return withType(ResultType::SetSelf);
    if (vim->activeAction() && vim->activeAction()->action_name == "d")
      return execute(mode, state, cursor, vim);
    return {};
  }
  ActionResult copyOnly(VimMode mode, MotionState &state, Cursor *cursor,
                        Vim *vim) {
    auto x = cursor->x;
    auto y = cursor->y;
    this->onlyCopy = true;
    auto out = execute(mode, state, cursor, vim);
    this->onlyCopy = false;
    cursor->x = x;
    cursor->y = y;
    return out;
  }
    ActionResult markOnly(VimMode mode, MotionState &state, Cursor *cursor,
                        Vim *vim) {
    this->onlyMark = true;
    auto out = execute(mode, state, cursor, vim);
    this->onlyMark = false;
    return out;
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

    if (!vim->activeAction() && mode == VimMode::NORMAL) {

      if (cursor->undo())
        vim->getState().status = U"Undo";
      else
        vim->getState().status = U"Undo failed";
      auto out = withType(ResultType::Silent);
      out.allowCoords = false;
      return out;
    }

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
      auto out = cursor->clearLine();
      vim->getState().tryCopyInput(out);
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
  DollarAction(bool ctrl_) : ctrl(ctrl_) {}
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    auto window = vim->getState().window;
    auto mods = vim->getKeyState().mods;

    bool ctrl_pressed =
        glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ||
        mods & GLFW_MOD_CONTROL;
    if (!vim->activeAction() &&
        (!ctrl ||
         (ctrl && ctrl_pressed && (mode == VimMode::INSERT || cursor->bind)))) {
      cursor->jumpEnd();
    } else {
      state.action = "$";
      return withType(ResultType::ExecuteSet);
    }
    return withType(ResultType::Silent);
  }

private:
  bool ctrl = false;
};

class ZeroAction : public Action {
public:
  ZeroAction(bool ctrl_) : ctrl(ctrl_) {}
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    auto window = vim->getState().window;
    auto mods = vim->getKeyState().mods;

    bool ctrl_pressed =
        glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ||
        mods & GLFW_MOD_CONTROL;
    if (!vim->activeAction() &&
        (!ctrl ||
         (ctrl && ctrl_pressed && (mode == VimMode::INSERT || cursor->bind)))) {
      cursor->jumpStart();
    }
    return withType(ResultType::Silent);
  }

private:
  bool ctrl = false;
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
private:
  DAction *d = nullptr;

public:
  YAction(DAction *d) { this->d = d; }
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    if (state.isInital) {
      auto l = cursor->lines[cursor->y];
      l += U"\n";
      vim->getState().tryCopyInput(l);
    } else {
      return d->copyOnly(mode, state, cursor, vim);
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
    if (!vim->activeAction())
      return withType(ResultType::SetSelf);
    if (vim->activeAction() && vim->activeAction()->action_name == "y")
      return execute(mode, state, cursor, vim);
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
      vim->getState().tryPaste();
      if (mode == VimMode::VISUAL) {
        vim->setMode(VimMode::NORMAL);
      }
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
      auto &state = vim->getState();
      if (state.hasHighlighting)
        cursor->jumpMatching(state.highlighter.language.stringCharacters,
                             state.highlighter.language.escapeChar);
      else {
        cursor->jumpMatching(U"\"", '\\');
      }
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
    if(cursor->y == cursor->lines.size()-1)
      cursor->jumpEnd();
    else
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
  DAction* d;

public:
  ParagraphAction(std::string symbol, DAction* d) { this->symbol = symbol; this->d = d; }
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (vim->activeAction() || mode == VimMode::VISUAL) {
      state.action = symbol;
      if(mode == VimMode::VISUAL)
        return d->markOnly(mode, state, cursor, vim);
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
  DAction* d;

public:
  BracketAction(std::string symbol, DAction* d) { this->symbol = symbol; this->d = d; }
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (vim->activeAction() || mode == VimMode::VISUAL) {
      state.action = symbol;
      if(mode == VimMode::VISUAL)
        return d->markOnly(mode, state, cursor, vim);
      return withType(ResultType::ExecuteSet);
    }
    return {};
  }
};
class ParenAction : public Action {
private:
  std::string symbol;
  DAction* d;

public:
  ParenAction(std::string symbol, DAction* d) { this->symbol = symbol; this->d = d; }
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (vim->activeAction() || mode == VimMode::VISUAL) {
      state.action = symbol;
      if(mode == VimMode::VISUAL)
        return d->markOnly(mode, state, cursor, vim);
      return withType(ResultType::ExecuteSet);
    }
    return {};
  }
};
class QuoteAction : public Action {
private:
  std::string symbol;
  DAction* d;

public:
  QuoteAction(std::string symbol, DAction* d) { this->symbol = symbol; this->d = d; }
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (vim->activeAction() || mode == VimMode::VISUAL) {
      state.action = symbol;
      if(mode == VimMode::VISUAL)
        return d->markOnly(mode, state, cursor, vim);
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
      return withType(ResultType::Silent);
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
        vim->getState().increaseFontSize(1);
      } else {
        vim->getState().increaseFontSize(-1);
      }
      auto out = withType(ResultType::Silent);
      out.allowCoords = false;
      return out;
    }
    return {};
  }
};
class MoveAction : public Action {
private:
  Direction direction;
  bool need_ctrl = true;

public:
  MoveAction(Direction v) : direction(v) {}
  MoveAction(Direction v, bool ctrl) : direction(v), need_ctrl(ctrl) {}
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    auto window = vim->getState().window;
    auto mods = vim->getKeyState().mods;
    bool ctrl_pressed =
        glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ||
        mods & GLFW_MOD_CONTROL || !need_ctrl;
    if (cursor->bind && ctrl_pressed) {
      if (direction == Direction::RIGHT)
        cursor->moveRight();
      else if (direction == Direction::LEFT)
        cursor->moveLeft();
      return withType(ResultType::Silent);
    }
    if (vim->activeAction() || (mode != VimMode::INSERT && need_ctrl)) {
      return withType(ResultType::Silent);
    }

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

class Finder : public Action {
private:
  bool active = false, backwards = false, before = false;
  int foundIndex = -1;
  char32_t cc;
  Cursor *cursor = nullptr;
  Vim *vim = nullptr;

public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {
    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    return {};
  }
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
class DDAction : public Action {

public:
  DDAction(DAction *finder_) : finder(finder_) {}
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (!vim->activeAction() && mode == VimMode::NORMAL) {
      MotionState st;
      st.isInital = false;
      st.action = "$";
      return finder->execute(mode, st, cursor, vim);
    }
    return {};
  }

private:
  DAction *finder;
};
class XAction : public Action {

public:
  XAction(bool control_) : control(control_) {}
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    auto window = vim->getState().window;
    auto mods = vim->getKeyState().mods;
    bool ctrl_pressed =
        glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ||
        mods & GLFW_MOD_CONTROL;
    if (cursor->bind && control && ctrl_pressed) {
      cursor->removeBeforeCursor();
      return withType(ResultType::Silent);
    }
    if (!vim->activeAction()) {
      if ((mode == VimMode::NORMAL && !control) ||
          (mode == VimMode::INSERT && control && ctrl_pressed)) {

        cursor->removeBeforeCursor();
      } else if (control && ctrl_pressed && mode != VimMode::INSERT) {
        cursor->gotoLine(cursor->y + (cursor->maxLines / 2) + 1);
      }
    }
    return withType(ResultType::Silent);
  }

private:
  bool control = false;
};
class CCAction : public Action {

public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (!vim->activeAction() && mode == VimMode::NORMAL) {

      if (cursor->x < cursor->lines[cursor->y].length()) {
        Utf8String &str = cursor->lines[cursor->y];
        auto length = str.size() - cursor->x;
        Utf8String w = str.substr(cursor->x, length);
        vim->getState().tryCopyInput(w);
        str.erase(cursor->x, length);
        cursor->historyPush(3, w.length(), w);
      }
      auto a = withType(ResultType::Emit);
      a.action_name = "i";
      return a;
    }
    return withType(ResultType::Silent);
  }
};
class CtrlUAction : public Action {

public:
  CtrlUAction(bool control_) : control(control_) {}
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    auto window = vim->getState().window;
    auto mods = vim->getKeyState().mods;
    bool ctrl_pressed =
        glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ||
        mods & GLFW_MOD_CONTROL;
    if (!vim->activeAction()) {
      if (control && ctrl_pressed) {
        cursor->gotoLine(cursor->y - (cursor->maxLines / 2) + 1);
      }
    }
    return withType(ResultType::Silent);
  }

private:
  bool control = false;
};
class CommentAction : public Action {

public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    auto window = vim->getState().window;
    auto mods = vim->getKeyState().mods;
    bool ctrl_pressed =
        glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ||
        mods & GLFW_MOD_CONTROL;
    if (!vim->activeAction()) {
      if (ctrl_pressed) {
        vim->getState().tryComment();
      }
    }
    return withType(ResultType::Silent);
  }

private:
};

class SimpleCopy : public Action {

public:
  SimpleCopy(bool copy_) : copy(copy_) {}
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {

    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    auto window = vim->getState().window;
    auto mods = vim->getKeyState().mods;
    bool ctrl_pressed =
        glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ||
        mods & GLFW_MOD_CONTROL;
    if (!vim->activeAction() && (mode == VimMode::INSERT || cursor->bind) &&
        ctrl_pressed) {
      if (copy) {
        vim->getState().tryCopy();
      } else {
        vim->getState().tryPaste();
      }
    }
    auto out = withType(ResultType::Silent);
    out.allowCoords = false;
    return out;
  }

private:
  bool copy = false;
};

class IndentAction : public Action {
public:
  ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override {
    int res = 0;
    State &st = vim->getState();
    auto useSpaces = st.provider.useSpaces;
    auto tabWidth = st.provider.tabWidth;
    if (st.hasHighlighting) {
      auto pref = useSpaces ? Utf8String(std::string(tabWidth, ' '))
                            : Utf8String(std::string(1, '\t'));
      auto &m = st.highlighter.indentLevels;
      if (state.isInital) {
        res = cursor->indent(m, cursor->y, cursor->y + 1, pref);

      } else if (state.action.length() == 1) {
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
              cursor->selection.activate(cursor->x == 0 ? 0 : cursor->x,
                                         cursor->y);
            else
              cursor->selection.activate(state.replaceMode == ReplaceMode::ALL
                                             ? cursor->x + 1
                                             : cursor->x,
                                         cursor->y);
            {
              if (st.hasHighlighting)
                cursor->jumpMatching(st.highlighter.language.stringCharacters,
                                     st.highlighter.language.escapeChar);
              else {
                cursor->jumpMatching(U"\"", '\\');
              }
            }
            res = cursor->indent(m, cursor->selection.getYSmaller(),
                                 cursor->selection.getYBigger(), pref);
            cursor->selection.stop();
            break;
          }
        }
      } else if (state.direction == Direction::UP ||
                 state.direction == Direction::DOWN) {
        if (state.count) {
          if (state.direction == Direction::UP) {
            res = cursor->indent(m, cursor->y - state.count, cursor->y, pref);
          } else {
            res = cursor->indent(m, cursor->y, cursor->y + state.count, pref);
          }
        }
      }
    }
    if (res) {
      st.status = U"Indented: " + Utf8String(std::to_string(res));
      ActionResult r = {};
      r.allowCoords = false;
      return r;
    }
    return {};
  }
  ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override {
    if (vim->activeAction() == nullptr && mode == VimMode::NORMAL)
      return withType(ResultType::SetSelf);
    if (vim->activeAction() == nullptr && mode == VimMode::VISUAL) {
      State &st = vim->getState();
      auto useSpaces = st.provider.useSpaces;
      auto tabWidth = st.provider.tabWidth;
      if (st.hasHighlighting) {
        auto pref = useSpaces ? Utf8String(std::string(tabWidth, ' '))
                              : Utf8String(std::string(1, '\t'));
        int res = cursor->indent(st.highlighter.indentLevels,
                                 cursor->selection.getYSmaller(),
                                 cursor->selection.getYBigger(), pref);
        if (res) {
          st.status = U"Indented: " + Utf8String(std::to_string(res));
          ActionResult r = {};
          r.allowCoords = false;
          return r;
        }
      }
    }
    if (vim->activeAction() && vim->activeAction()->action_name == "=")
      return execute(mode, state, cursor, vim);
    return {};
  }
};

void register_vim_commands(Vim &vim, State &state) {
  Finder *finder = new Finder();
  auto *d = new DAction();
  vim.registerTrie(new EscapeAction(), "ESC", GLFW_KEY_ESCAPE);
  vim.registerTrie(new BackspaceAction(), "BACKSPACE", GLFW_KEY_BACKSPACE);
  vim.registerTrie(new EnterAction(), "ENTER", GLFW_KEY_ENTER);
  vim.registerTrie(new TabAction(), "TAB", GLFW_KEY_TAB);
  vim.registerTrie(new XAction(true), "X_CTRL", GLFW_KEY_D);
  vim.registerTrie(new FontSizeAction(true), "F_INCREASE", GLFW_KEY_EQUAL);
  vim.registerTrie(new FontSizeAction(false), "F_DECREASE", GLFW_KEY_MINUS);
  vim.registerTrie(new TabAction(), "TAB", GLFW_KEY_TAB);
  vim.registerTrie(new CtrlUAction(true), "CTRL+U", GLFW_KEY_U);
  vim.registerTrie(new SimpleCopy(true), "CTRL+C", GLFW_KEY_C);
  vim.registerTrie(new SimpleCopy(false), "CTRL+V", GLFW_KEY_V);
  vim.registerTrie(new CommentAction(), "COMMENT", GLFW_KEY_SLASH);
  vim.registerTrie(new DollarAction(true), "CTRL+E", GLFW_KEY_E);
  vim.registerTrie(new ZeroAction(true), "CTRL+A", GLFW_KEY_A);
  vim.registerTrie(new MoveAction(Direction::UP), "M_UP", GLFW_KEY_P);
  vim.registerTrie(new MoveAction(Direction::RIGHT), "M_RIGHT", GLFW_KEY_F);
  vim.registerTrie(new MoveAction(Direction::DOWN), "M_DOWN", GLFW_KEY_N);
  vim.registerTrie(new MoveAction(Direction::LEFT), "M_LEFT", GLFW_KEY_B);
  vim.registerTrie(new MoveAction(Direction::UP, false), "M_ARROW_UP", GLFW_KEY_UP);
  vim.registerTrie(new MoveAction(Direction::RIGHT, false), "M_ARROW_RIGHT", GLFW_KEY_RIGHT);
  vim.registerTrie(new MoveAction(Direction::DOWN, false), "M_ARROW_DOWN", GLFW_KEY_DOWN);
  vim.registerTrie(new MoveAction(Direction::LEFT, false), "M_ARROW_LEFT", GLFW_KEY_LEFT);
  vim.registerTrieChar(new IAction(), "i", 'i');
  vim.registerTrieChar(new HAction(), "h", 'h');
  vim.registerTrieChar(new JAction(), "j", 'j');
  vim.registerTrieChar(new KAction(), "k", 'k');
  vim.registerTrieChar(new LAction(), "l", 'l');
  vim.registerTrieChar(new AAction(), "a", 'a');
  vim.registerTrieChar(new AAAction(), "A", 'A');
  vim.registerTrieChar(new WAction(), "w", 'w');
  vim.registerTrieChar(new BAction(), "b", 'b');
  vim.registerTrieChar(new OAction(), "o", 'o');
  vim.registerTrieChar(new OOAction(), "O", 'O');
  vim.registerTrieChar(d, "d", 'd');
  vim.registerTrieChar(new DDAction(d), "D", 'D');
  vim.registerTrieChar(new UAction(), "u", 'u');
  vim.registerTrieChar(new XAction(false), "x", 'x');
  vim.registerTrieChar(new VAction(), "v", 'v');
  vim.registerTrieChar(new CAction(), "c", 'c');
  vim.registerTrieChar(new CCAction(), "C", 'C');
  vim.registerTrieChar(new DollarAction(false), "$", '$');
  vim.registerTrieChar(new ZeroAction(false), "0", '0');
  vim.registerTrieChar(new ColonAction(), ":", ':');
  vim.registerTrieChar(new YAction(d), "y", 'y');
  vim.registerTrieChar(new PAction(), "p", 'p');
  vim.registerTrieChar(new PercentAction(), "%", '%');
  vim.registerTrieChar(new IIAction(), "I", 'I');
  vim.registerTrieChar(new GAction(), "g", 'g');
  vim.registerTrieChar(new GGAction(), "G", 'G');
  vim.registerTrieChar(new ParagraphAction("{", d), "{", '{');
  vim.registerTrieChar(new ParagraphAction("}", d), "}", '}');
  vim.registerTrieChar(new BracketAction("[", d), "[", '[');
  vim.registerTrieChar(new BracketAction("]", d), "]", ']');
  vim.registerTrieChar(new ParenAction("(", d), "(", '(');
  vim.registerTrieChar(new ParenAction(")", d), ")", ')');
  vim.registerTrieChar(new QuoteAction("\"", d), "\"", '\"');
  vim.registerTrieChar(new QuoteAction("'", d), "'", '\'');
  vim.registerTrieChar(new QuoteAction("`", d), "`", '`');
  vim.registerTrieChar(new SlashAction(), "/", '/');
  vim.registerTrieChar(new RAction(), "r", 'r');
  vim.registerTrieChar(new IndentAction(), "=", '=');
  vim.registerTrieChar(new FindAction(finder, false, false), "f", 'f');
  vim.registerTrieChar(new FindAction(finder, false, true), "t", 't');
  vim.registerTrieChar(new FindAction(finder, true, false), "F", 'F');
  vim.registerTrieChar(new FindAction(finder, true, true), "T", 'T');
  vim.registerTrieChar(new SemicolonAction(finder), ";", ';');
  vim.registerTrieChar(new CommaAction(finder), ",", ',');
  vim.addRef(finder);
  for (auto &entry : state.provider.vimRemaps)
    vim.remapCharTrie(entry.first, entry.second);
}

#endif