#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "la.h"
#include "glad.h"
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H
#include "defs.h"
#include "cursor.h"

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
  gState->cursor->append((char) codepoint);
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if(gState == nullptr) return;
  if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
    return;

  }
  bool ctrl_pressed = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
  bool alt_pressed = glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS;
  Cursor* cursor = gState->cursor;
  bool isPress = action == GLFW_PRESS || action == GLFW_REPEAT;
  if(ctrl_pressed) {
    if (key == GLFW_KEY_A && action == GLFW_PRESS)
      cursor->jumpStart();
    else if (key == GLFW_KEY_F && isPress)
      cursor->moveRight();
    else if (key == GLFW_KEY_E && isPress)
      cursor->jumpEnd();
    else if (key == GLFW_KEY_B && isPress)
      cursor->moveLeft();
    else if (key == GLFW_KEY_P && isPress)
      cursor->moveUp();
    else if (key == GLFW_KEY_N && isPress)
      cursor->moveDown();

  } else {
    if(isPress && key == GLFW_KEY_ENTER) {
      cursor->append('\n');
    }
    if(isPress && key == GLFW_KEY_BACKSPACE) {
      cursor->removeOne();
    }

  }

}
int main(int argc, char** argv) {

  std::string x(argv[1]);
  Cursor cursor(x);

    State state(&cursor, 1280, 720);
    gState = &state;
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* window = glfwCreateWindow(state.WIDTH, state.HEIGHT, "textedit", nullptr, nullptr);
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

    Shader text_shader("/Users/liz3/Projects/glfw/simple.vs", "/Users/liz3/Projects/glfw/simple.fs", {});
    text_shader.use();
    Shader cursor_shader("/Users/liz3/Projects/glfw/cursor.vert", "/Users/liz3/Projects/glfw/cursor.frag", {"/Users/liz3/Projects/glfw/camera.vert"});
    // glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(WIDTH), 0.0f, static_cast<float>(HEIGHT));
    // glUniformMatrix4fv(glGetUniformLocation(text_shader.pid, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    std::cout << text_shader.pid << "\n";
    float fontSize = 64;
    FontAtlas atlas("/Users/liz3/Downloads/Fira_Code_v5.2/ttf/FiraCode-Regular.ttf", fontSize);


    float toOffset = atlas.atlas_height;
    std::cout << toOffset << "\n";
    while (!glfwWindowShouldClose(window))
    {
      float WIDTH = state.WIDTH;
      float HEIGHT = state.HEIGHT;
      cursor.setBounds(HEIGHT, atlas.atlas_height * 0.5);
      glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);
      text_shader.use();
      text_shader.set2f("resolution", (float) WIDTH,(float) HEIGHT);
      glActiveTexture(GL_TEXTURE0);
      glBindVertexArray(state.vao);
      glBindTexture(GL_TEXTURE_2D, atlas.texture_id);
      glBindBuffer(GL_ARRAY_BUFFER, state.vbo);

      std::string::const_iterator c;
      float xpos =( -(int32_t)WIDTH) + 10;
      float ypos = -(float)HEIGHT + 15;
      std::vector<RenderChar> entries;
      std::string content = cursor.getContent();
      for (c = content.begin(); c != content.end(); c++) {
        if(*c == '\n') {

          xpos =  -(int32_t)WIDTH + 10;
          ypos += toOffset;
          continue;
        }
        entries.push_back(atlas.render(*c, xpos,ypos));
        xpos += atlas.getAdvance(*c);
      }

//
      glBindBuffer(GL_ARRAY_BUFFER, state.vbo);
      glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(RenderChar) *entries.size(), &entries[0]); // be sure to use glBufferSubData and not glBufferData

      glBindBuffer(GL_ARRAY_BUFFER, 0);

      glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 6, (GLsizei) entries.size());

      glBindTexture(GL_TEXTURE_2D, 0);
       glBindVertexArray(0);
       cursor_shader.use();
      cursor_shader.set1f("cursor_height", atlas.atlas_height);
      cursor_shader.set2f("resolution", (float) WIDTH,(float) HEIGHT);
      float cursorX = -(int32_t)(WIDTH) + 15 + (atlas.getAdvance(cursor.getCurrentAdvance()));
      float cursorY = -(int32_t)(HEIGHT) + 15 + toOffset  + (toOffset * (cursor.y - cursor.skip));
      cursor_shader.set2f("cursor_pos", cursorX, -cursorY);

      glBindVertexArray(state.vao);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
      glBindVertexArray(0);
      glBindTexture(GL_TEXTURE_2D, 0);



      glfwSwapBuffers(window);
      glfwPollEvents();

    }
    glfwTerminate();
  return 0;
};
