#include "../vim_actions.h"
ActionResult KAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){
    return {};
  }
ActionResult KAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
    if (vim->activeAction()) {
      state.direction = Direction::UP;
      ActionResult r;
      r.type = ResultType::ExecuteSet;
      return r;
    }
    if (mode == VimMode::NORMAL || mode == VimMode::VISUAL) {
      vim->iterate([cursor]() { cursor->moveUp(); });
    }
    return {};
  }
