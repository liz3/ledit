#if (MAC_OS_X_VERSION_MAX_ALLOWED < 120000) // Before macOS 12 Monterey
#define kIOMainPortDefault kIOMasterPortDefault
#endif
#include <iostream>
#include <math.h>
#include <map>
#include <string>
#include <vector>
#ifndef __APPLE__
#include <algorithm>
#endif
#ifdef _WIN32
#include <Windows.h>
#endif
#include "la.h"
#include "glad.h"
#include "../third-party/glfw/include/GLFW/glfw3.h"
#include "../third-party/freetype2/include/ft2build.h"
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H
#include "state.h"
#include "shader.h"
#include "font_atlas.h"
#include "cursor.h"
#include "shaders.h"
#include "highlighting.h"
#include "windows.h"
#include "u8String.h"
#ifdef LEDIT_WIN_MAIN
#include "win32_icon_utils.h"
#endif
WindowManager *g_windows = nullptr;
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  auto *gState = g_windows->windows[window]->state;
  glfwMakeContextCurrent(window);
  glViewport(0, 0, width, height);
  if (gState != nullptr) {
    gState->invalidateCache();

    gState->WIDTH = (float)width;
    gState->HEIGHT = (float)height;
  }
}
void window_focus_callback(GLFWwindow *window, int focused) {
  auto *gState = g_windows->windows[window]->state;

  gState->invalidateCache();
  gState->focused = focused;
  if (focused) {
    gState->checkChanged();
  }
}

void drop_callback(GLFWwindow *window, int count, const char **paths) {
  auto *gState = g_windows->windows[window]->state;

  for (size_t i = 0; i < count; i++) {
    const char *cstr = paths[i];
    std::string s(cstr);
    if (fs::is_directory(s)) {
      add_window(s);
    } else {
      gState->addCursor(s);
    }
  }
}
void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods) {
  auto *gState = g_windows->windows[window]->state;
   glfwMakeContextCurrent(gState->window);

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    gState->invalidateCache();
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    float xscale, yscale;
    glfwGetWindowContentScale(window, &xscale, &yscale);
    gState->cursor->setPosFromMouse((float)xpos * xscale, (float)ypos * yscale,
                                    gState->atlas, gState->lineWrapping);

    if (gState->mode == 0 &&
        (!gState->vim || gState->vim->shouldRenderCoords()))
      gState->renderCoords();
  }
}

void character_callback(GLFWwindow *window, unsigned int codepoint) {
  auto *gState = g_windows->windows[window]->state;
  if(gState->provider.enableAccents && gState->accentManager.blockCp(codepoint)){
     gState->renderCoords();
    return;
  }
 glfwMakeContextCurrent(gState->window);
  gState->invalidateCache();
  gState->exitFlag = false;
  if (gState->exitLoop) {
    glfwSetWindowShouldClose(window, false);
  }
  gState->exitLoop = false;
  if (gState->vim) {
    auto r = gState->vim->processCharacter((char32_t)codepoint);
    if (r && gState->vim->shouldRenderCoords())
      gState->renderCoords();
    return;
  }

#ifdef _WIN32
  bool ctrl_pressed = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
  if (ctrl_pressed)
    return;
#endif
  bool alt_pressed = glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS;
  if (alt_pressed) {
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
      gState->cursor->advanceWord();
      return;
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
      gState->tryCopy();
      return;
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
      gState->cursor->advanceWordBackwards();
      return;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
      gState->cursor->deleteWord();
      return;
    }
  }
  gState->cursor->append((char16_t)codepoint);
  gState->renderCoords();
}
void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  auto *gState = g_windows->windows[window]->state;
 glfwMakeContextCurrent(gState->window);

  if (gState == nullptr)
    return;
  gState->invalidateCache();
  if(gState->provider.enableAccents && gState->accentManager.checkBlock(key, action)) {
     gState->accentManager.processEvent(window, key, scancode, action, mods);
        gState->renderCoords();
     return;
  }
  if (gState->vim) {
        if(gState->provider.useCapsAsEscape && key == GLFW_KEY_CAPS_LOCK)
          key = GLFW_KEY_ESCAPE;
      auto r = gState->vim->processKey(key, scancode, action, mods);
      if (r && gState->vim->shouldRenderCoords())
        gState->renderCoords();
      if(gState->provider.enableAccents && gState->vim->getMode() == VimMode::INSERT)
       gState->accentManager.processEvent(window, key, scancode, action, mods);

    return;
  }
  if (key == GLFW_KEY_ESCAPE || (gState->provider.useCapsAsEscape && key == GLFW_KEY_CAPS_LOCK)) {
    if (action == GLFW_PRESS) {
      if (gState->cursor->selection.active) {
        gState->cursor->selection.stop();
        return;
      }
      if (gState->mode != 0) {
        gState->inform(false, false);
      } else {
        CursorEntry *edited = gState->hasEditedBuffer();
        if (gState->exitFlag || edited == nullptr) {
          glfwSetWindowShouldClose(window, true);
        } else {
          gState->exitFlag = true;
          gState->status =
              create(edited->path.length() ? edited->path : "New File") +
              U" edited, press ESC again to exit";
        }
      }
    }
    return;
  }
  gState->exitFlag = false;
  if (gState->exitLoop) {
    glfwSetWindowShouldClose(window, false);
  }
  gState->exitLoop = false;
  bool ctrl_pressed =
      glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
      glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ||
      mods & GLFW_MOD_CONTROL;
  gState->ctrlPressed = ctrl_pressed;
  bool shift_pressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
  bool x_pressed = glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS;
  bool alt_pressed = glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS;
  Cursor *cursor = gState->cursor;
  bool isPress = action == GLFW_PRESS || action == GLFW_REPEAT;
