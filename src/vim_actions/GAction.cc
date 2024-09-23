#include "../vim_actions.h"
ActionResult GAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){

    return {};
  }
ActionResult GAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
    if (vim->activeAction()) {
      if (vim->activeAction()->action_name == "g") {
        cursor->gotoLine(1);
        return {};
      }
      return withType(ResultType::Silent);
    }
    return withType(ResultType::SetSelf);
  }
