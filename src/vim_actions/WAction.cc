#include "../vim_actions.h"
ActionResult WAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){
    return {};
  }
ActionResult WAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
    if (vim->activeAction()) {
      state.action = "w";
      return withType(ResultType::ExecuteSet);
    }
    if (mode == VimMode::NORMAL || mode == VimMode::VISUAL) {
      vim->iterate([cursor]() { cursor->advanceWord(); });
    }
    return {};
  }
