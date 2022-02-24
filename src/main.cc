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
#include "state.h"
#include "shader.h"
#include "font_atlas.h"
#include "cursor.h"
#include "shaders.h"
#include "highlighting.h"
#include "languages.h"
State* gState = nullptr;
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    if(gState != nullptr) {
      gState->WIDTH = (float)width;
      gState->HEIGHT = (float)height;
    }

}
void window_focus_callback(GLFWwindow* window, int focused) {
  gState->focused = focused;
  if(focused) {
    gState->checkChanged();
  }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
      double xpos, ypos;
      glfwGetCursorPos(window, &xpos, &ypos);
    float xscale, yscale;
    glfwGetWindowContentScale(window, &xscale, &yscale);
      gState->cursor->setPosFromMouse((float)xpos * xscale, (float) ypos * yscale, gState->atlas);


    }
}

void character_callback(GLFWwindow* window, unsigned int codepoint)
{
  if(gState == nullptr)
    return;
  gState->exitFlag = false;
#ifdef _WIN32
  bool ctrl_pressed = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
  if(ctrl_pressed)
    return;
#endif
  bool alt_pressed = glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS;
  if(alt_pressed) {
    if(glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
      gState->cursor->advanceWord();
      return;
    }
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
      gState->tryCopy();
      return;
    }
    if(glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
      gState->cursor->advanceWordBackwards();
      return;
    }
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
      gState->cursor->deleteWord();
      return;
    }
  }
  gState->cursor->append((char16_t) codepoint);
  gState->lastStroke = glfwGetTime();
  gState->renderCoords();
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if(gState == nullptr) return;
  if(key == GLFW_KEY_ESCAPE) {
    if(action == GLFW_PRESS) {
    if(gState->cursor->selection.active) {
      gState->cursor->selection.stop();
      return;
    }
    if(gState->mode != 0) {
      gState->inform(false, false);
    } else {
       CursorEntry* edited = gState->hasEditedBuffer();
       if(gState->exitFlag || edited == nullptr) {
         glfwSetWindowShouldClose(window, true);
       } else {
         gState->exitFlag = true;
         gState->status = create(edited->path) + u" edited, press ESC again to exit";
       }
    }
    }
    return;

  }
  gState->exitFlag = false;
  bool ctrl_pressed = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
  gState->ctrlPressed = ctrl_pressed;
  bool shift_pressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
  bool x_pressed = glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS;
  bool alt_pressed = glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS;
  Cursor* cursor = gState->cursor;
  bool isPress = action == GLFW_PRESS || action == GLFW_REPEAT;
#ifdef __linux__
  if(alt_pressed) {
    if(key == GLFW_KEY_F && isPress) {
      gState->cursor->advanceWord();
      return;
    }
    if(key == GLFW_KEY_W && action == GLFW_PRESS) {
      gState->tryCopy();
      return;
    }
    if(key ==  GLFW_KEY_B && isPress) {
      gState->cursor->advanceWordBackwards();
      return;
    }
    if(key == GLFW_KEY_D && isPress) {
      gState->cursor->deleteWord();
      return;
    }
  }
