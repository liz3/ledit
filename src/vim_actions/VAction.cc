#include "../vim_actions.h"
ActionResult VAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){

    return {};
  }
ActionResult VAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){

    if (!vim->activeAction() && mode == VimMode::NORMAL) {
      vim->setMode(VimMode::VISUAL);
      if (cursor->x == cursor->getCurrentLineLength() - 1)
        cursor->x++;
      cursor->selection.activate(cursor->x, cursor->y);
    } else if (mode == VimMode::VISUAL) {
      auto a = withType(ResultType::Emit);
      a.action_name = "ESC";
      return a;
    }
    return {};
  }
