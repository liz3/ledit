#include "../vim_actions.h"
ActionResult RAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){

    return {};
  }
ActionResult RAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
    if (vim->getInterceptor() != &interceptor && !vim->activeAction() &&
        mode == VimMode::NORMAL)
      vim->setInterceptor(&interceptor);
    return withType(ResultType::Silent);
  }
