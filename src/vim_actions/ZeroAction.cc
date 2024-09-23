#include "../vim_actions.h"
ZeroAction::ZeroAction(bool ctrl_) : ctrl(ctrl_) {}
ActionResult ZeroAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){

    return {};
  }
ActionResult ZeroAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
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
      cursor->jumpStart();
    }
    return withType(ResultType::Silent);
  }
