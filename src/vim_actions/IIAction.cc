#include "../vim_actions.h"
ActionResult IIAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){

    return {};
  }
ActionResult IIAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
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
