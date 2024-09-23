#include "../vim_actions.h"
ActionResult BackspaceAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){
    return {};
  }
ActionResult BackspaceAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
    if (mode == VimMode::INSERT || cursor->bind) {
      bool alt_pressed =
          glfwGetKey(vim->getState().window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS;
      if (alt_pressed)
        cursor->deleteWordBackwards();
      else
        cursor->removeOne();
    }
    return {};
  }
