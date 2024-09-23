#include "../vim_actions.h"
ActionResult LAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){
    return {};
  }
ActionResult LAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
    if (vim->activeAction()) {
      state.direction = Direction::RIGHT;
      ActionResult r;
      r.type = ResultType::ExecuteSet;
      return r;
    }
    if (mode == VimMode::NORMAL || mode == VimMode::VISUAL) {
      vim->iterate([cursor]() {
        if (cursor->x < cursor->getCurrentLineLength())
          cursor->moveRight();
      });
    }
    return {};
  }
