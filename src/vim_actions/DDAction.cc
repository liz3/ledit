#include "../vim_actions.h"
DDAction::DDAction(DAction *finder_) : finder(finder_) {}
ActionResult DDAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){

    return {};
  }
ActionResult DDAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
    if (!vim->activeAction() && mode == VimMode::NORMAL) {
      MotionState st;
      st.isInital = false;
      st.action = "$";
      return finder->execute(mode, st, cursor, vim);
    }
    return {};
  }
