#include "../vim_actions.h"
ActionResult PercentAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){

    return {};
  }
ActionResult PercentAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){

    if (!vim->activeAction()) {
      auto &state = vim->getState();
      if (state.hasHighlighting)
        cursor->jumpMatching(state.highlighter.language.stringCharacters,
                             state.highlighter.language.escapeChar);
      else {
        cursor->jumpMatching(U"\"", '\\');
      }
    }
    return {};
  }
