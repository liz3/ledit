#include "../vim_actions.h"
FindAction::FindAction(Finder *finder_, bool backwards_, bool offset_)
      : finder(finder_), backwards(backwards_), offset(offset_) {
    interceptor = new FindInterceptor(this);
  }
ActionResult FindAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){

    return {};
  }
ActionResult FindAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
    if (vim->getInterceptor() != interceptor && !vim->activeAction())
      vim->setInterceptor(interceptor);
    return withType(ResultType::Silent);
  }
