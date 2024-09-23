#include "../vim_actions.h"
SemicolonAction::SemicolonAction(Finder *finder_) : finder(finder_) {}
ActionResult SemicolonAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){

    return {};
  }
ActionResult SemicolonAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
    if (!vim->activeAction()) {
      finder->next(false);
    }
    return {

    };
  }
