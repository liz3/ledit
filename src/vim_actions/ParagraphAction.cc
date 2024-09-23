#include "../vim_actions.h"
ParagraphAction::ParagraphAction(std::string symbol, DAction *d) {
    this->symbol = symbol;
    this->d = d;
  }
ActionResult ParagraphAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){

    return {};
  }
ActionResult ParagraphAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
    if (vim->activeAction() || mode == VimMode::VISUAL) {
      state.action = symbol;
      if (mode == VimMode::VISUAL)
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
