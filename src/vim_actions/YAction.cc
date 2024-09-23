#include "../vim_actions.h"
YAction::YAction(DAction *d) { this->d = d; }
ActionResult YAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){

    if (state.isInital) {
      auto l = cursor->lines[cursor->y];
      l += U"\n";
      vim->getState().tryCopyInput(l);
      auto out = withType(ResultType::Done);
      out.allowCoords = false;
      return out;
    } else {
      return d->copyOnly(mode, state, cursor, vim);
    }
    return {};
  }
ActionResult YAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){

    if (!vim->activeAction() && mode == VimMode::VISUAL) {
      vim->getState().tryCopy();
      cursor->selection.stop();
      vim->setMode(VimMode::NORMAL);
      ActionResult r;
      r.allowCoords = false;
      return r;
    }
    if (!vim->activeAction())
      return withType(ResultType::SetSelf);
    if (vim->activeAction() && vim->activeAction()->action_name == "y")
      return execute(mode, state, cursor, vim);
    return {};
  }
