#include "../vim_actions.h"
ActionResult CommentAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){

    return {};
  }
ActionResult CommentAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
    auto window = vim->getState().window;
    auto mods = vim->getKeyState().mods;
    bool ctrl_pressed =
        glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ||
        mods & GLFW_MOD_CONTROL;
    if (!vim->activeAction()) {
      if (ctrl_pressed) {
        vim->getState().tryComment();
      }
    }
    return withType(ResultType::Silent);
  }
