#include "../vim_actions.h"
DollarAction::DollarAction(bool ctrl_) : ctrl(ctrl_) {}
ActionResult DollarAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){

    return {};
  }
ActionResult DollarAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
    auto window = vim->getState().window;
    auto mods = vim->getKeyState().mods;

    bool ctrl_pressed =
        glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ||
        mods & GLFW_MOD_CONTROL;
    if (!vim->activeAction() &&
        (!ctrl ||
         (ctrl && ctrl_pressed && (mode == VimMode::INSERT || cursor->bind)))) {
      cursor->jumpEnd();
    } else {
      state.action = "$";
      return withType(ResultType::ExecuteSet);
    }
    return withType(ResultType::Silent);
  }
