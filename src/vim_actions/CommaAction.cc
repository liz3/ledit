#include "../vim_actions.h"
CommaAction::CommaAction(Finder *finder_) : finder(finder_) {}
ActionResult CommaAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){

    return {};
  }
ActionResult CommaAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
    if (!vim->activeAction()) {
      finder->next(true);
    }
    return {};
  }
