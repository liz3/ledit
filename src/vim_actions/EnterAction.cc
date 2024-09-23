#include "../vim_actions.h"
#include "../u8String.h"
ActionResult EnterAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){
    return {};
  }
ActionResult EnterAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
    if (vim->getState().mode != 0 ||
        (!vim->isCommandBufferActive() && cursor->isFolder)) {
      auto window = vim->getState().window;
      auto mods = vim->getKeyState().mods;

      bool ctrl_pressed =
          glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
          glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ||
          mods & GLFW_MOD_CONTROL;
      bool shift_pressed =
          glfwGetKey(vim->getState().window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

      vim->getState().inform(true, shift_pressed, ctrl_pressed);
      auto out = withType(ResultType::Silent);
      out.allowCoords = false;
      return out;
    }
    if (mode == VimMode::INSERT) {
      cursor->append('\n');
    }
    if (vim->isCommandBufferActive()) {
      Utf8String content = vim->getState().miniBuf;
      cursor->unbind();
      vim->setIsCommandBufferActive(false);
      commandParser(content, vim, cursor);
      auto out = withType(ResultType::Silent);
      out.allowCoords = false;
      return out;
    }

    return {};
  }
void EnterAction::commandParser(Utf8String &buffer, Vim *vim, Cursor *c){
    std::string content = buffer.getStr();
    State &state = vim->getState();
    if (content == "/") {
      state.search();
      return;
    }
    if (content == ":b") {
      state.switchBuffer();
      return;
    }
    if (content == ":c") {
      state.command();
      return;
    } else if (content.find(":c ") == 0 && content.length() > 3) {
      state.runCommand(content.substr(3));
      return;
    }
    if (content == ":ck") {
      state.killCommand();
      return;
    }
    if (content == ":co") {
      state.activateLastCommandBuffer();
      return;
    }
    if (content == ":%s" || content == ":replace") {
      state.startReplace();
      return;
    }
    if (content == ":lw") {
      state.toggleLineWrapping();
      return;
    }
    if (content.find(":tw ") == 0 && content.length() > 4) {
      state.provider.tabWidth = std::stoi(content.substr(4));
      state.status = U"Tab Width: " + Utf8String(content.substr(4));
      return;
    }
    if (content == ":hl") {
      state.switchLineHighlightMode();
      return;
    }
    if (content == ":rconfig" || content == ":rc") {
      state.provider.reloadConfig();
      return;
    }
    if (content == ":ln") {
      state.showLineNumbers = !state.showLineNumbers;
      return;
    }
    if (content == ":font") {
      state.changeFont();
      return;
    }
    if (content == ":config") {
      state.addCursor(state.provider.getConfigPath());
      return;
    }
    if (content == ":mode") {
      state.switchMode();
      return;
    } else if (content.find(":mode ") == 0 && content.length() > 6) {
      state.directlyEnableLanguage(content.substr(6));
      return;
    }
    if (content == ":e") {
      state.open();
      return;
    } else if (content.find(":e ") == 0 && content.length() > 3) {
      state.addCursor(content.substr(3));
      return;
    }
    if (content == ":win") {
      state.open(true);
      return;
    } else if (content.find(":win ") == 0 && content.length() > 3) {
      add_window(content.substr(5));
      return;
    }
    if (content == ":sh") {
      state.shellCommand();
      return;
    }
    if (content.find(":sh ") == 0) {
      if (state.execCommand(content.substr(4))) {
        state.lastCmd = content.substr(4);
      }
      return;
    }

    if (content == ":theme") {
      state.setTheme();
      return;
    } else if (content.find(":theme ") == 0 && content.length() > 3) {
      if (state.provider.loadTheme(content.substr(7)))
        state.status = U"Theme: " + Utf8String(content.substr(7));
      return;
    }
    if (content == ":n" || content == ":new") {
      state.addCursor("");
      return;
    }
    if (content == ":bd") {
      if (c->edited) {
        state.status =
            create(state.path.length() ? state.path : "New File") + U" edited";
        vim->setSpecialCase(true);
        return;
      }
      state.deleteActive();
      return;
    }
    if (content == ":bd!") {
      state.deleteActive();
      return;
    }
    if (content.find(":w") == 0) {
      state.save();
      if (vim->getState().mode != 0 || content == ":w")
        return;
    }

    if (content.find("q") != std::string::npos) {
      bool forced = content.find("!") != std::string::npos;
      if (content.find("a") != std::string::npos) {
        if (forced) {
          state.exitFlag = true;
          glfwSetWindowShouldClose(state.window, true);
        } else {
          CursorEntry *edited = state.hasEditedBuffer();
          if (edited) {
            state.status =
                create(edited->path.length() ? edited->path : "New File") +
                U" edited, use :qa! again to exit";
            vim->setSpecialCase(true);
          } else {
            state.exitFlag = true;
            glfwSetWindowShouldClose(state.window, true);
          }
        }
      } else {
        if (c->edited && !forced) {
          state.status = create(state.path.length() ? state.path : "New File") +
                         U" edited, use :q! again to exit";
          vim->setSpecialCase(true);
        } else {
          if (state.cursors.size() == 1) {
            state.exitFlag = true;
            glfwSetWindowShouldClose(state.window, true);
          } else {
            state.deleteActive();
          }
        }
      }
      return;
    }
    state.status = U"Unknown commmand " + Utf8String(content);
  }
