#include "../vim_actions.h"
ActionResult OAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){
    return {};
  }
ActionResult OAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
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
