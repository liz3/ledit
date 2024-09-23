#include "../vim_actions.h"
ActionResult CAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){
    return {};
  }
ActionResult CAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
    if (vim->activeAction() && vim->activeAction()->action_name == "d") {
      auto out = cursor->clearLine();
      vim->getState().tryCopyInput(out);
      vim->setNext("i");
      return {};
    }
    if (!vim->activeAction()) {
      ActionResult r;
      r.type = ResultType::EmitAndSetFuture;
      r.action_name = "d";
      r.emit_next = "i";
      return r;
    }
    return {};
  }
