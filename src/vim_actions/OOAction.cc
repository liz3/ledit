#include "../vim_actions.h"
ActionResult OOAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){
    return {};
  }
ActionResult OOAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
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
