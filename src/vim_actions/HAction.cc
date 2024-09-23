#include "../vim_actions.h"
ActionResult HAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){
    return {};
  }
ActionResult HAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
    if (vim->activeAction()) {
      state.direction = Direction::LEFT;
      ActionResult r;
      r.type = ResultType::ExecuteSet;
      return r;
    }
    if (mode == VimMode::NORMAL || mode == VimMode::VISUAL) {
      vim->iterate([cursor]() {
        if (cursor->x > 0)
          cursor->moveLeft();
      });
    }
    return {};
  }
