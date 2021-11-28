#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <chrono>
#include "glad.h"
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "la.h"
#include <ft2build.h>
#include FT_FREETYPE_H
using namespace std::chrono;

#include "shader.h"
#include "utils.h"
#include "cursor.h"
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void RenderText(Shader &shader, std::string text, Vec2f* pos, float scale, Vec4f color, FT_UInt aw, FT_UInt ah, std::vector<Free_Glyph>* out);
int getOffsetLeft();
unsigned int get_gl_var(unsigned int p, const char* name) {
  return glGetUniformLocation(p, name);
}
// settings
const unsigned int SCR_WIDTH = 1400;
const unsigned int SCR_HEIGHT = 800;

/// Holds all state information relevant to a character as loaded using FreeType
std::map<GLchar, Glyph_Metric> Characters;
unsigned int VAO, VBO;
Cursor cursor;
float yLineHeight;
bool was_pressed[300] = {false};
int main2(int argc, char** argv)
{
    std::cerr << "YAAAY" << "\n";
  // Cursor c(argv[1]);
  // cursor = c;
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
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

    // compile and setup the shader
    SimpleShader cameraShaderText("/Users/liz3/Projects/glfw/camera.vert");
    std::vector<unsigned int> toAttachText;
    toAttachText.push_back(cameraShaderText.ID);
    Shader shader("/Users/liz3/Projects/glfw/text.vs", "/Users/liz3/Projects/glfw/text.fs", nullptr, &toAttachText);
    SimpleShader cameraShader("/Users/liz3/Projects/glfw/camera.vert");
    std::vector<unsigned int> toAttach;
    toAttach.push_back(cameraShader.ID);
    Shader cursorShader("/Users/liz3/Projects/glfw/cursor.vert", "/Users/liz3/Projects/glfw/cursor.frag", nullptr, &toAttach);


//    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    shader.use();
//    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // FreeType
    // --------
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }

	// find path to font
    std::string font_name = "/Users/liz3/Downloads/Fira_Code_v5.2/ttf/FiraCode-Regular.ttf";
    if (font_name.empty())
    {
        std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
        return -1;
    }

    Free_Glyph* out = new Free_Glyph[600* 1000];
    memset(out, 0, sizeof(Free_Glyph) * 600 * 1000);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(out), out, GL_DYNAMIC_DRAW);
    for (int i = 0; i < COUNT_FREE_GLYPH_ATTRS; ++i) {
      Free_Glyph_Attr attr = static_cast<Free_Glyph_Attr>(i);
      glEnableVertexAttribArray(attr);
      switch (glyph_attr_defs[attr].type) {
      case GL_FLOAT:
        glVertexAttribPointer(
                              attr,
                              glyph_attr_defs[attr].comps,
                              glyph_attr_defs[attr].type,
                              GL_FALSE,
                              sizeof(Free_Glyph),
                              (void*) glyph_attr_defs[attr].offset);
        break;

      case GL_INT:
        glVertexAttribIPointer(
                               attr,
                               glyph_attr_defs[attr].comps,
                               glyph_attr_defs[attr].type,
                               sizeof(Free_Glyph),
                               (void*) glyph_attr_defs[attr].offset);
        break;

      default:
        assert(false && "unreachable");
        exit(1);
      }
      glVertexAttribDivisor(attr, 1);
    }
	// load font as face
    FT_Face face;
    FT_UInt atlas_width = 0;
    FT_UInt atlas_height = 0;
    GLuint glyphs_texture;
    if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return -1;
    }
    else {
        // set size to load glyphs as
        FT_Set_Pixel_Sizes(face, 0, 48);

        // disable byte-alignment restriction
//        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // load first 128 characters of ASCII set
        for (unsigned char c = 0; c < 128; c++)
        {
            atlas_width += face->glyph->bitmap.width;
            if(atlas_height < face->glyph->bitmap.rows)
              atlas_height = face->glyph->bitmap.rows;

        }
        glActiveTexture(GL_TEXTURE1);
        glGenTextures(1, &glyphs_texture);
        glBindTexture(GL_TEXTURE_2D, glyphs_texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            (GLsizei) atlas_width,
            (GLsizei) atlas_height,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            nullptr);
        int x = 0;
        for(int i = 32; i < 128;i++) {
            // Load character glyph
            if (FT_Load_Char(face, i, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }
            // std::cout << (int)face->glyph->bitmap.pixel_mode << "\n";

            // std::vector<unsigned char> png;
            // unsigned error = lodepng::encode(png, face->glyph->bitmap.buffer, face->glyph->bitmap.width, face->glyph->bitmap.rows, LCT_GREY, 8);
            // if(!error) lodepng::save_file(png, "../pngs/" + std::to_string(i) + ".png");
            // // if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL)) {
            // //     fprintf(stderr, "ERROR: could not render glyph of a character with code %d\n", i);


            // }
            Glyph_Metric metric;
            metric.ax = face->glyph->advance.x >> 6;
            metric.ay = face->glyph->advance.y >> 6;
            metric.bw = face->glyph->bitmap.width;
            metric.bh = face->glyph->bitmap.rows;
            metric.bl = face->glyph->bitmap_left;
            metric.bt = face->glyph->bitmap_top;
            metric.tx = (float) x / (float) atlas_width;
            Characters.insert(std::pair<char, Glyph_Metric>(i, metric));

            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexSubImage2D(
                GL_TEXTURE_2D,
                0,
                x,
                0,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer);
            x += face->glyph->bitmap.width;
        }

    }

    // // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);



    // configure VAO/VBO for texture quads
    // -----------------------------------
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // render loop
    // -----------
    float fontSize = 16;
    while (!glfwWindowShouldClose(window))
    {
      float scale = fontSize / 48;
        // input
        // -----
        high_resolution_clock::time_point start_point = high_resolution_clock::now();
        processInput(window);
        high_resolution_clock::time_point point_now = high_resolution_clock::now();
        // std::cout << ">> Processing input: " << (std::chrono::duration_cast<std::chrono::milliseconds>(point_now - start_point).count()) << " ms\n";

        // render

        // ------
        start_point = high_resolution_clock::now();
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        //glfwGetTime()
        float baseXOffset = (getOffsetLeft() * scale) - (float)(SCR_WIDTH / 2);
        float baseYOffset = (((cursor.y * (yLineHeight)) * scale) + cursor.y * 10) - ((SCR_HEIGHT/2) -53);
        glUniform2f(get_gl_var(cursorShader.ID, "cursor_pos"), baseXOffset + 16, -baseYOffset);
        point_now = high_resolution_clock::now();
        // std::cout << ">> setting up props: " << (std::chrono::duration_cast<std::chrono::milliseconds>(point_now - start_point).count()) << " ms\n";
        start_point = high_resolution_clock::now();

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        shader.use();
        glUniform1f(get_gl_var(shader.ID, "time"), (float) glfwGetTime() / 1000);
        glUniform2f(get_gl_var(shader.ID, "resolution"), (float) SCR_WIDTH, (float) SCR_HEIGHT);
//        glUniform1iARB(glGetUniformLocationARB(shader.ID, "font"), 0);
        size_t offset = 0;
        for(size_t row = 0; row < cursor.lines.size(); row++) {
          Vec2f begin = vec2f(0, -(float)row *48);
          Vec2f end = begin;
          std::vector<Free_Glyph> v;
          RenderText(shader, cursor.lines[row], &end, 1, vec4fs(1.0), atlas_width, atlas_height, &v);
          for(auto entry : v) {
            out[offset++] = entry;

          }
        }
        glBufferSubData(GL_ARRAY_BUFFER,
                        0,
                        offset * sizeof(Free_Glyph),
                        out);
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, (GLsizei) offset);


        // glUniform1f(get_gl_var(cursorShader.ID, "cursor_height"), (float) 48 * scale);
        // glUniform2f(get_gl_var(cursorShader.ID, "resolution"), (float) SCR_WIDTH, (float) SCR_HEIGHT);



        // glBindVertexArray(0);
        // glBindTexture(GL_TEXTURE_2D, 0);


// //        RenderText(shader, cursor.getContent(), 15.0f, (float) SCR_HEIGHT -50, scale, glm::vec3(0.5, 0.8f, 0.2f));
//         point_now = high_resolution_clock::now();
//         // std::cout << ">> text rendering:: " << (std::chrono::duration_cast<std::chrono::milliseconds>(point_now - start_point).count()) << " ms\n";
//         start_point = high_resolution_clock::now();
//         cursorShader.use();
//         glBindVertexArray(VAO);
//         glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//         glBindVertexArray(0);
//         glBindTexture(GL_TEXTURE_2D, 0);
//         point_now = high_resolution_clock::now();
//         // std::cout << ">> cursor rendering:: " << (std::chrono::duration_cast<std::chrono::milliseconds>(point_now - start_point).count()) << " ms\n";
//         start_point = high_resolution_clock::now();

        glfwSwapBuffers(window);
        glfwPollEvents();
        point_now = high_resolution_clock::now();
        // std::cout << ">> swapping buffers:: " << (std::chrono::duration_cast<std::chrono::milliseconds>(point_now - start_point).count()) << " ms\n";
    }

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }  else {
      bool enter_pressed = glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS;
      if (enter_pressed && was_pressed[0] == false) {
        cursor.append('\n');
        was_pressed[0] = true;
      }  else if (!enter_pressed && was_pressed[0] == true) {
        was_pressed[0] = false;

      }

      bool delete_pressed = glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS;
      if (delete_pressed && was_pressed[2] == false) {
        cursor.removeOne();
        was_pressed[2] = true;
      }  else if (!delete_pressed && was_pressed[2] == true) {
        was_pressed[2] = false;

      }


      bool space_pressed = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
      if (space_pressed && was_pressed[1] == false) {
        cursor.append(' ');
        was_pressed[1] = true;
      }  else if (!space_pressed && was_pressed[1] == true) {
        was_pressed[1] = false;

      }
      bool shift_pressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
      bool ctrl_pressed = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;

      for(int i = 65; i < 98; i++) {
        bool pressed = glfwGetKey(window, i) == GLFW_PRESS;
        if (pressed && was_pressed[i] == false) {
          char x = (char) i + (shift_pressed ? 0 : 32);
          was_pressed[i] = true;
          if(ctrl_pressed) {
            if (x == 'f') {
              cursor.moveRight();
              continue;
            }

            if (x == 'a') {
              cursor.jumpStart();
              continue;
            }

            if (x == 'e') {
              cursor.jumpEnd();
              continue;
            }

            if (x == 'b') {
              cursor.moveLeft();
              continue;
            }
            if (x == 'p') {
              cursor.moveUp();
              continue;
            }
            if (x == 'n') {
              cursor.moveDown();
              continue;
            }

          }
          cursor.append(x);

        } else if (!pressed && was_pressed[i]) {
          was_pressed[i] = false;
        }
      }
      for(int i = 44; i < 58; i++) {
        bool pressed = glfwGetKey(window, i) == GLFW_PRESS;
        if (pressed && was_pressed[i] == false) {
          cursor.append((char) i);
          was_pressed[i] = true;
        } else if (!pressed && was_pressed[i]) {
          was_pressed[i] = false;
        }
      }

    }

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

int getOffsetLeft() {
   float v = 0;
  // std::string text = cursor.getCurrentAdvance();
  // std::string::const_iterator c;
  // for (c = text.begin(); c != text.end(); c++)
  //   {
  //       Character ch = Characters[*c];
  //       v += (ch.Advance >> 6);
  //   }
  return v;
}

// render line of text
// -------------------
void RenderText(Shader &shader, std::string text, Vec2f* pos, float scale, Vec4f color, FT_UInt aw, FT_UInt ah, std::vector<Free_Glyph>* out)
{
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
      {
        Glyph_Metric metric = Characters[*c];
        float x2 = pos->x + metric.bl;
        float y2 = -pos->y - metric.bt;
        float w  = metric.bw;
        float h  = metric.bh;

        pos->x += metric.ax;
        pos->y += metric.ay;

        Free_Glyph glyph;
        glyph.pos = vec2f(x2, -y2);
        glyph.size = vec2f(w, -h);
        glyph.uv_pos = vec2f(metric.tx, 0.0f);
        glyph.uv_size = vec2f(metric.bw / (float) aw, metric.bh / (float) ah);
        glyph.fg_color = color;
        glyph.bg_color = vec4fs(0);
        out->push_back(glyph);
    }
}