#ifndef __APPLE__
  if (alt_pressed) {
    if (key == GLFW_KEY_F && isPress) {
      gState->cursor->advanceWord();
      return;
    }
    if (key == GLFW_KEY_W && action == GLFW_PRESS) {
      gState->tryCopy();
      return;
    }
    if (key == GLFW_KEY_B && isPress) {
      gState->cursor->advanceWordBackwards();
      return;
    }
    if (key == GLFW_KEY_D && isPress) {
      gState->cursor->deleteWord();
      return;
    }
  }
#endif
  if (ctrl_pressed) {

    if (x_pressed) {
      if (action == GLFW_PRESS && key == GLFW_KEY_S) {
        gState->save();
      }
      if (action == GLFW_PRESS && key == GLFW_KEY_SLASH) {
        gState->tryComment();
      }
      if (action == GLFW_PRESS && key == GLFW_KEY_M) {
        gState->switchMode();
      }
      if (action == GLFW_PRESS && key == GLFW_KEY_C) {
        gState->addCursor(gState->provider.getConfigPath());
      }
      if (action == GLFW_PRESS && key == GLFW_KEY_Y) {
        gState->provider.reloadConfig();
      }
      if (action == GLFW_PRESS && key == GLFW_KEY_L) {
        gState->showLineNumbers = !gState->showLineNumbers;
      }
      if (action == GLFW_PRESS && key == GLFW_KEY_R) {
        gState->toggleLineWrapping();
      }
      if (action == GLFW_PRESS && key == GLFW_KEY_T) {
        gState->setTheme();
      }
      if (action == GLFW_PRESS && key == GLFW_KEY_SEMICOLON) {
        gState->activateLastCommandBuffer();
      }
      if (action == GLFW_PRESS && key == GLFW_KEY_P) {
        gState->killCommand();
      }
      if (action == GLFW_PRESS && key == GLFW_KEY_H) {
        gState->switchLineHighlightMode();
      }
      if (action == GLFW_PRESS && key == GLFW_KEY_O) {
        gState->open(shift_pressed);
      }
      if (action == GLFW_PRESS && key == GLFW_KEY_0) {
        gState->changeFont();
      }
      if (action == GLFW_PRESS && key == GLFW_KEY_PERIOD) {
        gState->shellCommand();
      }
      if (action == GLFW_PRESS && key == GLFW_KEY_K) {
        gState->switchBuffer();
      }
      if (action == GLFW_PRESS && key == GLFW_KEY_N) {
        gState->saveNew();
      }
      if (action == GLFW_PRESS && key == GLFW_KEY_G) {
        gState->gotoLine();
      }
      if (action == GLFW_PRESS && key == GLFW_KEY_W) {
        gState->deleteActive();
      }
      if (action == GLFW_PRESS && key == GLFW_KEY_A) {
        cursor->gotoLine(1);
        gState->renderCoords();
      }
      if (action == GLFW_PRESS && key == GLFW_KEY_E) {
        cursor->gotoLine(cursor->lines.size());
        gState->renderCoords();
      }
      return;
    }
    if (shift_pressed) {
      if (key == GLFW_KEY_P && isPress) {
        gState->cursor->moveLine(-1);
      } else if (key == GLFW_KEY_N && isPress) {
        gState->cursor->moveLine(1);
      }
      gState->renderCoords();
      return;
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
      gState->search();
    } else if (key == GLFW_KEY_R && isPress) {
      gState->startReplace();
    } else if (key == GLFW_KEY_SEMICOLON && isPress) {
      gState->command();
    } else if (key == GLFW_KEY_Z && isPress) {
      gState->undo();
    } else if (key == GLFW_KEY_W && isPress) {
      gState->cut();
    } else if (key == GLFW_KEY_TAB && isPress) {
      gState->fastSwitch();
      return;
    } else if (key == GLFW_KEY_SPACE && isPress) {
      gState->toggleSelection();
    } else if (key == GLFW_KEY_C && action == GLFW_PRESS) {
      gState->tryCopy();

    } else if (key == GLFW_KEY_EQUAL && isPress) {
      gState->increaseFontSize(1);
    } else if (key == GLFW_KEY_MINUS && isPress) {
      gState->increaseFontSize(-1);
    } else if ((key == GLFW_KEY_V || key == GLFW_KEY_Y) && isPress) {
      gState->tryPaste();
    } else {
      if (!isPress)
        return;
      if (key == GLFW_KEY_A && action == GLFW_PRESS)
        cursor->jumpStart();
      else if (key == GLFW_KEY_F && isPress)
        cursor->moveRight();
      else if (key == GLFW_KEY_D && isPress)
        cursor->removeBeforeCursor();
      else if (key == GLFW_KEY_E && isPress)
        cursor->jumpEnd();
      else if (key == GLFW_KEY_B && isPress)
        cursor->moveLeft();
      else if (key == GLFW_KEY_P && isPress)
        cursor->moveUp();
      else if (key == GLFW_KEY_N && isPress)
        cursor->moveDown();
      else if (key == GLFW_KEY_5 && isPress) {
        auto &state = *gState;
        if (state.hasHighlighting)
          cursor->jumpMatching(state.highlighter.language.stringCharacters,
                               state.highlighter.language.escapeChar);
        else {
          cursor->jumpMatching(U"\"", '\\');
        }
      }
      gState->renderCoords();
    }
  } else {
    if (isPress && key == GLFW_KEY_RIGHT)
      cursor->moveRight();
    if (isPress && key == GLFW_KEY_LEFT)
      cursor->moveLeft();
    if (isPress && key == GLFW_KEY_UP)
      cursor->moveUp();
    if (isPress && key == GLFW_KEY_DOWN)
      cursor->moveDown();
    if (isPress && key == GLFW_KEY_ENTER) {
      if (gState->mode != 0 || gState->cursor->isFolder) {
        gState->inform(true, shift_pressed, ctrl_pressed);
        return;
      } else
        cursor->append('\n');
    }
    if (isPress && key == GLFW_KEY_TAB) {
      if (gState->mode != 0)
        gState->provideComplete(shift_pressed);
      else {
        if (shift_pressed) {
          gState->fold();
        } else {
          bool useSpaces = gState->provider.useSpaces;
          auto am = gState->provider.tabWidth;
          if (useSpaces)
            cursor->append(std::string(am, ' '));
          else
            cursor->append('\t');
        }
      }
    }
    if (isPress && key == GLFW_KEY_BACKSPACE) {
      if (alt_pressed) {
        cursor->deleteWordBackwards();
      } else {
        cursor->removeOne();
      }
    }
    if(gState->provider.enableAccents)
      gState->accentManager.processEvent(window, key, scancode, action, mods);
    if (isPress)
      gState->renderCoords();
  }
}

