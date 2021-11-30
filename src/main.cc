#include <math.h>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "la.h"
#include "glad.h"
#include <GLFW/glfw3.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "state.h"
#include "shader.h"
#include "font_atlas.h"
#include "cursor.h"
#include "shaders.h"
#include "fira_code.h"
#include "highlighting.h"
#include "languages.h"
State* gState = nullptr;
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.

    glViewport(0, 0, width, height);
    if(gState != nullptr) {
      gState->WIDTH = (float)width;
      gState->HEIGHT = (float)height;
    }
}
void character_callback(GLFWwindow* window, unsigned int codepoint)
{
  if(gState == nullptr)
    return;
  bool alt_pressed = glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS;
  if(alt_pressed) {
    if(glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
      gState->cursor->advanceWord();
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
  if(codepoint < 32 || codepoint > 127) {
    gState->status = "Unknown character: " + std::to_string(codepoint);
    return;
  }
  gState->cursor->append((char) codepoint);
  gState->lastStroke = glfwGetTime();
  gState->renderCoords();
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if(gState == nullptr) return;
  if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    if(gState->mode != 0)
      gState->inform(false);
    else
      glfwSetWindowShouldClose(window, true);
    return;

  }
  bool ctrl_pressed = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
  bool x_pressed = glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS;
  bool alt_pressed = glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS;
  Cursor* cursor = gState->cursor;
  bool isPress = action == GLFW_PRESS || action == GLFW_REPEAT;

  if(ctrl_pressed) {

    if(x_pressed) {
      if(action == GLFW_PRESS && key == GLFW_KEY_S) {
        gState->save();
      }
      if(action == GLFW_PRESS && key == GLFW_KEY_O) {
        gState->open();
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
     if (key == GLFW_KEY_S && action == GLFW_PRESS) {
       gState->search();
     } else if (key == GLFW_KEY_Z && isPress) {
       gState->undo();
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
      gState->lastStroke = glfwGetTime();
    if(isPress && key == GLFW_KEY_ENTER) {
      if(gState->mode != 0) {
        gState->inform(true);
        return;
      }  else
        cursor->append('\n');
    }
    if(isPress && key == GLFW_KEY_TAB) {
      cursor->append("  ");
    }
    if(isPress && key == GLFW_KEY_BACKSPACE) {
      cursor->removeOne();
    }
    if(isPress)
      gState->renderCoords();
  }

}
int main(int argc, char** argv) {
  std::string x = argc >=2 ?std::string(argv[1]) : "";
  Cursor cursor = argc >= 2 ? Cursor(x) : Cursor();
    State state(&cursor, 1280, 720, x);
    gState = &state;
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* window = glfwCreateWindow(state.WIDTH, state.HEIGHT, "ledit", nullptr, nullptr);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCharCallback(window, character_callback);



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

    float fontSize = 40;
    FontAtlas atlas(std::string(fira_code), fontSize);
    float toOffset = atlas.atlas_height;
    float xscale, yscale;
    glfwGetWindowContentScale(window, &xscale, &yscale);
    state.WIDTH *= xscale;
    state.HEIGHT *= yscale;


    while (!glfwWindowShouldClose(window))
    {
      float WIDTH = state.WIDTH;
      float HEIGHT = state.HEIGHT;
      bool isSearchMode = state.mode == 2 || state.mode == 6 || state.mode == 7;
      cursor.setBounds(HEIGHT, atlas.atlas_height);
      glClearColor(0.0, 0.0,0.0, 1.0);
      glClear(GL_COLOR_BUFFER_BIT);
      text_shader.use();
      text_shader.set2f("resolution", (float) WIDTH,(float) HEIGHT);
      glActiveTexture(GL_TEXTURE0);
      glBindVertexArray(state.vao);
      glBindTexture(GL_TEXTURE_2D, atlas.texture_id);
      glBindBuffer(GL_ARRAY_BUFFER, state.vbo);
      std::vector<RenderChar> entries;
      std::string::const_iterator c;
      float xpos =( -(int32_t)WIDTH/2) + 10;
      float ypos = -(float)HEIGHT/2 + 15;
      int start = cursor.skip;
      float linesAdvance = 0;
      int maxLines = cursor.skip + cursor.maxLines <= cursor.lines.size() ? cursor.skip + cursor.maxLines : cursor.lines.size();
//      std::cout << start<< "\n";
      int biggestLine = std::to_string(maxLines).length();
      for (int i = start; i < maxLines; i++) {
        std::string value = std::to_string(i+1);
        linesAdvance = 0;
        for (c = value.begin(); c != value.end(); c++) {
          entries.push_back(atlas.render(*c, xpos,ypos, vec4fs(0.8)));
          xpos += atlas.getAdvance(*c);
          linesAdvance += atlas.getAdvance(*c);

        }
        xpos =  -(int32_t)WIDTH/2 + 10;
        ypos += toOffset;
      }
      ypos = -(float)HEIGHT/2 + 15;
      xpos = -(int32_t)WIDTH/2 + 20 + linesAdvance;
      Vec4f color = vec4fs(0.95);
      auto maxRenderWidth = WIDTH - 20 - linesAdvance -50;
      auto allLines = cursor.getContent(&atlas, maxRenderWidth);
      if(state.hasHighlighting) {
        auto* colored = state.highlighter.get();
        int cOffset = cursor.getTotalOffset();
        for(size_t x = 0; x < allLines.size(); x++) {
          auto content = allLines[x];
          if((*colored).count(cOffset))
            color = (*colored)[cOffset];
          int charAdvance = 0;
          for (c = content.begin(); c != content.end(); c++) {
            if((*colored).count(cOffset))
              color = (*colored)[cOffset];
            cOffset++;
            charAdvance++;
            entries.push_back(atlas.render(*c, xpos,ypos, color));
            xpos += atlas.getAdvance(*c);
            if(xpos > maxRenderWidth) {
              cOffset += content.length() - charAdvance;
              break;
            }
          }
          if (x < allLines.size() -1) {
            if((*colored).count(cOffset))
              color = (*colored)[cOffset];

            cOffset++;

            xpos = -(int32_t)WIDTH/2 + 20 + linesAdvance;
            ypos += toOffset;

          }

        }
      } else {
        for(size_t x = 0; x < allLines.size(); x++) {
          auto content = allLines[x];
          for (c = content.begin(); c != content.end(); c++) {
            entries.push_back(atlas.render(*c, xpos,ypos, color));
            xpos += atlas.getAdvance(*c);
            if(xpos > maxRenderWidth) {
              break;
            }
          }
          if (x < allLines.size() -1) {
            xpos = -(int32_t)WIDTH/2 + 20 + linesAdvance;
            ypos += toOffset;

          }

        }

      }
      xpos =( -(int32_t)WIDTH/2) + 15;
      ypos = (float)HEIGHT/2 - toOffset;
      std::string status = state.status;
      for (c = status.begin(); c != status.end(); c++) {
        entries.push_back(atlas.render(*c, xpos,ypos, vec4f(0.8,0.8,1.0, 0.9)));
        xpos += atlas.getAdvance(*c);
      }
      float statusAdvance = atlas.getAdvance(state.status);
      if(state.mode != 0) {
        // draw minibuffer
        xpos =( -(int32_t)WIDTH/2) + 20 + statusAdvance;
        ypos = (float)HEIGHT/2 - toOffset;
        std::string status = state.miniBuf;
        for (c = status.begin(); c != status.end(); c++) {
          entries.push_back(atlas.render(*c, xpos,ypos, vec4fs(1.0)));
          xpos += atlas.getAdvance(*c);
        }

      }


      glBindBuffer(GL_ARRAY_BUFFER, state.vbo);
      glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(RenderChar) *entries.size(), &entries[0]); // be sure to use glBufferSubData and not glBufferData

      glBindBuffer(GL_ARRAY_BUFFER, 0);

      glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 6, (GLsizei) entries.size());

      glBindTexture(GL_TEXTURE_2D, 0);
       glBindVertexArray(0);
       cursor_shader.use();
      cursor_shader.set1f("cursor_height", atlas.atlas_height * 0.85);
      cursor_shader.set1f("last_stroke", state.lastStroke);
      cursor_shader.set1f("time", (float)glfwGetTime());
      cursor_shader.set2f("resolution", (float) WIDTH,(float) HEIGHT);
      if(state.mode != 0) {
        // use cursor for minibuffer
        float cursorX = -(int32_t)(WIDTH/2) + 15 + (atlas.getAdvance(cursor.getCurrentAdvance())) + 5 + statusAdvance;
        float cursorY = ((int32_t)(HEIGHT/2) - 10);
        cursor_shader.set2f("cursor_pos", cursorX, -cursorY);

      glBindVertexArray(state.vao);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
      glBindVertexArray(0);
      glBindTexture(GL_TEXTURE_2D, 0);

      }

      if(isSearchMode || state.mode == 0){
        float cursorX = -(int32_t)(WIDTH/2) + 15 + (atlas.getAdvance(cursor.getCurrentAdvance(isSearchMode))) + linesAdvance + 4 - cursor.xSkip;
        if(cursorX > WIDTH / 2)
          cursorX = (WIDTH / 2) - 3;
      float cursorY = -(int32_t)(HEIGHT/2) +  15 + (toOffset - (atlas.atlas_height *  0.15)) + (toOffset * (cursor.y - cursor.skip));

      cursor_shader.set2f("cursor_pos", cursorX, -cursorY);


      glBindVertexArray(state.vao);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
      glBindVertexArray(0);
      glBindTexture(GL_TEXTURE_2D, 0);


      }
      glfwSwapBuffers(window);
      glfwPollEvents();

    }
    glfwTerminate();
  return 0;
};
