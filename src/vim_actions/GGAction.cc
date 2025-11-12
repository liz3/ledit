#include "../vim_actions.h"
ActionResult GGAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){

    return {};
  }
ActionResult GGAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
    if (vim->activeAction()) {
      return withType(ResultType::Silent);
    }
    if (state.count) {
      cursor->gotoLine(state.count);
      return {};
    }
    if (cursor->y == cursor->lines.size() - 1) 
      cursor->jumpEnd();
    else{
      cursor->gotoLine(cursor->lines.size());
       cursor->jumpEnd();
    }
    return {};
  }