int window_func(Window *instance) {
  float &WIDTH = instance->midState.WIDTH;
  float &HEIGHT = instance->midState.HEIGHT;
  auto &state = *instance->state;
  auto *gState = instance->state;
  Shader &selection_shader = *instance->shaders["selection"];
  Shader &text_shader = *instance->shaders["text"];
  Shader &cursor_shader = *instance->shaders["cursor"];
  FontAtlas &atlas = *instance->fontAtlas;

  int &maxRenderWidth = instance->midState.maxRenderWidth;
  int &fontSize = instance->midState.fontSize;
  auto *window = instance->window;
  do {
    if (glfwWindowShouldClose(window)) {
      if (state.exitFlag)
        break;
      auto *edited = state.hasEditedBuffer();
      if (!edited) {
        break;
      }

      gState->status =
          create(edited->path.length() ? edited->path : "New File") +
          U" edited, press ESC again to exit";
      if (!gState->exitLoop)
        state.invalidateCache();
      gState->exitLoop = true;
    }
    if (state.checkCommandRun())
      state.invalidateCache();
    if (state.cacheValid) {
      return 1;
    }
    bool changed = false;
    if (HEIGHT != state.HEIGHT || WIDTH != state.WIDTH ||
        fontSize != state.fontSize) {
      WIDTH = state.WIDTH;
      fontSize = state.fontSize;
      state.highlighter.wasCached = false;
      HEIGHT = state.HEIGHT;
      changed = true;
    }
    const auto renderHeight = HEIGHT - state.atlas->atlas_height - 6;
    Cursor *cursor = state.cursor;
    if (state.vim && cursor->bind == nullptr && cursor->x > 0 &&
        cursor->x >= cursor->getCurrentLineLength() &&
        state.vim->getMode() == VimMode::NORMAL)
      cursor->x = cursor->getCurrentLineLength() == 0
                      ? 0
                      : cursor->getCurrentLineLength() - 1;
    float toOffset = atlas.atlas_height;
    bool isSearchMode = state.mode == 2 || state.mode == 6 || state.mode == 7 ||
                        state.mode == 32;
    if (state.lineWrapping)
      cursor->setBoundsDirect(cursor->getMaxLinesWrapped(
          atlas, -maxRenderWidth, -(int32_t)(HEIGHT / 2) + 4 + toOffset,
          maxRenderWidth, toOffset, renderHeight));
    else
      cursor->setBounds(renderHeight, toOffset);
    if (maxRenderWidth != 0) {
      cursor->getContent(&atlas, maxRenderWidth, true, state.lineWrapping);
    }

    auto be_color = state.provider.colors.background_color;
    auto status_color = state.provider.colors.status_color;
    glClearColor(be_color.x, be_color.y, be_color.z, be_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    std::vector<RenderChar> entries;
    Utf8String::const_iterator c;
    std::string::const_iterator cc;
    float xpos = (-(int32_t)WIDTH / 2) + 10;
    float ypos = -(float)HEIGHT / 2;
    int start = cursor->skip;
    float linesAdvance = 0;
    int maxLines = cursor->skip + cursor->maxLines <= cursor->lines.size()
                       ? cursor->skip + cursor->maxLines
                       : cursor->lines.size();

    auto maxLineAdvance = atlas.getAdvance(std::to_string(maxLines));

    if (state.provider.highlightLine == "full" ||
        (state.showLineNumbers && state.provider.highlightLine == "small")) {
      if (state.provider.relativeLineNumbers) {
        int biggestLine = 0;
        auto endLines = maxLines - (cursor->y - cursor->skip);
        for (int i = (cursor->y - cursor->skip) * -1; i < endLines; i++) {
          int v = i == 0 ? cursor->y + 1 + cursor->getFoldOffset(cursor->y)
                         : (i < 0 ? i * -1 : i);
          if (v > biggestLine)
            biggestLine = v;
        }
        maxLineAdvance = atlas.getAdvance(std::to_string(biggestLine));
      }
      selection_shader.use();
      glBindVertexArray(state.highlight_vao);
      auto color = state.provider.colors.highlight_color;
      selection_shader.set4f("selection_color", color.x, color.y, color.z,
                             color.w);
      selection_shader.set2f("resolution", (float)WIDTH, (float)HEIGHT);
      glBindBuffer(GL_ARRAY_BUFFER, state.highlight_vbo);
      SelectionEntry entry;
      float hWidth =
          (state.showLineNumbers && state.provider.highlightLine == "small")
              ? maxLineAdvance
              : (((int32_t)WIDTH / 2) * 2) - 20;

      if (state.lineWrapping) {
        auto out = cursor->getPosLineWrapped(
            atlas, -maxRenderWidth, -(int32_t)(HEIGHT / 2) + 4 + toOffset,
            maxRenderWidth, toOffset, cursor->x, cursor->y);
        entry = {vec2f((-(int32_t)WIDTH / 2) + 10, -out.second),
                 vec2f(hWidth, toOffset)};
      } else {
        entry = {vec2f((-(int32_t)WIDTH / 2) + 10,
                       (float)HEIGHT / 2 - 5 - toOffset -
                           ((cursor->y - cursor->skip) * toOffset)),
                 vec2f(hWidth, toOffset)};
      }

      glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(SelectionEntry), &entry);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 6, 1);
    }
    text_shader.use();
    text_shader.set2f("resolution", (float)WIDTH, (float)HEIGHT);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(state.vao);
    glBindTexture(GL_TEXTURE_2D, atlas.texture_id);
    glBindBuffer(GL_ARRAY_BUFFER, state.vbo);
    if (state.showLineNumbers) {
      if (state.lineWrapping) {
        int biggestLine = maxLines;
        bool relative = state.provider.relativeLineNumbers;
        if (relative) {
          biggestLine = 0;
          auto endLines = maxLines - (cursor->y - cursor->skip);
          for (int i = (cursor->y - cursor->skip) * -1; i < endLines; i++) {
            int v = i == 0 ? cursor->y + 1 + cursor->getFoldOffset(cursor->y)
                           : (i < 0 ? i * -1 : i);
            if (v > biggestLine)
              biggestLine = v;
          }
        }
        auto maxLineAdvance = atlas.getAdvance(std::to_string(biggestLine));
        linesAdvance = maxLineAdvance;
        auto endLines = maxLines;
        if (relative)
          endLines = (cursor->maxLines - (cursor->y - cursor->skip));
        auto heightRemaining = renderHeight;
        int off = relative ? (cursor->y - cursor->skip) : 0;
        int s = start;
        for (int i = relative ? ((cursor->y - cursor->skip) * -1) : start;
             i < endLines; i++) {

          std::string value =
              s >= cursor->lines.size() ? "~"
              : relative
                  ? std::to_string(i == 0 ? cursor->y + 1 +
                                                cursor->getFoldOffset(cursor->y)
                                          : (i < 0 ? i * -1 : i))
                  : std::to_string(i + cursor->getFoldOffset(i) + 1);
          auto tAdvance = atlas.getAdvance(value);
          xpos += maxLineAdvance - tAdvance;
          auto out = cursor->getPosLineWrapped(atlas, -maxRenderWidth,
                                               -(int32_t)(HEIGHT / 2),
                                               maxRenderWidth, toOffset, 0, s);
          for (cc = value.begin(); cc != value.end(); cc++) {
            entries.push_back(
                atlas.render(*cc, xpos, out.second,
                             state.provider.colors.line_number_color));
            auto advance = atlas.getAdvance(*cc);
            xpos += advance;
          }
          xpos = -(int32_t)WIDTH / 2 + 10;
          s++;
          if (heightRemaining <= 0)
            break;
        }
      } else {
        int biggestLine = maxLines;
        bool relative = state.provider.relativeLineNumbers;
        if (relative) {
          biggestLine = 0;
          auto endLines = (cursor->maxLines - (cursor->y - cursor->skip));
          for (int i = (cursor->y - cursor->skip) * -1; i < endLines; i++) {
            int v = i == 0 ? cursor->y + 1 + cursor->getFoldOffset(cursor->y)
                           : (i < 0 ? i * -1 : i);
            if (v > biggestLine)
              biggestLine = v;
          }
        }
        auto maxLineAdvance = atlas.getAdvance(std::to_string(biggestLine));
        linesAdvance = maxLineAdvance;
        auto endLines = maxLines;
        if (relative)
          endLines = (cursor->maxLines - (cursor->y - cursor->skip));
        int s = start;
        for (int i = relative ? ((cursor->y - cursor->skip) * -1) : start;
             i < endLines; i++) {
          std::string value =
              s >= cursor->lines.size() ? "~"
              : relative
                  ? std::to_string(i == 0 ? cursor->y + 1 +
                                                cursor->getFoldOffset(cursor->y)
                                          : (i < 0 ? i * -1 : i))
                  : std::to_string(i + cursor->getFoldOffset(i) + 1);
          auto tAdvance = atlas.getAdvance(value);
          xpos += maxLineAdvance - tAdvance;
          for (cc = value.begin(); cc != value.end(); cc++) {
            entries.push_back(atlas.render(
                *cc, xpos, ypos, state.provider.colors.line_number_color));
            auto advance = atlas.getAdvance(*cc);
            xpos += advance;
          }
          s++;
          xpos = -(float)WIDTH / 2 + 10;
          ypos += toOffset;
        }
      }
    }
    maxRenderWidth = (WIDTH / 2) - 20 - linesAdvance;
    auto skipNow = cursor->skip;
    auto *allLines =
        cursor->getContent(&atlas, maxRenderWidth, false, state.lineWrapping);
    state.reHighlight();
    ypos = (-(HEIGHT / 2));
    xpos = -(int32_t)WIDTH / 2 + 20 + linesAdvance;
    cursor->setRenderStart(20 + linesAdvance, 15);
    Vec4f color = state.provider.colors.default_color;
    Vec4f fold_color = state.provider.colors.fold_color;
    if (state.hasHighlighting) {
      auto highlighter = state.highlighter;
      int lineOffset = cursor->skip;
      auto *colored = state.highlighter.get();

      if (lineOffset > 0) {
        for (int t = lineOffset; t >= 0; t--) {
          if (highlighter.lineIndex.count(t)) {
            auto entry = highlighter.lineIndex[t];
            auto start = colored->begin();
            std::advance(start, entry.second - 1);
            color = start->second;

            break;
          }
        }
      }
      int cOffset = cursor->getTotalOffset();
      int cxOffset = cursor->xOffset;
      auto heightRemaining = renderHeight;
      //        std::cout << cxOffset << ":" << lineOffset << "\n";

      for (size_t x = 0; x < allLines->size(); x++) {
        const bool isFold = cursor->foldEntries.count(x + cursor->skip);
        auto content = (*allLines)[x].second;
        auto hasColorIndex = highlighter.lineIndex.count(x + lineOffset);
        if (content.length())
          cOffset += cxOffset;
        else
          cOffset += (*allLines)[x].first;
        if (cxOffset > 0) {
          if (hasColorIndex) {
            auto entry = highlighter.lineIndex[x + lineOffset];
            auto start = colored->begin();
            std::advance(start, entry.first);
            auto end = colored->begin();
            std::advance(end, entry.second);
            for (std::map<int, Vec4f>::iterator it = start; it != end; ++it) {
              int xx = it->first;
              if (xx >= cOffset)
                break;
              color = it->second;
            }
          }
        }
        if ((*colored).count(cOffset)) {
          color = (*colored)[cOffset];
        }
        int charAdvance = 0;
        for (c = content.begin(); c != content.end(); c++) {
          if ((*colored).count(cOffset)) {
            color = (*colored)[cOffset];
          }

          cOffset++;
          charAdvance++;
          if (*c != '\t')
            entries.push_back(
                atlas.render(*c, xpos, ypos, isFold ? fold_color : color));
          xpos += atlas.getAdvance(*c);
          if (state.lineWrapping) {
            if (xpos > (maxRenderWidth + atlas.getAdvance(*c))) {
              if ((*colored).count(cOffset)) {
                color = (*colored)[cOffset];
              }
              xpos = -maxRenderWidth;
              ypos += toOffset;
              heightRemaining -= toOffset;
              if (heightRemaining <= 0)
                break;
            }
            continue;
          }
          if (xpos > (maxRenderWidth + atlas.getAdvance(*c)) &&
              c != content.end()) {
            int remaining = content.length() - (charAdvance);

            if (remaining > 0) {
              if (hasColorIndex) {
                auto entry = highlighter.lineIndex[x + lineOffset];
                auto start = colored->begin();
                std::advance(start, entry.first);
                auto end = colored->begin();
                std::advance(end, entry.second);
                for (std::map<int, Vec4f>::iterator it = start; it != end;
                     ++it) {
                  int xx = it->first;
                  if (xx > cOffset + remaining)
                    break;
                  if (xx >= cOffset)
                    color = it->second;
                }
              }
              cOffset += remaining;
            }

            break;
          }
        }
        if (state.lineWrapping && heightRemaining <= 0)
          break;

        if (x < allLines->size() - 1) {
          if ((*colored).count(cOffset)) {
            color = (*colored)[cOffset];
          }
          cOffset++;
          xpos = -maxRenderWidth;
          ypos += toOffset;
        }
      }
    } else {
      auto heightRemaining = renderHeight;

      for (size_t x = 0; x < allLines->size(); x++) {
        const bool isFold = cursor->foldEntries.count(x + cursor->skip);
        auto content = (*allLines)[x].second;
        for (c = content.begin(); c != content.end(); c++) {
          if (*c != '\t')
            entries.push_back(
                atlas.render(*c, xpos, ypos, isFold ? fold_color : color));
          xpos += atlas.getAdvance(*c);
          if (state.lineWrapping) {
            if (xpos > (maxRenderWidth + atlas.getAdvance(*c))) {
              xpos = -maxRenderWidth;
              ypos += toOffset;
              heightRemaining -= toOffset;
              if (heightRemaining <= 0)
                break;
            }
          }

          if (xpos > maxRenderWidth + atlas.getAdvance(*c)) {
            break;
          }
        }

        if (state.lineWrapping && heightRemaining <= 0)
          break;
        if (x < allLines->size() - 1) {
          xpos = -maxRenderWidth;
          ypos += toOffset;
        }
      }
    }
    xpos = (-(int32_t)WIDTH / 2) + 15;
    ypos = (float)HEIGHT / 2 - toOffset - 10;
    Utf8String status = state.status;
    for (c = status.begin(); c != status.end(); c++) {
      entries.push_back(atlas.render(*c, xpos, ypos, status_color));
      xpos += atlas.getAdvance(*c);
    }
    float statusAdvance = atlas.getAdvance(state.status);
    if (state.mode != 0 && state.mode != 32 ||
        (state.vim && state.vim->isCommandBufferActive())) {
      // draw minibuffer
      xpos = (-(int32_t)WIDTH / 2) + 20 + statusAdvance;
      ypos = (float)HEIGHT / 2 - toOffset - 10;
      Utf8String status = state.miniBuf;
      for (c = status.begin(); c != status.end(); c++) {
        entries.push_back(atlas.render(*c, xpos, ypos,
                                       state.provider.colors.minibuffer_color));
        xpos += atlas.getAdvance(*c);
      }

    } else {
      auto tabInfo = state.getTabInfo();
      xpos = ((int32_t)WIDTH / 2) - atlas.getAdvance(tabInfo);
      ypos = (float)HEIGHT / 2 - toOffset - 10;
      for (c = tabInfo.begin(); c != tabInfo.end(); c++) {
        entries.push_back(atlas.render(*c, xpos, ypos, status_color));
        xpos += atlas.getAdvance(*c);
      }
    }

    glBindBuffer(GL_ARRAY_BUFFER, state.vbo);
    glBufferSubData(
        GL_ARRAY_BUFFER, 0, sizeof(RenderChar) * entries.size(),
        &entries[0]); // be sure to use glBufferSubData and not glBufferData
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 6, (GLsizei)entries.size());
    if (state.focused) {
      cursor_shader.use();
      cursor_shader.set1f("cursor_width", 4);
      cursor_shader.set1f("cursor_height", toOffset);
      cursor_shader.set2f("resolution", (float)WIDTH, (float)HEIGHT);
      cursor_shader.set4f("cursor_color",
                          state.provider.colors.cursor_color_standard);
      if (state.mode != 0 && state.mode != 32 ||
          (state.vim && state.vim->isCommandBufferActive())) {
        // use cursor for minibuffer
        float cursorX = -(int32_t)(WIDTH / 2) + 15 +
                        (atlas.getAdvance(cursor->getCurrentAdvance())) + 5 +
                        statusAdvance;
        float cursorY = (float)HEIGHT / 2 - 10;
        cursor_shader.set2f("cursor_pos", cursorX, -cursorY);

        glBindVertexArray(state.vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
      }

      if ((isSearchMode || state.mode == 0) &&
          (!state.vim || !state.vim->isCommandBufferActive())) {
        if (state.lineWrapping) {
          auto out = cursor->getPosLineWrapped(
              atlas, -maxRenderWidth, -(int32_t)(HEIGHT / 2) + 4 + toOffset,
              maxRenderWidth, toOffset, cursor->x, cursor->y);
          if (state.vim) {
            if (state.vim->getMode() == VimMode::INSERT) {
              cursor_shader.set1f("cursor_width", 4);

            } else {
              cursor_shader.set1f("cursor_width", atlas.getAdvance(' '));
              cursor_shader.set4f("cursor_color",
                                  state.provider.colors.cursor_color_vim);
            }
          }
          cursor_shader.set2f("cursor_pos", out.first, -out.second);
        } else {
          auto cAdvance =
              (atlas.getAdvance(cursor->getCurrentAdvance(isSearchMode)));
          float cursorX = -(int32_t)(WIDTH / 2) + 15 + cAdvance + linesAdvance +
                          4 - cursor->xSkip;
          if (cursorX > WIDTH / 2)
            cursorX = (WIDTH / 2) - 3;
          float cursorY = -(int32_t)(HEIGHT / 2) + 4 +
                          (toOffset * ((cursor->y - cursor->skip) + 1));
          if (state.vim) {
            if (state.vim->getMode() == VimMode::INSERT) {
              cursor_shader.set1f("cursor_width", 4);

            } else {
              cursor_shader.set1f(
                  "cursor_width",
                  atlas.getAdvance(cursor->getCurrentLineLength() == 0
                                       ? ' '
                                       : cursor->getCurrentChar()));
              cursor_shader.set4f("cursor_color",
                                  state.provider.colors.cursor_color_vim);
            }
          }
          cursor_shader.set2f("cursor_pos", cursorX, -cursorY);
        }

        glBindVertexArray(state.vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
      }
    }
    if (cursor->selection.active) {
      std::vector<SelectionEntry> selectionBoundaries;
      if (cursor->selection.getYSmaller() < cursor->skip &&
          cursor->selection.getYBigger() > cursor->skip + cursor->maxLines) {
        // select everything
      } else {
        maxRenderWidth += atlas.getAdvance(U" ");
        int yStart = cursor->selection.getYStart();
        int yEnd = cursor->selection.getYEnd();
        if (cursor->selection.yStart == cursor->selection.yEnd) {
          if (cursor->selection.xStart != cursor->selection.xEnd) {
            int smallerX = cursor->selection.getXSmaller();
            if (smallerX >= cursor->xOffset) {

              float renderDistance = atlas.getAdvance(
                  (*allLines)[yEnd - cursor->skip].second.substr(
                      0, smallerX - cursor->xOffset));
              float renderDistanceBigger = atlas.getAdvance(
                  (*allLines)[yEnd - cursor->skip].second.substr(
                      0, cursor->selection.getXBigger() - cursor->xOffset));
              if (renderDistance < maxRenderWidth * 2) {
                float start = ((float)HEIGHT / 2) - 5 -
                              (toOffset * ((yEnd - cursor->skip) + 1));
                selectionBoundaries.push_back(
                    {vec2f(-(int32_t)WIDTH / 2 + 20 + linesAdvance +
                               renderDistance,
                           start),
                     vec2f(renderDistanceBigger - renderDistance, toOffset)});
              } else {
                float renderDistanceBigger = atlas.getAdvance(
                    (*allLines)[yEnd - cursor->skip].second.substr(
                        0, cursor->selection.getXBigger() - cursor->xOffset));
                float start = ((float)HEIGHT / 2) - 5 -
                              (toOffset * ((yEnd - cursor->skip) + 1));
                selectionBoundaries.push_back(
                    {vec2f(-(int32_t)WIDTH / 2 + 20 + linesAdvance +
                               (maxRenderWidth - renderDistance),
                           start),
                     vec2f(maxRenderWidth > renderDistanceBigger
                               ? maxRenderWidth
                               : renderDistanceBigger,
                           toOffset)});
              }
            } else {
              float renderDistanceBigger = atlas.getAdvance(
                  (*allLines)[yEnd - cursor->skip].second.substr(
                      0, cursor->selection.getXBigger() - cursor->xOffset));
              float start = ((float)HEIGHT / 2) - 5 -
                            (toOffset * ((yEnd - cursor->skip) + 1));
              selectionBoundaries.push_back(
                  {vec2f(-(int32_t)WIDTH / 2 + 20 + linesAdvance, start),
                   vec2f(renderDistanceBigger > maxRenderWidth * 2
                             ? maxRenderWidth * 2
                             : renderDistanceBigger,
                         toOffset)});
            }
          }
        } else {
          if (yStart >= cursor->skip &&
              yStart <= (cursor->skip + cursor->maxLines) - 1) {
            int yEffective = cursor->selection.getYStart() - cursor->skip;
            int xStart = cursor->selection.getXStart();
            float renderDistance =
                atlas.getAdvance((*allLines)[yEffective].second.substr(
                    0, xStart - cursor->xOffset));
            if (xStart >= cursor->xOffset) {

              if (renderDistance < (maxRenderWidth * 2)) {
                if (yStart < yEnd) {

                  float start =
                      ((float)HEIGHT / 2) - 5 - (toOffset * (yEffective + 1));
                  selectionBoundaries.push_back(
                      {vec2f(-(int32_t)WIDTH / 2 + 20 + linesAdvance +
                                 renderDistance,
                             start),
                       vec2f((maxRenderWidth * 2) - renderDistance, toOffset)});
                } else {
                  float start =
                      ((float)HEIGHT / 2) - 5 - (toOffset * (yEffective + 1));
                  selectionBoundaries.push_back(
                      {vec2f(-(int32_t)WIDTH / 2 + 20 + linesAdvance, start),
                       vec2f(renderDistance, toOffset)});
                }
              } else {
                float start =
                    ((float)HEIGHT / 2) - 5 - (toOffset * (yEffective + 1));
                selectionBoundaries.push_back(
                    {vec2f(-(int32_t)WIDTH / 2 + 20 + linesAdvance, start),
                     vec2f((maxRenderWidth * 2), toOffset)});
              }
            }
          }
          if (yEnd >= cursor->skip && yEnd <= cursor->skip + cursor->maxLines) {
            int yEffective = cursor->selection.getYEnd() - cursor->skip;
            int xStart = cursor->selection.getXEnd();
            if (xStart >= cursor->xOffset) {
              float renderDistance =
                  atlas.getAdvance((*allLines)[yEffective].second.substr(
                      0, xStart - cursor->xOffset));
              if (renderDistance < (maxRenderWidth * 2)) {
                if (yEnd < yStart) {
                  float start =
                      ((float)HEIGHT / 2) - 5 - (toOffset * (yEffective + 1));
                  selectionBoundaries.push_back(
                      {vec2f(-(int32_t)WIDTH / 2 + 20 + linesAdvance +
                                 renderDistance,
                             start),
                       vec2f((maxRenderWidth * 2) - renderDistance, toOffset)});
                } else {
                  float start =
                      ((float)HEIGHT / 2) - 5 - (toOffset * (yEffective + 1));
                  selectionBoundaries.push_back(
                      {vec2f(-(int32_t)WIDTH / 2 + 20 + linesAdvance, start),
                       vec2f(renderDistance, toOffset)});
                }
              } else {

                float start =
                    ((float)HEIGHT / 2) - 5 - (toOffset * (yEffective + 1));
                selectionBoundaries.push_back(
                    {vec2f(-(int32_t)WIDTH / 2 + 20 + linesAdvance, start),
                     vec2f((maxRenderWidth * 2), toOffset)});
              }
            }
          }
          bool found = false;
          int offset = 0;
          int count = 0;
          for (int i = cursor->selection.getYSmaller();
               i < cursor->selection.getYBigger() - 1; i++) {
            if (i >= (cursor->skip + cursor->maxLines) - 1)
              break;
            if (i >= cursor->skip - 1) {
              if (!found) {
                found = true;
                offset = i - cursor->skip;
              }
              count++;
            }
          }
          if (found) {
            float start = (float)HEIGHT / 2 - 5 - (toOffset * (offset + 1));
            selectionBoundaries.push_back(
                {vec2f((-(int32_t)WIDTH / 2) + 20 + linesAdvance, start),
                 vec2f(maxRenderWidth * 2, -(count * toOffset))});
          }
        }
      }
      if (selectionBoundaries.size()) {
        selection_shader.use();
        glBindVertexArray(state.sel_vao);
        auto color = state.provider.colors.selection_color;
        selection_shader.set4f("selection_color", color.x, color.y, color.z,
                               color.w);
        selection_shader.set2f("resolution", (float)WIDTH, (float)HEIGHT);
        glBindBuffer(GL_ARRAY_BUFFER, state.sel_vbo);

        glBufferSubData(GL_ARRAY_BUFFER, 0,
                        sizeof(SelectionEntry) * selectionBoundaries.size(),
                        &selectionBoundaries[0]);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 6,
                              (GLsizei)selectionBoundaries.size());
      }
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glfwSwapBuffers(window);
    state.accentManager.interrupt();
    state.cacheValid = true;
    return 1;
  } while (0);
  return 0;
};
Window *create_window(std::string path, bool isFirst = false) {
  Window *instance = new Window();
  instance->state = new State(1280, 720, 28);
  const std::string window_name = (path.length() ? path : "New File");
  State &state = *instance->state;
  GLFWwindow *window = glfwCreateWindow(state.WIDTH, state.HEIGHT,
                                        window_name.c_str(), nullptr, nullptr);
  if (!window) {
    delete instance;
    return nullptr;
  }
#ifdef LEDIT_WIN_MAIN
  HICON hIcon =
      (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1),
                       IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
  if (hIcon) {

    IconData iconData = ExtractIconData(hIcon);
    GLFWimage image;
    image.width = iconData.width;
    image.height = iconData.height;
    image.pixels = (unsigned char *)iconData.pixels.data();

    glfwSetWindowIcon(window, 1, &image);
    DestroyIcon(hIcon);
  }
#endif
  instance->window = window;
  if (state.provider.vim_emulation)
    state.registerVim();
  state.window = window;
  state.addCursor(path);

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetKeyCallback(window, key_callback);
  glfwSetCharCallback(window, character_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetWindowFocusCallback(window, window_focus_callback);
  glfwSetDropCallback(window, drop_callback);
  GLFWcursor *mouseCursor = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
  glfwSetCursor(window, mouseCursor);
#ifdef __APPLE__
  if (state.provider.titleBarColorSet) {
    auto e = state.provider.titleBarColor;
    glfwSetWindowTitlebarColor(window, 255 * e.x, 255 * e.y, 255 * e.z,
                               255 * e.w);
  }
#endif
  if (isFirst) {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      std::cout << "Failed to initialize GLAD" << std::endl;
      delete instance;
      return nullptr;
    }
  }
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  state.init();
  Shader *text_shader = new Shader(text_shader_vert, text_shader_frag, {});
  text_shader->use();
  instance->shaders["text"] = text_shader;
  Shader *cursor_shader =
      new Shader(cursor_shader_vert, cursor_shader_frag, {camera_shader_vert});
  instance->shaders["cursor"] = cursor_shader;
  Shader *selection_shader =
      new Shader(selection_shader_vert, selection_shader_frag, {});
  instance->shaders["selection"] = selection_shader;

  instance->fontAtlas = new FontAtlas(state.provider.fontPath, state.fontSize);
  auto &atlas = *instance->fontAtlas;
  for (auto &path : state.provider.extraFonts) {
    atlas.readFont(path, state.fontSize);
  }
  atlas.tabWidth = state.provider.tabWidth;
  state.atlas = &atlas;
  if (atlas.errors.size()) {
    state.status += U" " + atlas.errors[0];
  }
  float xscale, yscale;
  glfwGetWindowContentScale(window, &xscale, &yscale);
  state.WIDTH *= xscale;
  state.HEIGHT *= yscale;

  return instance;
}
void add_window(std::string p) {
  Window *first = create_window(p);
  if (!first)
    return;
  g_windows->addAndActivate(first);
}
#ifdef LEDIT_WIN_MAIN
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PWSTR pCmdLine, int nCmdShow) {
  LPWSTR *szArglist;
  int nArgs;
  int i;

  szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
  std::string initialPath = nArgs >= 2 ? winStrToStr(szArglist[1]) : "";
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  WindowManager windowManager;
  g_windows = &windowManager;
  Window *first = create_window(initialPath, true);
  if (!first)
    return 1;
  windowManager.addAndActivate(first);
  Window *lastActive = nullptr;
  while (true) {
    std::vector<Window *> toRemove;
    for (auto entry : windowManager.windows) {
      glfwMakeContextCurrent(entry.second->window);
      auto result = window_func(entry.second);
      if (result == 0)
        toRemove.push_back(entry.second);
    }
    glfwMakeContextCurrent(nullptr);
    for (auto c : toRemove) {
      windowManager.removeWindow(c->window);
      delete c;
    }
    if (!windowManager.windows.size())
      break;
    glfwWaitEvents();
  }
  glfwTerminate();
  return 0;
}
#else
int main(int argc, char **argv) {
  std::string initialPath = argc >= 2 ? std::string(argv[1]) : "";

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  WindowManager windowManager;
  g_windows = &windowManager;
  Window *first = create_window(initialPath, true);
  if (!first)
    return 1;
  windowManager.addAndActivate(first);
  Window *lastActive = nullptr;
  while (true) {
    std::vector<Window *> toRemove;
    for (auto entry : windowManager.windows) {
      glfwMakeContextCurrent(entry.second->window);
      auto result = window_func(entry.second);
      if (result == 0)
        toRemove.push_back(entry.second);
    }
    glfwMakeContextCurrent(nullptr);
    for (auto c : toRemove) {
      windowManager.removeWindow(c->window);
      delete c;
    }
    if (!windowManager.windows.size())
      break;
    glfwWaitEvents();
  }
  glfwTerminate();
  return 0;
}
#endif