#endif
  if(ctrl_pressed) {

    if(x_pressed) {
      if(action == GLFW_PRESS && key == GLFW_KEY_S) {
        gState->save();
      }
      if(action == GLFW_PRESS && key == GLFW_KEY_SLASH) {
        gState->tryComment();
      }
      if(action == GLFW_PRESS && key == GLFW_KEY_M) {
        gState->switchMode();
      }
      if(action == GLFW_PRESS && key == GLFW_KEY_L) {
        gState->showLineNumbers = !gState->showLineNumbers;
      }
      if(action == GLFW_PRESS && key == GLFW_KEY_H) {
        gState->highlightLine = !gState->highlightLine;
      }
      if(action == GLFW_PRESS && key == GLFW_KEY_O) {
        gState->open();
      }
      if(action == GLFW_PRESS && key == GLFW_KEY_0) {
        gState->changeFont();
      }
      if(action == GLFW_PRESS && key == GLFW_KEY_K) {
        gState->switchBuffer();
      }
      if(action == GLFW_PRESS && key == GLFW_KEY_N) {
        gState->saveNew();
      }
      if(action == GLFW_PRESS && key == GLFW_KEY_G) {
        gState->gotoLine();
      }
      if(action == GLFW_PRESS && key == GLFW_KEY_W) {
        gState->deleteActive();
      }
      if(action == GLFW_PRESS && key == GLFW_KEY_A) {
        cursor->gotoLine(1);
        gState->renderCoords();
      }
      if(action == GLFW_PRESS && key == GLFW_KEY_E) {
        cursor->gotoLine(cursor->lines.size());
        gState->renderCoords();
      }
      return;
    }
     if (shift_pressed) {
    if(key == GLFW_KEY_P && isPress) {
       gState->cursor->moveLine(-1);
    } else  if(key == GLFW_KEY_N && isPress) {
       gState->cursor->moveLine(1);
    }
      gState->renderCoords();
      return;
    }
     if (key == GLFW_KEY_S && action == GLFW_PRESS) {
       gState->search();
     } else if (key == GLFW_KEY_R && isPress) {
       gState->startReplace();
     } else if (key == GLFW_KEY_Z && isPress) {
       gState->undo();
     } else if (key == GLFW_KEY_W && isPress) {
       gState->cut();
     } else if (key == GLFW_KEY_SPACE && isPress) {
       gState->toggleSelection();
     } else if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        gState->tryCopy();

     }   else if (key == GLFW_KEY_EQUAL && isPress) {
            gState->increaseFontSize(2);
     }   else if (key == GLFW_KEY_MINUS && isPress) {
            gState->increaseFontSize(-2);
      } else if ((key == GLFW_KEY_V || key == GLFW_KEY_Y) && isPress) {
         gState->tryPaste();
     } else {
       if (!isPress)
         return;
       gState->lastStroke = glfwGetTime();
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
        gState->renderCoords();

      }
  } else {
    if(isPress && key == GLFW_KEY_RIGHT)
      cursor->moveRight();
    if(isPress && key == GLFW_KEY_LEFT)
      cursor->moveLeft();
    if (isPress && key == GLFW_KEY_UP)
      cursor->moveUp();
    if (isPress && key == GLFW_KEY_DOWN)
      cursor->moveDown();
    gState->lastStroke = glfwGetTime();
    if(isPress && key == GLFW_KEY_ENTER) {
      if(gState->mode != 0) {
        gState->inform(true, shift_pressed);
        return;
      }  else
        cursor->append('\n');
    }
    if(isPress && key == GLFW_KEY_TAB) {
      if(gState->mode != 0)
        gState->provideComplete(shift_pressed);
      else
        cursor->append(u"  ");
    }
    if(isPress && key == GLFW_KEY_BACKSPACE) {
      cursor->removeOne();
    }
    if(isPress)
      gState->renderCoords();
  }

}
int main(int argc, char** argv) {
#ifdef _WIN32
  ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif
    std::string x = argc >=2 ?std::string(argv[1]) : "";
    State state(1280, 720, x, 30);
    gState = &state;
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    if(state.provider.allowTransparency)
      glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, 1);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    const std::string window_name = "ledit: " + (x.length() ? x : "New File");
    GLFWwindow* window = glfwCreateWindow(state.WIDTH, state.HEIGHT, window_name.c_str(), nullptr, nullptr);
    if (window == NULL)
    {
        const char* description;
        int code = glfwGetError(&description);
        std::cout << "Failed to create GLFW window: " << description << std::endl;
        glfwTerminate();
        return -1;
    }
    state.window = window;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCharCallback(window, character_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetWindowFocusCallback(window, window_focus_callback);
    GLFWcursor* mouseCursor = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    glfwSetCursor(window, mouseCursor);





    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // OpenGL state
    // ------------
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    state.init();
    Shader text_shader(text_shader_vert, text_shader_frag, {});
    text_shader.use();
    Shader cursor_shader(cursor_shader_vert, cursor_shader_frag, {camera_shader_vert});
    Shader selection_shader(selection_shader_vert, selection_shader_frag, {});
    FontAtlas atlas(state.provider.fontPath, state.fontSize);
    state.atlas = &atlas;
    float xscale, yscale;
    glfwGetWindowContentScale(window, &xscale, &yscale);
    state.WIDTH *= xscale;
    state.HEIGHT *= yscale;
    int fontSize;
    float WIDTH;
    float HEIGHT;
    while (!glfwWindowShouldClose(window))
    {
//      glfwPollEvents();
      bool changed = false;
      if(HEIGHT != state.HEIGHT || WIDTH != state.WIDTH || fontSize != state.fontSize) {
         WIDTH = state.WIDTH;
         fontSize = state.fontSize;
         state.highlighter.wasCached = false;
         HEIGHT = state.HEIGHT;
         changed = true;
      }
      Cursor* cursor = state.cursor;
      float toOffset = atlas.atlas_height * 1.15;
      bool isSearchMode = state.mode == 2 || state.mode == 6 || state.mode == 7 || state.mode == 32;
      cursor->setBounds(HEIGHT - state.atlas->atlas_height - 6, toOffset);
      auto be_color = state.provider.colors.background_color;
      auto status_color = state.provider.colors.status_color;
      glClearColor(be_color.x, be_color.y, be_color.z, be_color.w);
      glClear(GL_COLOR_BUFFER_BIT);

      if(state.highlightLine) {
        selection_shader.use();
        glBindVertexArray(state.highlight_vao);
        auto color = state.provider.colors.highlight_color;
        selection_shader.set4f("selection_color", color.x,color.y, color.z, color.w);
        selection_shader.set2f("resolution", (float) WIDTH,(float) HEIGHT);
        glBindBuffer(GL_ARRAY_BUFFER, state.highlight_vbo);
        SelectionEntry entry{vec2f((-(int32_t)WIDTH/2) +10,  (float)HEIGHT/2 - 5 - toOffset - ((cursor->y - cursor->skip) * toOffset)), vec2f((((int32_t)WIDTH/2) * 2) - 20, toOffset)};
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(SelectionEntry), &entry);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 6, 1);



      }
      text_shader.use();
      text_shader.set2f("resolution", (float) WIDTH,(float) HEIGHT);
      glActiveTexture(GL_TEXTURE0);
      glBindVertexArray(state.vao);
      glBindTexture(GL_TEXTURE_2D, atlas.texture_id);
      glBindBuffer(GL_ARRAY_BUFFER, state.vbo);
      std::vector<RenderChar> entries;
      std::u16string::const_iterator c;
      std::string::const_iterator cc;
      float xpos =( -(int32_t)WIDTH/2) + 10;
      float ypos = -(float)HEIGHT/2;
      int start = cursor->skip;
      float linesAdvance = 0;
      int maxLines = cursor->skip + cursor->maxLines <= cursor->lines.size() ? cursor->skip + cursor->maxLines : cursor->lines.size();
      if(state.showLineNumbers) {
        int biggestLine = std::to_string(maxLines).length();
        auto maxLineAdvance = atlas.getAdvance(std::to_string(maxLines));
        for (int i = start; i < maxLines; i++) {
          std::string value = std::to_string(i+1);
          auto tAdvance = atlas.getAdvance(value);
          xpos += maxLineAdvance - tAdvance;
          linesAdvance = 0;
          for (cc = value.begin(); cc != value.end(); cc++) {
            entries.push_back(atlas.render(*cc, xpos,ypos, state.provider.colors.line_number_color));
            auto advance = atlas.getAdvance(*cc);
            xpos += advance;
            linesAdvance += advance;

          }
          xpos =  -(int32_t)WIDTH/2 + 10;
          ypos += toOffset;
        }
      }
      auto maxRenderWidth = (WIDTH /2) - 20 - linesAdvance;
      auto skipNow = cursor->skip;
      auto* allLines = cursor->getContent(&atlas, maxRenderWidth);
      state.reHighlight();
      ypos = (-(HEIGHT/2));
      xpos = -(int32_t)WIDTH/2 + 20 + linesAdvance;
      cursor->setRenderStart(20+linesAdvance, 15);
      Vec4f color = state.provider.colors.default_color;
      if(state.hasHighlighting) {
        auto highlighter = state.highlighter;
        int lineOffset = cursor->skip;
        auto* colored = state.highlighter.get();
        int cOffset = cursor->getTotalOffset();
        int cxOffset = cursor->xOffset;
//        std::cout << cxOffset << ":" << lineOffset << "\n";


        for(size_t x = 0; x < allLines->size(); x++) {
          auto content = (*allLines)[x].second;
          auto hasColorIndex = highlighter.lineIndex.count(x+lineOffset);
          if(content.length())
            cOffset += cxOffset;
          else
            cOffset += (*allLines)[x].first;
          if(cxOffset > 0) {
            if(hasColorIndex) {
              auto entry = highlighter.lineIndex[x+lineOffset];
              auto start = colored->begin();
              std::advance(start, entry.first);
              auto end = colored->begin();
              std::advance(end, entry.second);
              for(std::map<int, Vec4f>::iterator it = start; it != end; ++it) {
                int xx = it->first;
                if(xx >= cOffset)
                  break;
                color = it->second;
              }
            }
          }
          if((*colored).count(cOffset)) {
            color = (*colored)[cOffset];
          }
          int charAdvance = 0;
          for (c = content.begin(); c != content.end(); c++) {
            if((*colored).count(cOffset)) {
              color = (*colored)[cOffset];
            }

            cOffset++;
            charAdvance++;
            if(*c != '\t')
            entries.push_back(atlas.render(*c, xpos,ypos, color));
            xpos += atlas.getAdvance(*c);
            if(xpos > (maxRenderWidth+ atlas.getAdvance(*c)) && c != content.end()) {
              int remaining = content.length() - (charAdvance ) ;

              if(remaining > 0) {
                if(hasColorIndex) {
                  auto entry = highlighter.lineIndex[x+lineOffset];
                  auto start = colored->begin();
                  std::advance(start, entry.first);
                  auto end = colored->begin();
                  std::advance(end, entry.second);
                  for(std::map<int, Vec4f>::iterator it = start; it != end; ++it) {
                    int xx = it->first;
                    if(xx > cOffset + remaining)
                      break;
                    if(xx >= cOffset)
                    color = it->second;
                  }

                }
                cOffset += remaining;
              }

              break;
            }
          }

          if (x < allLines->size() -1) {
            if((*colored).count(cOffset)) {
              color = (*colored)[cOffset];
            }
            cOffset++;
            xpos = -maxRenderWidth;
            ypos += toOffset;

          }

        }
      } else {
        for(size_t x = 0; x < allLines->size(); x++) {
          auto content = (*allLines)[x].second;
          for (c = content.begin(); c != content.end(); c++) {
            if(*c != '\t')
              entries.push_back(atlas.render(*c, xpos,ypos, color));
            xpos += atlas.getAdvance(*c);
            if(xpos > maxRenderWidth+ atlas.getAdvance(*c)) {
              break;
            }
          }
          if (x < allLines->size() -1) {
            xpos = -maxRenderWidth;
            ypos += toOffset;

          }

        }

      }
      xpos =( -(int32_t)WIDTH/2) + 15;
      ypos = (float)HEIGHT/2 - toOffset - 10;
      std::u16string status = state.status;
      for (c = status.begin(); c != status.end(); c++) {
        entries.push_back(atlas.render(*c, xpos,ypos, status_color));
        xpos += atlas.getAdvance(*c);
      }
      float statusAdvance = atlas.getAdvance(state.status);
      if(state.mode != 0 && state.mode != 32) {
        // draw minibuffer
        xpos =( -(int32_t)WIDTH/2) + 20 + statusAdvance;
        ypos = (float)HEIGHT/2 - toOffset - 10;
        std::u16string status = state.miniBuf;
        for (c = status.begin(); c != status.end(); c++) {
          entries.push_back(atlas.render(*c, xpos,ypos, state.provider.colors.minibuffer_color));
          xpos += atlas.getAdvance(*c);
        }

      } else {
        auto tabInfo = state.getTabInfo();
        xpos =((int32_t)WIDTH/2) - atlas.getAdvance(tabInfo);
        ypos = (float)HEIGHT/2 - toOffset - 10;
        for (c = tabInfo.begin(); c != tabInfo.end(); c++) {
          entries.push_back(atlas.render(*c, xpos,ypos, status_color));
          xpos += atlas.getAdvance(*c);
        }

      }


      glBindBuffer(GL_ARRAY_BUFFER, state.vbo);
      glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(RenderChar) *entries.size(), &entries[0]); // be sure to use glBufferSubData and not glBufferData
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 6, (GLsizei) entries.size());
    if(state.focused) {
      cursor_shader.use();
      cursor_shader.set1f("cursor_height", toOffset);
      cursor_shader.set1f("last_stroke", state.lastStroke);
      cursor_shader.set1f("time", (float)glfwGetTime());
      cursor_shader.set2f("resolution", (float) WIDTH,(float) HEIGHT);
      if(state.mode != 0 && state.mode != 32) {
        // use cursor for minibuffer
        float cursorX = -(int32_t)(WIDTH/2) + 15 + (atlas.getAdvance(cursor->getCurrentAdvance())) + 5 + statusAdvance;
        float cursorY = (float)HEIGHT/2 - 10;
        cursor_shader.set2f("cursor_pos", cursorX, -cursorY);

        glBindVertexArray(state.vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);

      }

      if(isSearchMode || state.mode == 0) {
        float cursorX = -(int32_t)(WIDTH/2) + 15 + (atlas.getAdvance(cursor->getCurrentAdvance(isSearchMode))) + linesAdvance + 4 - cursor->xSkip;
        if(cursorX > WIDTH / 2)
          cursorX = (WIDTH / 2) - 3;
        float cursorY = -(int32_t)(HEIGHT/2) + 4 + (toOffset * ((cursor->y - cursor->skip)+1));

        cursor_shader.set2f("cursor_pos", cursorX, -cursorY);


        glBindVertexArray(state.vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
      }
    }
      if(cursor->selection.active){
        std::vector<SelectionEntry> selectionBoundaries;
        if(cursor->selection.getYSmaller() < cursor->skip && cursor->selection.getYBigger() > cursor->skip + cursor->maxLines) {
          // select everything
        } else {
          maxRenderWidth += atlas.getAdvance(u" ");
          int yStart = cursor->selection.getYStart();
          int yEnd = cursor->selection.getYEnd();
          if(cursor->selection.yStart == cursor->selection.yEnd) {
            if(cursor->selection.xStart != cursor->selection.xEnd) {
              int smallerX = cursor->selection.getXSmaller();
              if(smallerX >= cursor->xOffset) {

                float renderDistance = atlas.getAdvance((*allLines)[yEnd-cursor->skip].second.substr(0, smallerX-cursor->xOffset));
                float renderDistanceBigger = atlas.getAdvance((*allLines)[yEnd-cursor->skip].second.substr(0, cursor->selection.getXBigger()-cursor->xOffset));
                if (renderDistance < maxRenderWidth*2) {
                  float start = ((float)HEIGHT/2) - 5  -  (toOffset *( (yEnd - cursor->skip)+1));
                  selectionBoundaries.push_back({ vec2f(-(int32_t)WIDTH/2 + 20 + linesAdvance + renderDistance, start), vec2f(renderDistanceBigger - renderDistance, toOffset)});
                } else {
                float renderDistanceBigger = atlas.getAdvance((*allLines)[yEnd-cursor->skip].second.substr(0, cursor->selection.getXBigger()-cursor->xOffset));
                float start = ((float)HEIGHT/2) - 5 -  (toOffset *( (yEnd - cursor->skip)+1));
                selectionBoundaries.push_back({ vec2f(-(int32_t)WIDTH/2 + 20 + linesAdvance + (maxRenderWidth-renderDistance), start), vec2f(maxRenderWidth >renderDistanceBigger ? maxRenderWidth : renderDistanceBigger, toOffset)});

                }
              } else {
                float renderDistanceBigger = atlas.getAdvance((*allLines)[yEnd-cursor->skip].second.substr(0, cursor->selection.getXBigger()-cursor->xOffset));
                float start = ((float)HEIGHT/2) - 5 - (toOffset *( (yEnd - cursor->skip)+1));
                  selectionBoundaries.push_back({ vec2f(-(int32_t)WIDTH/2 + 20 + linesAdvance, start), vec2f(renderDistanceBigger > maxRenderWidth*2 ? maxRenderWidth*2 : renderDistanceBigger, toOffset)});
              }
            }
          } else {
            if(yStart >= cursor->skip && yStart <= cursor->skip + cursor->maxLines) {
              int yEffective = cursor->selection.getYStart() - cursor->skip;
              int xStart = cursor->selection.getXStart();
              if(xStart >= cursor->xOffset) {
                float renderDistance = atlas.getAdvance((*allLines)[yEffective].second.substr(0, xStart-cursor->xOffset));
                if (renderDistance < maxRenderWidth) {
                  if (yStart < yEnd) {

                    float start = ((float)HEIGHT/2) - 5 - (toOffset * (yEffective +1));
                    selectionBoundaries.push_back({ vec2f(-(int32_t)WIDTH/2 + 20 + linesAdvance + renderDistance, start), vec2f((maxRenderWidth * 2) - renderDistance, toOffset)});
                  }else {
                    float start = ((float)HEIGHT/2) - 5 - (toOffset * (yEffective+1));
                    selectionBoundaries.push_back({ vec2f(-(int32_t)WIDTH/2 + 20 + linesAdvance, start), vec2f(renderDistance, toOffset)});
                  }
                }
              }
            }
            if(yEnd >= cursor->skip && yEnd <= cursor->skip + cursor->maxLines) {
              int yEffective = cursor->selection.getYEnd() - cursor->skip;
              int xStart = cursor->selection.getXEnd();
              if(xStart >= cursor->xOffset) {
                float renderDistance = atlas.getAdvance((*allLines)[yEffective].second.substr(0, xStart-cursor->xOffset));
                if (renderDistance < maxRenderWidth) {
                  if(yEnd < yStart) {
                    float start = ((float)HEIGHT/2) - 5 - (toOffset * (yEffective+1));
                    selectionBoundaries.push_back({ vec2f(-(int32_t)WIDTH/2 + 20 + linesAdvance + renderDistance, start), vec2f((maxRenderWidth *2) - renderDistance, toOffset)});
                  } else {
                    float start = ((float)HEIGHT/2) - 5 - (toOffset * (yEffective+1));
                    selectionBoundaries.push_back({ vec2f(-(int32_t)WIDTH/2 + 20 + linesAdvance, start), vec2f(renderDistance, toOffset)});

                  }
                }
              }
            }
            bool found = false;
            int offset = 0;
            int count = 0;
            for(int i = cursor->selection.getYSmaller(); i < cursor->selection.getYBigger()-1; i++) {
              if(i > cursor->skip + cursor->maxLines)
                break;
              if(i >= cursor->skip -1) {
                if(!found) {
                  found = true;
                  offset = i - cursor->skip;
                }
                count++;
              }
            }
            if(found) {
              float start = (float)HEIGHT/2 - 5 - (toOffset * (offset +1));
              selectionBoundaries.push_back({vec2f( (-(int32_t)WIDTH/2) + 20 + linesAdvance, start), vec2f(maxRenderWidth * 2,- (count * toOffset))});
            }
          }

      }
        if(selectionBoundaries.size()) {
          selection_shader.use();
          glBindVertexArray(state.sel_vao);
          auto color = state.provider.colors.selection_color;
          selection_shader.set4f("selection_color", color.x, color.y, color.z, color.w);
          selection_shader.set2f("resolution", (float) WIDTH,(float) HEIGHT);
          glBindBuffer(GL_ARRAY_BUFFER, state.sel_vbo);

          glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(SelectionEntry) *selectionBoundaries.size(), &selectionBoundaries[0]);

          glBindBuffer(GL_ARRAY_BUFFER, 0);
          glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 6, (GLsizei)selectionBoundaries.size());

        }
      }
      glBindVertexArray(0);
      glBindTexture(GL_TEXTURE_2D, 0);


      glfwSwapBuffers(window);
      glfwPollEvents();
    }
    glfwTerminate();
  return 0;
};
