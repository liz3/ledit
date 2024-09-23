#include "../vim_actions.h"
SimpleCopy::SimpleCopy(bool copy_) : copy(copy_) {}
ActionResult SimpleCopy::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){

    return {};
  }
ActionResult SimpleCopy::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
    auto window = vim->getState().window;
    auto mods = vim->getKeyState().mods;
    bool ctrl_pressed =
        glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ||
        mods & GLFW_MOD_CONTROL;
    if (!vim->activeAction() && (mode == VimMode::INSERT || cursor->bind) &&
        ctrl_pressed) {
      if (copy) {
        vim->getState().tryCopy();
      } else {
        vim->getState().tryPaste();
      }
    }
    auto out = withType(ResultType::Silent);
    out.allowCoords = false;
    return out;
  }
