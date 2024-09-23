#include "../vim_actions.h"
CtrlUAction::CtrlUAction(bool control_) : control(control_) {}
ActionResult CtrlUAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){

    return {};
  }
ActionResult CtrlUAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
    auto window = vim->getState().window;
    auto mods = vim->getKeyState().mods;
    bool ctrl_pressed =
        glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ||
        mods & GLFW_MOD_CONTROL;
    if (!vim->activeAction()) {
      if (control && ctrl_pressed) {
        cursor->gotoLine(cursor->y - (cursor->maxLines / 2) + 1);
      }
    }
    return withType(ResultType::Silent);
  }
