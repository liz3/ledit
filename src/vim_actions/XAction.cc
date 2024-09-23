#include "../vim_actions.h"
XAction::XAction(bool control_) : control(control_) {}
ActionResult XAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){

    return {};
  }
ActionResult XAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
    auto window = vim->getState().window;
    auto mods = vim->getKeyState().mods;
    bool ctrl_pressed =
        glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ||
        mods & GLFW_MOD_CONTROL;
    if (cursor->bind && control && ctrl_pressed) {
      cursor->removeBeforeCursor();
      return withType(ResultType::Silent);
    }
    if (!vim->activeAction()) {
      if ((mode == VimMode::NORMAL && !control) ||
          (mode == VimMode::INSERT && control && ctrl_pressed)) {

        cursor->removeBeforeCursor();
      } else if (control && ctrl_pressed && mode != VimMode::INSERT) {
        cursor->gotoLine(cursor->y + (cursor->maxLines / 2) + 1);
      }
    }
    return withType(ResultType::Silent);
  }
