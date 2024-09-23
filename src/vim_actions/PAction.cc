#include "../vim_actions.h"
ActionResult PAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){

    return {};
  }
ActionResult PAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){

    if (!vim->activeAction() && mode == VimMode::NORMAL ||
        mode == VimMode::VISUAL) {
      vim->getState().tryPaste();
      if (mode == VimMode::VISUAL) {
        vim->setMode(VimMode::NORMAL);
      }
      ActionResult r;
      r.allowCoords = false;
      return r;
    }
    return {};
  }
