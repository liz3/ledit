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

const uint32_t WIDTH = 1280;
const uint32_t HEIGHT = 720;
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

int main(int argc, char** argv) {

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "textedit", nullptr, nullptr);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

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

    Shader text_shader("/Users/liz3/Projects/glfw/simple.vs", "/Users/liz3/Projects/glfw/simple.fs", {});
    text_shader.use();
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(WIDTH), 0.0f, static_cast<float>(HEIGHT));
    glUniformMatrix4fv(glGetUniformLocation(text_shader.pid, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        std::cout << text_shader.pid << "\n";
    State state;
    FontAtlas atlas("/Users/liz3/Downloads/Fira_Code_v5.2/ttf/FiraCode-Regular.ttf", 20);


    while (!glfwWindowShouldClose(window))
    {
      if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
      }

      glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);
      text_shader.use();

      glActiveTexture(GL_TEXTURE0);
      glBindVertexArray(state.vao);
      float xpos = 15;
      float ypos = HEIGHT -atlas.atlas_height-15;
      float w = atlas.atlas_width;
      float h = atlas.atlas_height;
      float vertices[6][4] = {
        { xpos,     ypos + h,   0.0f, 0.0f },
        { xpos,     ypos,       0.0f, 1.0f },
        { xpos + w, ypos,       1.0f, 1.0f },

        { xpos,     ypos + h,   0.0f, 0.0f },
        { xpos + w, ypos,       1.0f, 1.0f },
        { xpos + w, ypos + h,   1.0f, 0.0f }
      };

        glBindTexture(GL_TEXTURE_2D, atlas.texture_id);
        glBindBuffer(GL_ARRAY_BUFFER, state.vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices) , vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);



      glfwSwapBuffers(window);
      glfwPollEvents();

    }
    glfwTerminate();
  return 0;
};
