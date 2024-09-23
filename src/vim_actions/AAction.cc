#include "../vim_actions.h"
ActionResult AAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){
    return {};
  }
ActionResult AAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
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
