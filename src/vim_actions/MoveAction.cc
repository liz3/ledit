#include "../vim_actions.h"
MoveAction::MoveAction(Direction v) : direction(v) {}
MoveAction::MoveAction(Direction v, bool ctrl) : direction(v), need_ctrl(ctrl) {}
ActionResult MoveAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){

    return {};
  }
ActionResult MoveAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
    auto window = vim->getState().window;
    auto mods = vim->getKeyState().mods;
    bool ctrl_pressed =
        glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ||
        mods & GLFW_MOD_CONTROL || !need_ctrl;
    if (cursor->bind && ctrl_pressed) {
      if (direction == Direction::RIGHT)
        cursor->moveRight();
      else if (direction == Direction::LEFT)
        cursor->moveLeft();
      return withType(ResultType::Silent);
    }
    if (vim->activeAction() || (mode != VimMode::INSERT && need_ctrl)) {
      return withType(ResultType::Silent);
    }

    if (ctrl_pressed) {
      if (direction == Direction::UP)
        cursor->moveUp();
      else if (direction == Direction::RIGHT)
        cursor->moveRight();
      else if (direction == Direction::LEFT)
        cursor->moveLeft();
      else if (direction == Direction::DOWN)
        cursor->moveDown();
    }
    return {};
  }
