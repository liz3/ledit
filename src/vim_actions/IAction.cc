#include "../vim_actions.h"
ActionResult IAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){
    return {

    };
  }
ActionResult IAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
    if (vim->activeAction() || mode == VimMode::VISUAL) {
      if (state.replaceMode == ReplaceMode::UNSET)
        state.replaceMode = ReplaceMode::INNER;
      return withType(ResultType::Silent);
    }
    vim->setMode(VimMode::INSERT);
    return {};
  }
