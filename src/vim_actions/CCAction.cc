#include "../vim_actions.h"
ActionResult CCAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){

    return {};
  }
ActionResult CCAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
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
