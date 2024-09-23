#include "../vim_actions.h"
ActionResult UAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){

    return {};
  }
ActionResult UAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){

    if (!vim->activeAction() && mode == VimMode::NORMAL) {

      if (cursor->undo())
        vim->getState().status = U"Undo";
      else
        vim->getState().status = U"Undo failed";
      auto out = withType(ResultType::Silent);
      out.allowCoords = false;
      return out;
    }

    return {};
  }
