#include "../vim_actions.h"
ActionResult DAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){

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
            if (onlyMark)
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
          if (onlyMark)
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
ActionResult DAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
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
ActionResult DAction::copyOnly(VimMode mode, MotionState &state, Cursor *cursor,
                        Vim *vim){
    auto x = cursor->x;
    auto y = cursor->y;
    this->onlyCopy = true;
    auto out = execute(mode, state, cursor, vim);
    this->onlyCopy = false;
    cursor->x = x;
    cursor->y = y;
    out.allowCoords = false;
    return out;
  }
ActionResult DAction::markOnly(VimMode mode, MotionState &state, Cursor *cursor,
                        Vim *vim){
    this->onlyMark = true;
    auto out = execute(mode, state, cursor, vim);
    this->onlyMark = false;
    return out;
  }
