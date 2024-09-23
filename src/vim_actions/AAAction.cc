#include "../vim_actions.h"
ActionResult AAAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){
    return {};
  }
ActionResult AAAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
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
