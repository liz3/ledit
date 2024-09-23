#include "../vim_actions.h"
ActionResult JAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){
    return {};
  }
ActionResult JAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
    if (vim->activeAction()) {
      state.direction = Direction::DOWN;
      ActionResult r;
      r.type = ResultType::ExecuteSet;
      return r;
    }
    if (mode == VimMode::NORMAL || mode == VimMode::VISUAL) {
      vim->iterate([cursor]() { cursor->moveDown(); });
    }
    return {};
  }
