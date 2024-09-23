#include "../vim_actions.h"
ActionResult EscapeAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){
    return {};
  }
ActionResult EscapeAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
    if (vim->getState().mode != 0) {
      bool shift_pressed =
          glfwGetKey(vim->getState().window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
      vim->getState().inform(false, shift_pressed);
      ActionResult r;
      r.allowCoords = false;
      return r;
    }
    vim->setSpecialCase(false);
    if (vim->isCommandBufferActive()) {
      cursor->unbind();
      vim->setIsCommandBufferActive(false);
    }
    vim->setInterceptor(nullptr);
    cursor->selection.stop();
    if (vim->activeAction()) {
      auto o = withType(ResultType::Cancel);
      o.allowCoords = false;
      return o;
    }
    if (mode == VimMode::VISUAL) {

      vim->setMode(VimMode::NORMAL);
    }
    if (mode == VimMode::INSERT)
      vim->setMode(VimMode::NORMAL);
    return {};
  }
