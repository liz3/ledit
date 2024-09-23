#include "../vim_actions.h"
ActionResult TabAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){
    return {};
  }
ActionResult TabAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){

    if (vim->getState().mode != 0) {
      bool shift_pressed =
          glfwGetKey(vim->getState().window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
      vim->getState().provideComplete(shift_pressed);
      auto out = withType(ResultType::Silent);
      out.allowCoords = false;
      return out;
    }
    auto window = vim->getState().window;
    auto mods = vim->getKeyState().mods;

    bool ctrl_pressed =
        glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ||
        mods & GLFW_MOD_CONTROL;
    bool shift_pressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
    State &st = vim->getState();

    if (mode == VimMode::INSERT && !ctrl_pressed) {
      bool useSpaces = st.provider.useSpaces;
      auto am = st.provider.tabWidth;
      if (useSpaces)
        cursor->append(std::string(am, ' '));
      else
        cursor->append('\t');
    } else if (mode == VimMode::NORMAL) {

      if (ctrl_pressed)
        vim->getState().fastSwitch();
      else if (shift_pressed) {
        st.fold();
        ActionResult r;
        r.allowCoords = false;
        return r;
      }
    } else if (shift_pressed && mode == VimMode::VISUAL) {
      st.fold();
      ActionResult r;
      r.type = ResultType::Emit;
      r.action_name = "ESC";
      r.allowCoords = false;
      return r;
    }
    return {};
  }
