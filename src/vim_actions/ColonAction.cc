#include "../vim_actions.h"
ActionResult ColonAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){

    return {};
  }
ActionResult ColonAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){

    if (!vim->activeAction() && mode == VimMode::NORMAL) {
      State &st = vim->getState();

      st.miniBuf = U":";
      vim->getState().status = U"";
      cursor->bindTo(&st.miniBuf);
      vim->setIsCommandBufferActive(true);
      auto out = withType(ResultType::Silent);
      out.allowCoords = false;
      return out;
    }
    return {};
  }
