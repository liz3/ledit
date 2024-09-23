#include "../vim_actions.h"
ActionResult SlashAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){

    return {};
  }
ActionResult SlashAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){

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
