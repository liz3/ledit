#include "../vim_actions.h"
ActionResult BAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){
    return {};
  }
ActionResult BAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
    if (vim->activeAction()) {
      state.action = "b";
      return withType(ResultType::ExecuteSet);
    }
    if (mode == VimMode::NORMAL || mode == VimMode::VISUAL) {
      vim->iterate([cursor]() { cursor->advanceWordBackwards(); });
    }
    return {};
  }
