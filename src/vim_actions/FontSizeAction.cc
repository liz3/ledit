#include "../vim_actions.h"
FontSizeAction::FontSizeAction(bool v) : increase(v) {}
ActionResult FontSizeAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){

    return {};
  }
ActionResult FontSizeAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
    if (vim->activeAction()) {
      return withType(ResultType::Silent);
    }
    auto window = vim->getState().window;
    auto mods = vim->getKeyState().mods;
    bool ctrl_pressed =
        glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ||
        mods & GLFW_MOD_CONTROL;
    if (ctrl_pressed) {
      if (increase) {
        vim->getState().increaseFontSize(1);
      } else {
        vim->getState().increaseFontSize(-1);
      }
      auto out = withType(ResultType::Silent);
      out.allowCoords = false;
      return out;
    }
    return {};
  }
