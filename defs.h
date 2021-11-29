#ifndef DEFS_H_H
#define DEFS_H_H

#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include "cursor.h"
std::string file_to_string(std::string path) {
  std::ifstream stream(path);
  std::stringstream ss;
  ss << stream.rdbuf();
  stream.close();
  return ss.str();
}

class Shader {
 public:
  GLuint pid;
  std::vector<GLuint> shader_ids;
   Shader(std::string vertex, std::string fragment, std::vector<std::string> others) {
     auto vertex_shader = compileSimple(GL_VERTEX_SHADER, vertex);
     auto fragment_shader = compileSimple(GL_FRAGMENT_SHADER, fragment);
     pid = glCreateProgram();
     glAttachShader(pid, vertex_shader);
     glAttachShader(pid, fragment_shader);
     shader_ids.push_back(vertex_shader);
     shader_ids.push_back(fragment_shader);
     for(auto& other : others) {
       auto shader_id = compileSimple(GL_VERTEX_SHADER, other);
       glAttachShader(pid, shader_id);
       shader_ids.push_back(shader_id);
     }
     glLinkProgram(pid);
     checkCompileErrors(pid, "PROGRAM");
   }
  void set2f(std::string name, float x, float y) {
    glUniform2f(glGetUniformLocation(pid, name.c_str()), x, y);
  }
  void set1f(std::string name, float v) {
    glUniform1f(glGetUniformLocation(pid, name.c_str()), v);
  }

  void use() {
       glUseProgram(pid);
  }
 private:
   GLuint compileSimple(GLuint type, std::string path) {
    std::string content = file_to_string(path);
    auto id = glCreateShader(type);
    const char* contentp = content.c_str();
    glShaderSource(id, 1, &contentp, nullptr);
    glCompileShader(id);
    checkCompileErrors(id, shaderTypeString(type));
    return id;

  }
  void checkCompileErrors(GLuint shader, std::string type) {
        GLint success;
        GLchar infoLog[1024];
        if(type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if(!success) {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if(!success) {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
  const std::string shaderTypeString(GLuint shader) {
    switch (shader) {
    case GL_VERTEX_SHADER:
      return "GL_VERTEX_SHADER";
    case GL_FRAGMENT_SHADER:
      return "GL_FRAGMENT_SHADER";
    default:
      return "(Unknown)";
    }
  }
};
struct CharacterEntry {
  float width;
  float height;
  float top;
  float left;
  float advance;
  float offset;
  unsigned char* buffer;
  char c;
};
struct RenderChar {
  Vec2f pos;
  Vec2f size;
  Vec2f uv_pos;
  Vec2f uv_size;
  Vec4f fg_color;
  Vec4f bg_color;
};
class State {
 public:
  GLuint vao, vbo;
  Cursor* cursor;
  float WIDTH, HEIGHT;
  State() {}
  State(Cursor* c, float w, float h) {
    cursor = c;
    WIDTH = w;
    HEIGHT = h;
  }
  void init() {
        glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    //TODO set size of vbo base on buffer size?

    glBufferData(GL_ARRAY_BUFFER, sizeof(RenderChar) * 600 * 1000, nullptr, GL_DYNAMIC_DRAW);
    activate_entries();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

  }
private:
  void activate_entries() {
    //pos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(RenderChar), (void*)offsetof(RenderChar, pos));
    glVertexAttribDivisor(0, 1);

    //size
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(RenderChar), (void*)offsetof(RenderChar, size));
    glVertexAttribDivisor(1, 1);

    //uv_pos
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(RenderChar), (void*)offsetof(RenderChar, uv_pos));
    glVertexAttribDivisor(2, 1);
    //uv_size
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(RenderChar), (void*)offsetof(RenderChar, uv_size));
    glVertexAttribDivisor(3, 1);

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(RenderChar), (void*)offsetof(RenderChar, fg_color));
    glVertexAttribDivisor(4, 1);
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(RenderChar), (void*)offsetof(RenderChar, bg_color));
    glVertexAttribDivisor(5, 1);

  }

};

class FontAtlas {
public:
  std::map<char, CharacterEntry> entries;
  GLuint texture_id;
  FT_UInt atlas_width, atlas_height, smallest_top;
  uint32_t fs;
  RenderChar render(char c, float x = 0.0, float y = 0.0) {
    auto entry = entries[c];
    RenderChar r;
    float x2 = x + entry.left;
    float y2 = y - entry.top + (atlas_height - smallest_top);
    r.pos = vec2f(x2, -y2);
    r.size = vec2f(entry.width, -entry.height);
    r.uv_pos = vec2f(entry.offset, 0.0f);
    r.uv_size = vec2f(entry.width / (float) atlas_width, entry.height / atlas_height);
    return r;
  }
  float getAdvance(char c) {
    return entries[c].advance;
  }
  FontAtlas(std::string path, uint32_t fontSize) {
    fs = fontSize;
    atlas_width = 0;
    atlas_height = 0;
    smallest_top = 1e9;
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return;
    }
    FT_Face face;
    if (FT_New_Face(ft, path.c_str(), 0, &face)) {
      std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
      return;
    }
    FT_Set_Pixel_Sizes(face, 0, fontSize);
    // TODO should this be here?
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for(int i = 0; i < 128; i++) {
      if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
        std::cout << "Failed to load char: " << (char) i << "\n";
        return;
      }
      auto bm = face->glyph->bitmap;
      atlas_width += bm.width;
      atlas_height = bm.rows > atlas_height ? bm.rows : atlas_height;

    }


    // texture
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    //params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, (GLsizei) atlas_width, (GLsizei) atlas_height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    int  xOffset = 0;
    for(int i = 0; i < 128; i++) {
      if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
        std::cout << "Failed to load char: " << (char) i << "\n";
        return;
      }
      CharacterEntry entry;
      auto bm = face->glyph->bitmap;
      entry.width = bm.width;
      entry.height = bm.rows;
      entry.top = face->glyph->bitmap_top;
      entry.left = face->glyph->bitmap_left;
      entry.advance = face->glyph->advance.x >> 6;
//      entry.buffer = bm.buffer;
      entry.c = (char)i;
      if(smallest_top == 0 && entry.top > 0)
        smallest_top = entry.top;
      else
        smallest_top = entry.top < smallest_top && entry.top != 0 ? entry.top : smallest_top;
      entries.insert(std::pair<char, CharacterEntry>(entry.c, entry));


      entries[(char)i].offset = (float) xOffset / (float) atlas_width;
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glTexSubImage2D(
                      GL_TEXTURE_2D,
                      0,
                      xOffset,
                      0,
                      entry.width,
                      entry.height,
                      GL_RED,
                      GL_UNSIGNED_BYTE,
                      face->glyph->bitmap.buffer);

      xOffset += entry.width;

    }
  }
  float getAdvance(std::string line) {
    float v = 0;
    std::string::const_iterator c;
    for (c = line.begin(); c != line.end(); c++) {
      v += entries[*c].advance;
    }
    return v;
  }
};
#endif
